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
  \
  # SINTAXE CORRETA:
  -fsdev local,id=myid,path=$HOST_SHARE,security_model=passthrough \
  -device virtio-9p-pci,fsdev=myid,mount_tag=hostshare 