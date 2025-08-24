
# DEU ERRADO ESSA VERSAO


#!/bin/bash
set -e
set -x

# Caminhos
KERNEL_IMG="output/images/bzImage"
ROOTFS_IMG="output/images/rootfs.ext2"

# Nome da interface TAP
TAP_IF="tap0"

# Cria interface TAP se não existir
if ! ip link show $TAP_IF >/dev/null 2>&1; then
    sudo ip tuntap add dev $TAP_IF mode tap user $(whoami)
    sudo ip link set $TAP_IF up
    sleep 0.5
    sudo sysctl -w net.ipv4.ip_forward=1
fi

# Rota para a target (simula IP interno)
sudo route add -host 192.168.1.10 dev $TAP_IF || true

# Executa QEMU
sudo qemu-system-i386 \
    --kernel "$KERNEL_IMG" \
    --hda "$ROOTFS_IMG" \
    --nographic \
    --append "console=ttyS0 root=/dev/sda" \
    -netdev tap,id=net0,ifname=$TAP_IF,script=no,downscript=no \
    -device e1000,netdev=net0,mac=aa:bb:cc:dd:ee:ff \
    -net user,hostfwd=tcp::8080-:8080
