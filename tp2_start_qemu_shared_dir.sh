#!/bin/bash

# Caminho da pasta compartilhada no host (visível no Codespace)
HOST_SHARE="/workspaces/labsisop-buildroot/shared"

# Cria se não existir
mkdir -p "$HOST_SHARE"

qemu-system-i386 \
  --kernel output/images/bzImage \
  --hda output/images/rootfs.ext2 \
  --hdb sdb.bin \
  --nographic \
  --append "console=ttyS0 root=/dev/sda" \
  -fsdev local,id=myid,path=$HOST_SHARE,security_model=none \
  -device virtio-9p-pci,fsdev=myid,mount_tag=hostshare \
  -net user,hostfwd=tcp::2223-:22 -net nic