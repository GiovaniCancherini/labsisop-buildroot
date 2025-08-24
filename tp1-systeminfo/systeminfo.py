#!/usr/bin/env python3

import json
import os
import time
from http.server import BaseHTTPRequestHandler, HTTPServer
from typing import Dict, List, Tuple

# --- Alunos devem implementar as funções abaixo --- #

def get_datetime():
    try:
        # Timestamp atual via /proc
        with open("/proc/stat", "r") as f:
            for line in f:
                if line.startswith("btime"):
                    btime = int(line.split()[1])
                    break
            else:
                return ""

        with open("/proc/uptime", "r") as f:
            uptime = float(f.readline().split()[0])

        timestamp = btime + uptime - 3*3600  # Ajuste UTC-3

        # Converte para struct_time
        tm = time.localtime(int(timestamp))

        # Calcula milissegundos
        ms = int((timestamp - int(timestamp)) * 1000)

        # Formata com milissegundos
        return time.strftime(f"%d/%m/%Y %H:%M:%S:{ms:03d}", tm)
    except Exception:
        return ""

def get_uptime():
    line = _read_first_line("/proc/uptime")
    if not line:
        return 0
    try:
        return int(float(line.split()[0]))
    except Exception:
        return 0

def _cpu_times():
    line = ""
    for l in _read_all("/proc/stat").splitlines():
        if l.startswith("cpu "):
            line = l
            break
    if not line:
        return (0, 0)
    parts = line.split()
    nums = [int(x) for x in parts[1:]]
    idle = nums[3] + (nums[4] if len(nums) > 4 else 0)  # idle + iowait
    total = sum(nums)
    return (idle, total)

def _cpu_usage_percent(sample_ms: int = 100) -> float:
    idle1, total1 = _cpu_times()
    time.sleep(sample_ms / 1000.0)
    idle2, total2 = _cpu_times()
    didle = idle2 - idle1
    dtotal = total2 - total1
    if dtotal <= 0:
        return 0.0
    usage = 100.0 * (1.0 - (didle / dtotal))
    return max(0.0, min(100.0, usage))

def _cpu_model_and_mhz():
    cpuinfo = _read_all("/proc/cpuinfo")
    model = ""
    mhz = -1
    for line in cpuinfo.splitlines():
        if not model and "model name" in line:
            _, val = line.split(":", 1)
            model = val.strip()
        if "cpu MHz" in line:
            try:
                _, val = line.split(":", 1)
                mhz = int(float(val.strip()))
            except Exception:
                pass
    return (model or "Unknown", mhz)

def get_cpu_info():
    model, mhz = _cpu_model_and_mhz()
    usage = _cpu_usage_percent()
    return {
        "model": model,
        "speed_mhz": mhz,
        "usage_percent": round(usage, 1)
    }

def get_memory_info():
    meminfo = _read_all("/proc/meminfo")
    total_kb = 0
    available_kb = 0
    for line in meminfo.splitlines():
        if line.startswith("MemTotal:"):
            total_kb = _to_int(line.split()[1])
        elif line.startswith("MemAvailable:"):
            available_kb = _to_int(line.split()[1])
    total_mb = int(total_kb / 1024)
    used_mb = int((total_kb - available_kb) / 1024) if total_kb and available_kb else 0
    return {"total_mb": total_mb, "used_mb": used_mb}

def get_os_version():
    v = _read_first_line("/proc/version")
    return v or "Unknown"

def get_process_list():
    procs = []
    for name in _listdir("/proc"):
        if not _is_pid_dir(name):
            continue
        pid = _to_int(name)
        comm = _read_first_line(f"/proc/{pid}/comm")
        if not comm:
            status = _read_all(f"/proc/{pid}/status")
            for line in status.splitlines():
                if line.startswith("Name:"):
                    comm = line.split(":", 1)[1].strip()
                    break
        procs.append({"pid": pid, "name": comm or "unknown"})
    procs.sort(key=lambda p: p["pid"])
    return procs

def get_disks() -> List[Dict]:
    skip_prefixes = ("loop", "ram", "zram", "sr")
    disks = []
    for dev in _listdir("/sys/block"):
        if dev.startswith(skip_prefixes):
            continue
        sectors_str = _read_first_line(f"/sys/block/{dev}/size")
        sectors = _to_int(sectors_str, 0)
        size_bytes = sectors * 512
        size_mb = _bytes_to_mb(size_bytes)
        if size_mb <= 0:
            continue
        disks.append({
            "device": f"/dev/{dev}",
            "size_mb": size_mb
        })
    disks.sort(key=lambda d: d["device"])
    return disks

def get_usb_devices() -> List[Dict]:
    base = "/sys/bus/usb/devices"
    items = []
    for dev in _listdir(base):
        path = os.path.join(base, dev)
        manufacturer = _read_first_line(os.path.join(path, "manufacturer"))
        product = _read_first_line(os.path.join(path, "product"))
        description = (manufacturer + " " + product).strip()
        items.append({"port": dev, "description": description})
    items.sort(key=lambda x: x["port"])
    return items

def _parse_ipv4_from_fib_trie():
    lines = _read_all("/proc/net/fib_trie").splitlines()
    results = []
    for i, line in enumerate(lines):
        if line.strip().startswith("32 host LOCAL"):
            if i >= 1:
                prev = lines[i - 1].strip().lstrip("|- ").strip()
                if prev.count(".") == 3:
                    ip = prev
                    for j in range(i, min(i + 6, len(lines))):
                        if " dev " in lines[j]:
                            iface = lines[j].split(" dev ", 1)[-1].strip().split()[0]
                            results.append((iface, ip))
                            break
    return results

def get_network_adapters():
    ifaces = _listdir("/sys/class/net")
    ip_map = {}
    for iface, ip in _parse_ipv4_from_fib_trie():
        ip_map.setdefault(iface, ip)
    adapters = []
    for iface in sorted(ifaces):
        adapters.append({"interface": iface, "ip_address": ip_map.get(iface, "")})
    return adapters

# --- Utils --- #
def _read_first_line(path: str) -> str:
    try:
        with open(path, "r") as f:
            return f.readline().strip()
    except Exception:
        return ""

def _read_all(path: str) -> str:
    try:
        with open(path, "r") as f:
            return f.read()
    except Exception:
        return ""

def _listdir(path: str):
    try:
        return os.listdir(path)
    except Exception:
        return []

def _is_pid_dir(name: str) -> bool:
    return name.isdigit()

def _to_int(s: str, default: int = 0) -> int:
    try:
        return int(s)
    except Exception:
        return default

def _bytes_to_mb(b: int) -> int:
    return int(b // (1024 * 1024))

# --- Servidor HTTP --- #

class StatusHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path != "/status":
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b"Not Found")
            return

        response = {
            "datetime": get_datetime(),
            "uptime_seconds": get_uptime(),
            "cpu": get_cpu_info(),
            "memory": get_memory_info(),
            "os_version": get_os_version(),
            "processes": get_process_list(),
            "disks": get_disks(),
            "usb_devices": get_usb_devices(),
            "network_adapters": get_network_adapters()
        }

        data = json.dumps(response, indent=2).encode()
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

def run_server(port=8080):
    print(f"Servidor disponível em http://0.0.0.0:{port}/status")
    server = HTTPServer(("0.0.0.0", port), StatusHandler)
    server.serve_forever()

if __name__ == "__main__":
    run_server()
