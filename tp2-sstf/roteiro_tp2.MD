### Roteiro de Testes: Avaliando o Escalador de Disco SSTF (TP2)

Este guia documenta as etapas para a avaliação de desempenho do seu escalonador **SSTF**. O objetivo é comparar as métricas do seu escalonador com as de outros escalonadores do kernel, como `noop`, `deadline`, e `cfq`.

-----

#### 1\. Preparação do Ambiente

Antes de iniciar os testes, compile seu módulo e o aplicativo de testes `sector_read`. Certifique-se de que o ambiente está configurado para o teste de disco.

  * **Compilar o Projeto:**
    ```sh
    make
    ```
  * **Preparar o Disco:**
    [cite\_start]Execute estes comandos na sua máquina virtual para limpar caches e otimizar o disco para testes de escalonamento. [cite: 32]
    ```sh
    echo 3 > /proc/sys/vm/drop_caches
    echo 2 > /sys/block/sdb/queue/nomerges
    echo 4 > /sys/block/sdb/queue/max_sectors_kb
    echo 0 > /sys/block/sdb/queue/read_ahead_kb
    ```
      * [cite\_start]`nomerges=2`: Evita a fusão de requisições de I/O adjacentes. [cite: 32]
      * [cite\_start]`max_sectors_kb=4`: Limita o tamanho máximo de cada requisição. [cite: 32]
      * [cite\_start]`read_ahead_kb=0`: Desativa a leitura antecipada. [cite: 32]

-----

#### 2\. Execução dos Testes

O teste consiste em rodar seu programa `sector_read` para cada escalonador, coletando métricas relevantes de desempenho. Repita cada teste algumas vezes para garantir a consistência dos resultados.

  * **Teste para cada escalonador:**
    Para cada escalonador (`noop`, `deadline`, `cfq`, `sstf`), siga estes passos:
    1.  **Ativar o Escalador:**
        ```sh
        echo [nome_do_escalonador] > /sys/block/sdb/queue/scheduler
        ```
    2.  **Executar o Teste e Coletar o Tempo:**
        Use o comando `time` para medir o tempo de execução total.
        ```sh
        time ./sector_read
        ```
    3.  **Coletar Métricas de Desempenho:**
        Antes e depois de cada teste, colete as estatísticas de I/O do disco.
        ```sh
        cat /sys/block/sdb/stat
        ```
        [cite\_start]Você pode usar o `iostat` para coletar métricas em tempo real. [cite: 32]
        ```sh
        iostat -x 1 /dev/sdb
        ```

-----

#### 3\. Análise e Comparação de Resultados

Depois de coletar os dados para cada escalonador, organize-os em uma tabela para uma análise clara e visual.

  * **Métricas a Coletar:**

      * **Tempo total:** A métrica `real` do comando `time`.
      * **Número de `seeks`:** Pode ser estimado a partir das estatísticas do disco, comparando o número de leituras e o número de fusões (`merges`).
      * **Latência média:** Fornecida pelo `iostat`.

  * **Tabela Comparativa:**
    Preencha uma tabela como a abaixo para comparar o desempenho de forma objetiva.

| Escalonador | Tempo (s) | Leituras | Merges | Latência Média (ms) |
| :--- | :--- | :--- | :--- | :--- |
| **noop** | ... | ... | ... | ... |
| **deadline** | ... | ... | ... | ... |
| **cfq** | ... | ... | ... | ... |
| **sstf** | ... | ... | ... | ... |

Lembre-se de que o principal objetivo do SSTF é reduzir o tempo de busca, o que deve se refletir em um número menor de `seeks` e uma latência mais baixa em comparação com escalonadores mais simples como o `noop`.