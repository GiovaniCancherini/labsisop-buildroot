# 🖥️ Tutorial de Manipulação da Máquina Target no QEMU

Este guia serve para lembrar os principais comandos e procedimentos para **subir, interagir e desligar** a máquina-alvo (`target`) no laboratório de Sistemas Operacionais.

---

## 📂 Estrutura de Arquivos

- **`start_qemu.sh`** → script que inicia a máquina target no QEMU.
- **`output/images/bzImage`** → kernel Linux gerado pelo Buildroot.
- **`output/images/rootfs.ext2`** → sistema de arquivos raiz (*root filesystem*).

---

## 🚀 Iniciando a Máquina Target

1. **Dar permissão de execução** (apenas uma vez):
   ```bash
   chmod +x start_qemu.sh
   ```

2. **Subir a máquina no QEMU**:
   ```bash
   ./start_qemu.sh
   ```

> O script `start_qemu.sh` contém o comando:
>
> ```bash
> qemu-system-i386 \
>   --kernel output/images/bzImage \
>   --hda output/images/rootfs.ext2 \
>   --nographic \
>   --append "console=ttyS0 root=/dev/sda"
> ```

---

## 🛠️ Trabalhando Dentro da Máquina

Após iniciar, você estará no **prompt do Linux emulado**.

### Exemplos de comandos úteis:
```bash
ls            # Lista arquivos e diretórios
pwd           # Mostra diretório atual
uname -a      # Mostra informações do kernel
df -h         # Mostra uso de disco
```

### Instalando programas (se configurado no Buildroot)
> O sistema não tem gerenciador de pacotes como `apt` ou `yum`, pois é mínimo.  
> Programas extras devem ser incluídos na configuração do Buildroot antes de recompilar.

---

## 🔌 Desligando a Máquina Target

Existem três formas principais:

### 1. **Desligamento limpo** (recomendado)
No terminal da máquina emulada:
```bash
poweroff
```
ou
```bash
halt -p
```

### 2. **Saída forçada do QEMU**
Pressione:
```
Ctrl + A  depois  X
```

### 3. **Monitor do QEMU** (se habilitado)
No prompt do QEMU:
```bash
system_powerdown
```

---

## 💡 Dicas

* **Evite sair com `Ctrl+C`** no host, pois isso pode corromper o `rootfs.ext2`.
* Para saber se a máquina está rodando, verifique os processos:
  ```bash
  ps aux | grep qemu
  ```
* Para desmontar imagens montadas manualmente:
  ```bash
  sudo umount /ponto/de/montagem
  ```

---

## 📌 Fluxo Resumido

1. Iniciar: `./start_qemu.sh`  
2. Usar a máquina normalmente  
3. Desligar: `poweroff` **ou** `Ctrl+A` depois `X`

---
