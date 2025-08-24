
# Monitor de Sistema Linux

## Descrição do Funcionamento Básico

Este programa é um monitor de sistema para Linux que coleta e exibe informações detalhadas sobre o estado atual do sistema operacional. Ele funciona como um servidor HTTP que disponibiliza um endpoint `/status` na porta 8080, retornando dados em formato JSON.

### Como executar

```bash
python3 systeminfo.py
````

O servidor ficará disponível em `http://localhost:8080/status`.

**Observação importante:**
Como o ambiente de execução está dentro do **Codespaces**, não foi possível acessar diretamente o endpoint pelo navegador do PC local. Tentativas de criar um start\_qemu personalizado com configuração de rede e port forwarding falharam.
Portanto, o acesso foi feito via `curl` dentro do shell da máquina host do Codespaces, obtendo o sucesso esperado:

```bash
curl http://127.0.0.1:8080/status
```

### Informações Coletadas

O programa coleta as seguintes informações do sistema:

* **Data e hora atual** do sistema
* **Tempo de atividade** (uptime) em segundos
* **Informações da CPU**: modelo, velocidade e percentual de uso
* **Memória RAM**: total e em uso (em MB)
* **Versão do sistema operacional**
* **Lista de processos** em execução
* **Discos** disponíveis e seus tamanhos
* **Dispositivos USB** conectados
* **Adaptadores de rede** e seus endereços IP

## Como Cada Informação é Obtida

### 1. Data e Hora (`get_datetime()`)

* **Fonte**: `/proc/stat` e `/proc/uptime`

  * Lê `btime` de `/proc/stat` (timestamp do boot)
  * Lê o uptime do sistema `/proc/uptime`
  * Calcula o timestamp atual combinando ambos e ajusta para o fuso horário brasileiro (UTC-3)
  * Converte para data e hora formatada com milissegundos

### 2. Tempo de Atividade (`get_uptime()`)

* **Fonte**: `/proc/uptime`

  * O primeiro valor representa o tempo total em segundos desde que o sistema foi iniciado
  * Converte o valor de ponto flutuante para inteiro

### 3. Informações da CPU (`get_cpu_info()`)

#### Modelo e Velocidade

* **Fonte**: `/proc/cpuinfo`

  * Linha `model name` fornece o modelo do processador
  * Linha `cpu MHz` fornece a velocidade atual em MHz

#### Uso da CPU

* **Fonte**: `/proc/stat`

  * Lê a linha `cpu ` agregada
  * Coleta duas amostras com intervalo de 100ms
  * Calcula o percentual de uso: `uso = 100 * (1 - (delta_idle / delta_total))`

### 4. Informações de Memória (`get_memory_info()`)

* **Fonte**: `/proc/meminfo`

  * `MemTotal` e `MemAvailable`
  * Memória usada: `total - disponível`
  * Conversão de KB para MB

### 5. Versão do Sistema Operacional (`get_os_version()`)

* **Fonte**: `/proc/version`

  * Contém informações do kernel, compilador e data de compilação

### 6. Lista de Processos (`get_process_list()`)

* **Fonte**: diretórios numéricos em `/proc/`

  * Cada diretório representa um PID
  * Nome do processo: `/proc/[PID]/comm` ou `/proc/[PID]/status` (`Name:`)
  * Lista ordenada por PID

### 7. Discos (`get_disks()`)

* **Fonte**: `/sys/block/`

  * Lista dispositivos de bloco (ignora loop, ram, zram, sr)
  * Tamanho em setores `/sys/block/[device]/size`
  * Converte para MB: `(setores * 512) / (1024*1024)`

### 8. Dispositivos USB (`get_usb_devices()`)

* **Fonte**: `/sys/bus/usb/devices/`

  * Fabricante: `/sys/bus/usb/devices/[device]/manufacturer`
  * Produto: `/sys/bus/usb/devices/[device]/product`
  * Combina fabricante e produto para descrição

### 9. Adaptadores de Rede (`get_network_adapters()`)

* **Fonte**: `/sys/class/net/` (interfaces) e `/proc/net/fib_trie` (IPs)

  * Analisa a tabela de roteamento IPv4
  * Associa cada IP à sua interface correspondente

