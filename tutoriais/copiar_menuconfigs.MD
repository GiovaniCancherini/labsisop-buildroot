# Guia de Replicação de Ambiente Buildroot (VirtFS/9p)

Este tutorial garante que seu ambiente Buildroot seja idêntico ao ambiente de origem, incluindo o suporte completo ao compartilhamento de diretórios Host-Target via VirtFS (9p).

---

## 1. Arquivos Necessários

Certifique-se de que você recebeu os seguintes arquivos do ambiente de origem:

1.  `defconfig` (Configuração principal do Buildroot)
2.  `kernel.config` (Configuração do Kernel com suporte a 9p/VirtIO)

## 2. Configuração do Ambiente

Execute estes passos na **raiz** do seu projeto Buildroot (onde você encontra os diretórios `output/` e `configs/`).

### Passo A: Aplicar a Configuração Principal do Buildroot

Este comando configura a sua árvore de *build* com os mesmos pacotes e versões.

```bash
# 1. Copia o defconfig do colega para a raiz do seu projeto
cp /caminho/do/arquivo/defconfig ./

# 2. Aplica a configuração principal (cria o .config)
make defconfig
````

### Passo B: Substituir a Configuração do Kernel

A configuração do Kernel é o passo mais crucial para garantir que o suporte ao VirtFS (9p) esteja ativo.

  * O Buildroot usará um arquivo de configuração do Kernel baseado no que foi definido no `defconfig`. Assumindo que o arquivo de destino seja `configs/linux-custom_defconfig` (o padrão para configurações customizadas):

<!-- end list -->

```bash
# 1. Copia o arquivo kernel.config (do colega) para o nome que o Buildroot usará
# NOTA: O nome do arquivo pode variar. Verifique o seu .config se esta linha falhar.
cp /caminho/do/arquivo/kernel.config configs/linux-custom_defconfig
```

### Passo C: Compilar o Buildroot

Com as configurações aplicadas, execute o `make` para construir o sistema do zero.

```bash
make clean # Recomendado para garantir um build limpo
make
```

-----

## 3\. Inicialização e Montagem do Compartilhamento

Após a compilação ser bem-sucedida, use o script QEMU fornecido para iniciar a VM com o canal de comunicação VirtFS ativo.

### Passo A: Iniciar o QEMU

O script já contém a sintaxe `fsdev`/`device` e as tags de montagem necessárias (provavelmente `mount_tag=hostshare`).

```bash
./start_qemu.sh 
```
ou
```bash
./tp2_start_qemu_shared_dir.sh
```

### Passo B: Montar o Diretório Compartilhado (No Target/QEMU)

No shell do Buildroot (`#`), execute o comando de montagem que garante a permissão de escrita.

```bash
# 1. Cria o ponto de montagem
mkdir -p /shared

# 2. Monta com as opções de permissão forçada (essencial para escrita)
mount -t 9p -o trans=virtio,version=9p2000.L,msize=65536,fmask=000,dmask=000,uname=root hostshare /shared
```

### Passo C: Verificar o Sucesso

Se o comando retornar sem erros, o compartilhamento está ativo.

```bash
# Tente criar um arquivo para confirmar que a escrita funciona
echo "Sucesso!" > /shared/replicacao_ok.txt

# Verifique no Host (Codespace) se o arquivo apareceu na pasta compartilhada.
```

```