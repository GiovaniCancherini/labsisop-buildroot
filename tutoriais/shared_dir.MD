# Tutorial: Compartilhamento de Diretório entre Host e Target (VirtFS/9p)

Este guia detalha os passos para configurar um diretório compartilhado usando o protocolo VirtFS (9p) com o driver VirtIO.

-----

## 1\. Configuração do Buildroot (`make menuconfig`)

Você precisa garantir que o kernel e o *root filesystem* do Buildroot tenham os pacotes e módulos necessários para lidar com o compartilhamento VirtFS/9p.

### A. Ativação do Pacote `mount` (BusyBox)

Certifique-se de que o utilitário `mount` (que é fornecido pelo BusyBox no Buildroot) está configurado para suportar 9p.

1.  Execute: `make menuconfig`
2.  Navegue para: **Target packages** ->$ **System tools**
3.  Marque: **`[*] busybox`**
4.  Certifique-se de que a opção `mount` dentro da configuração do BusyBox está ativada (geralmente é o padrão).

### B. Ativação do Filesystem VirtFS/9p (Kernel)

O kernel deve ter suporte para o próprio sistema de arquivos 9p e, crucialmente, para o driver de transporte VirtIO.

1.  Execute: `make linux-menuconfig`

2.  Navegue para: **File systems**

3.  Navegue para: **->$ 9p Network File System**

4.  Marque as seguintes opções:

      * `[*]` **Plan 9 Resource Sharing Support (9P2000)**
      * `[*]` **9P POSIX Access Control Lists**
      * `[*]` **9P Security Labels**

5.  Navegue de volta para o menu **File systems** e depois para **->$ Network File Systems**

6.  Navegue para: **->$ Virtio devices**

7.  Marque: 

    * `[*]` **PCI driver for virtio devices** (Requisito essencial para o canal)
    * `[*]` **9P Virtio transport** (É o driver que o `trans=virtio` usa. Se não aparecer, verifique se os drivers Virtio básicos estão ativos).

    *Recomendação: Escolha `[*]` (compilar no kernel) em vez de `[M]` (módulo) para garantir que o driver esteja sempre disponível.*

8.  Salve a configuração do kernel e saia.

-----

## 2\. Preparação no Host (Codespace)

Esta etapa envolve a criação do diretório que será compartilhado e a garantia de que as permissões estejam abertas para evitar o erro **`Operation not permitted`**.

### A. Criar a Pasta Compartilhada

O script a seguir cria o diretório e define as permissões abertas (`777`) que minimizam os erros:

```bash
# Definindo a variável para o caminho do host
HOST_SHARE="/workspaces/labsisop-buildroot/shared"

# 1. Cria o diretório
mkdir -p "$HOST_SHARE"

# 2. Define as permissões mais abertas para evitar "Operation not permitted"
chmod 777 "$HOST_SHARE"
```

### B. Compilar o Buildroot

Após as alterações de configuração, compile o Buildroot. Isso gerará as imagens do kernel e do *rootfs* atualizadas.

```bash
make
```

-----

## 3\. Comando de Start do QEMU

Você deve usar a sintaxe **`-fsdev` e `-device virtio-9p-pci`** para configurar o compartilhamento, pois a sintaxe `-virtfs` causou problemas de canal.

Use este template para o seu script de inicialização do QEMU (ex: `./start_qemu.sh`):

```bash
#!/bin/bash

# Caminho da pasta compartilhada no host (visível no Codespace)
HOST_SHARE="/workspaces/labsisop-buildroot/shared"

# Certifica que a pasta existe e tem permissão total
mkdir -p "$HOST_SHARE"
chmod 777 "$HOST_SHARE"

qemu-system-i386 \
  --kernel output/images/bzImage \
  --hda output/images/rootfs.ext2 \
  --hdb sdb.bin \
  --nographic \
  --append "console=ttyS0 root=/dev/sda" \
  \
  # Configuração ESSENCIAL do VirtFS (9p)
  # fsdev: Define a origem e o modelo de segurança
  # device: Cria o dispositivo VirtIO PCI usando a mount_tag 'hostshare'
  -fsdev local,id=myid,path=$HOST_SHARE,security_model=none \
  -device virtio-9p-pci,fsdev=myid,mount_tag=hostshare
```

-----

## 4\. Montagem e Uso no Target (Buildroot/QEMU)

Após iniciar o QEMU com o script corrigido, execute estes comandos no shell do Buildroot (`#`).

### A. Criar o Ponto de Montagem

```bash
mkdir -p /shared
```

### B. Montar o Compartilhamento

Use o comando final otimizado, que inclui opções de performance (`msize`) e, mais importante, o mapeamento forçado de usuário (`uname=root`) para garantir permissão total de leitura e escrita.

```bash
mount -t 9p -o trans=virtio,version=9p2000.L,msize=65536,fmask=000,dmask=000,uname=root hostshare /shared
```

| Opção | Propósito |
| :--- | :--- |
| `hostshare` | A tag de montagem definida no QEMU. |
| `trans=virtio` | O driver de transporte (o `virtio-9p-pci` do QEMU). |
| `msize=65536` | Aumenta a velocidade de transferência (evita o aviso de performance). |
| `uname=root` | **Essencial** para resolver o erro `Operation not permitted`. Força a escrita como `root`. |
| `fmask=000,dmask=000` | Garante permissão total de escrita para arquivos e diretórios. |

### C. Verificar o Sucesso

```bash
# Se retornar ao prompt sem erro, a montagem funcionou.
# Verifique o conteúdo do diretório compartilhado
ls /shared

# Tente copiar um arquivo de volta (o que estava falhando antes)
touch /shared/teste_escrita.txt
```