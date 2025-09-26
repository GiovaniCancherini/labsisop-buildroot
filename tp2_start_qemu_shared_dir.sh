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
  -virtfs local,path=$HOST_SHARE,mount_tag=hostshare,security_model=passthrough,id=hostshare
