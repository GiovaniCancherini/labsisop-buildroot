Objetivo
========

Nos últimos tutoriais foram apresentadas as formas básicas de uso da
ferramenta Ftrace, sendo essa acessada diretamente pelo sistema de
arquivos *debugfs*. Apesar da interface provida pelo *debugfs* ser
simples, em alguns casos essa pode ser inconveniente por motivos
diversos. Geralmente quando estamos trabalhando com dispositivos
embarcados, uma interface como a apresentada pode ser útil pois não é
necessário instalar muitas ferramentas no dispositivo. Em alguns casos,
no entanto, pode ser mais conveniente o uso de uma ferramenta por linha
de comando que trabalha com o Ftrace, ao invés de termos que interagir
com o mesmo usando comandos em arquivos estranhos e lendo o resultado de
um outro arquivo (como o arquivo *trace* por exemplo).

*trace-cmd* é uma ferramenta de linha de comando que trabalha em espaço
de usuário e serve como uma interface para o Ftrace. Nesse tutorial
iremos instalar essa ferramenta e usá-la juntamente com a ferramenta
*KernelShark* com o objetivo de realizar a depuração de maneira mais
abstrata.

Instalando a ferramenta trace-cmd
=================================

No diretório do *buildroot*, execute *make menuconfig* e selecione a
instalação do pacote *trace-cmd* por meio das opções:

    $ make menuconfig
    Target packages  --->
        Debugging, profiling and benchmark  --->
             [*] trace-cmd

Se necessário, forneça o local com os fontes do kernel:

    $ export LINUX_OVERRIDE_SRCDIR=~/MATRICULA/linux-4.13.9/

Recompile o kernel (apenas os fontes necessários serão compilados) e
monte sua distribuição usando o comando *make* dentro do diretório do
*buildroot*.

Uso da ferramenta trace-cmd
===========================

Um caso simples de uso do trace-cmd é gravar um trace e após apresentar
o mesmo.

    $ trace-cmd record -e ext4 ls
    trace.dat       trace.dat.cpu0
    CPU0 data recorded at offset=0xf4000
        4096 bytes in size
    $ trace-cmd report
    cpus=1
                  ls-81    [000]   374.399512: ext4_es_lookup_extent_enter: dev 8,0 ino 376 lblk 0
                  ls-81    [000]   374.399555: ext4_es_lookup_extent_exit: dev 8,0 ino 376 found 1 [0/1) 2280 W
                  ls-81    [000]   374.399602: ext4_journal_start:   dev 8,0 blocks, 2 rsv_blocks, 0 caller ext4_dirty_inode
                  ls-81    [000]   374.399608: ext4_mark_inode_dirty: dev 8,0 ino 376 caller ext4_dirty_inode

O exemplo acima habilita tracepoints para os eventos do subsistema
*ext4* para o Ftrace, executa o comando *ls* e grava os dados do Ftrace
em um arquivo chamado *trace.dat*. O comando *report* lê o arquivo
*trace.dat* e apresenta a saída em texto simples no terminal. Diversos
tracepoints foram adicionados ao kernel Linux. Esses são agrupados por
subsistema onde pode-se habilitar todos os eventos de um mesmo
subsistema ou habilitar eventos específicos dentro de um subsistema. Por
exemplo, usando a opção \"-e sched\_switch\" irá habilitar o evento
*sched\_switch* enquanto a opçao \"-e sched\" irá habilitar todos os
eventos do subsistema *sched*.

Por padrão, as opções *record* e *report* leem e escrevem no arquivo
*trace.dat*. Pode-se utilizar as opções *-o* e *-i* para escolher um
arquivo diferente para escrita e leitura respectivamente, mas iremos
usar o nome padrão no restante do tutorial.

Durante a gravação de um trace, *trace-cmd* irá realizar um fork de um
processo para cada CPU no sistema. Cada um desses processos irá abrir um
arquivo no tracefs que representa a CPU. Por exemplo, o processo
representando a CPU0 irá abrir o arquivo
*/sys/kernel/tracing/per\_cpu/cpu0/trace\_pipe\_raw*. O arquivo
*trace\_pipe\_raw* é um mapeamento para o buffer interno do Ftrace para
cada CPU. Cada processo irá ler esses arquivos e no final irá concatenar
o conteúdo em um único arquivo *trace.dat*.

Não existe a necessidade de se montar manualmente o sistema de arquivos
tracefs antes de usar a ferramenta trace-cmd. Esta irá procurar onde o
mesmo está montado, e caso não esteja irá montá-lo automaticamente em
*/sys/kernel/tracing*.

Filtrando apenas uma única função
---------------------------------

Como exemplo de uso da ferramenta para filtrar uma única função, iremos
considerar o cenário onde é necessário visualizar quando o kernel está
tratando um *page fault*. No Linux, quando é realizada a alocação de
memória a mesma é feita de maneira preguiçosa. Isso significa que quando
uma aplicação tentar escrever em uma região de memória previamente
alocada, é gerado um *page fault* e o kernel precisa fornecer à
aplicação uma região de memória física para seu uso (disfarçada em uma
região mapeada no espaço de endereçamento virtual do processo).

Para ilustrar esse cenário, usaremos referências à função
*\_\_do\_page\_fault()*:

    $ trace-cmd record -p function -l __do_page_fault

Após executar o comando por alguns segundos (e gerar alguns *page
faults* teclando Ctrl+Z, executando o comando *top*, voltando à
aplicação trace-cmd com o comando *fg* e finalmente teclando Ctrl+C para
terminar o trace) tem-se o seguinte:

    $ trace-cmd report
    cpus=1
           trace-cmd-85    [000]  4102.071745: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072419: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072439: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072474: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072487: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072508: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072513: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072530: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072597: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072611: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072624: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072644: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072656: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072667: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072686: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072692: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072711: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072722: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072768: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072794: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072809: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4102.072988: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4103.171278: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4103.173055: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4103.173230: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4103.173284: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4103.173346: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4104.771472: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4104.771526: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4104.771546: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4104.771575: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4104.771592: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4104.771612: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4104.771636: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4104.771651: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771719: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771740: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771763: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771770: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771775: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771781: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771801: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771815: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771820: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771833: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771839: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771851: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771856: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771879: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771924: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771938: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.771948: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.772002: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.772018: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.772553: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.772650: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.772704: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.772754: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.772872: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.773029: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.773134: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.773417: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.773583: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.773954: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.773971: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.774649: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.775074: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.775223: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.775290: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.775440: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.776223: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.777169: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.777182: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.777709: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.777859: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.777892: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.778005: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.778287: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.778315: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.778510: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.778951: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.779442: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.779474: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.779502: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.779563: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.780573: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.780711: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.782605: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.792554: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.903996: function:             __do_page_fault <-- do_page_fault
                 top-87    [000]  4104.905959: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4106.587672: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4106.587801: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4106.587824: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4106.588001: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4106.588064: function:             __do_page_fault <-- do_page_fault
                  sh-72    [000]  4106.588120: function:             __do_page_fault <-- do_page_fault
           trace-cmd-86    [000]  4111.218628: function:             __do_page_fault <-- do_page_fault

O relatório apresenta o nome dos processos, a CPU associada e a função
que foi monitorada. Podemos visualizar, por exemplo, o número de *page
faults* associados ao processo do shell (que foi invocado ao colocarmos
a ferramenta trace-cmd em background com Ctrl+Z) durante o tempo de
execução do trace:

    $ trace-cmd report | grep sh | wc -l
    19

Para listar todas as funções que podem ser monitoradas, o seguinte
comando pode ser usado:

    $ trace-cmd list -f

Filtrando um único processo
---------------------------

Visualizar apenas uma única função é algo bastante limitado. Digamos que
seja interessante saber tudo o que está acontecendo em um programa.
Podemos, dessa forma, saber tudo o que o kernel está fazendo para que o
programa seja executado. Digamos que o *shell* tenha o PID 72:

    $ trace-cmd record -p function -P 72
    $ trace-cmd report | wc -l
    6852

O comando *report* imprimiu 6852 linhas em poucos segundos.

Usando o modo function\_graph
-----------------------------

O modo de tracing *function\_graph* pode ser usado dentro do trace-cmd.
Esse modo monitora o tempo na entrada e saída de cada função. Um exemplo
de uso, semelhante ao exemplo anterior porém usando o novo modo:

    $ trace-cmd record -p function_graph -P 72
    $ trace-cmd report | head -50
    cpus=1
               <...>-110   [000] 79209.265109: funcgraph_entry:      + 90.930 us  |  switch_mm_irqs_off();
                  sh-72    [000] 79209.265512: funcgraph_entry:                   |  finish_task_switch() {
                  sh-72    [000] 79209.265528: funcgraph_entry:                   |    smp_irq_work_interrupt() {
                  sh-72    [000] 79209.265532: funcgraph_entry:                   |      irq_enter() {
                  sh-72    [000] 79209.265536: funcgraph_entry:        1.205 us   |        rcu_irq_enter();
                  sh-72    [000] 79209.265542: funcgraph_exit:         7.337 us   |      }
                  sh-72    [000] 79209.265548: funcgraph_entry:                   |      __wake_up() {
                  sh-72    [000] 79209.265551: funcgraph_entry:        0.879 us   |        _raw_spin_lock_irqsave();
                  sh-72    [000] 79209.265557: funcgraph_entry:        0.833 us   |        __wake_up_common();
                  sh-72    [000] 79209.265562: funcgraph_entry:        0.933 us   |        _raw_spin_unlock_irqrestore();
                  sh-72    [000] 79209.265567: funcgraph_exit:       + 16.920 us  |      }
                  sh-72    [000] 79209.265570: funcgraph_entry:                   |      __wake_up() {
                  sh-72    [000] 79209.265572: funcgraph_entry:        0.695 us   |        _raw_spin_lock_irqsave();
                  sh-72    [000] 79209.265577: funcgraph_entry:                   |        __wake_up_common() {
                  sh-72    [000] 79209.265583: funcgraph_entry:                   |          autoremove_wake_function() {
                  sh-72    [000] 79209.265586: funcgraph_entry:                   |            default_wake_function() {
                  sh-72    [000] 79209.265589: funcgraph_entry:                   |              try_to_wake_up() {
                  sh-72    [000] 79209.265591: funcgraph_entry:        0.673 us   |                _raw_spin_lock_irqsave();
                  sh-72    [000] 79209.265597: funcgraph_entry:        0.716 us   |                _raw_spin_unlock_irqrestore();
                  sh-72    [000] 79209.265602: funcgraph_exit:       + 10.905 us  |              }
                  sh-72    [000] 79209.265604: funcgraph_exit:       + 15.711 us  |            }
                  sh-72    [000] 79209.265606: funcgraph_exit:       + 21.401 us  |          }
                  sh-72    [000] 79209.265609: funcgraph_exit:       + 29.468 us  |        }
                  sh-72    [000] 79209.265611: funcgraph_entry:        0.640 us   |        _raw_spin_unlock_irqrestore();
                  sh-72    [000] 79209.265615: funcgraph_exit:       + 43.734 us  |      }
                  sh-72    [000] 79209.265619: funcgraph_entry:                   |      irq_exit() {
                  sh-72    [000] 79209.265622: funcgraph_entry:        1.168 us   |        rcu_irq_exit();
                  sh-72    [000] 79209.265627: funcgraph_exit:         6.423 us   |      }
                  sh-72    [000] 79209.265629: funcgraph_exit:       + 98.591 us  |    }
                  sh-72    [000] 79209.265633: funcgraph_exit:       ! 115.951 us |  }
                  sh-72    [000] 79209.265636: funcgraph_entry:        1.016 us   |  _raw_read_lock();
                  sh-72    [000] 79209.265645: funcgraph_entry:                   |  wait_consider_task() {
                  sh-72    [000] 79209.265651: funcgraph_entry:        1.323 us   |    task_stopped_code();
                  sh-72    [000] 79209.265657: funcgraph_entry:        0.945 us   |    _raw_spin_lock_irq();
                  sh-72    [000] 79209.265662: funcgraph_entry:        0.654 us   |    task_stopped_code();
                  sh-72    [000] 79209.265668: funcgraph_entry:        1.387 us   |    __task_pid_nr_ns();
                  sh-72    [000] 79209.265675: funcgraph_exit:       + 28.278 us  |  }
                  sh-72    [000] 79209.265678: funcgraph_entry:                   |  remove_wait_queue() {
                  sh-72    [000] 79209.265681: funcgraph_entry:        0.736 us   |    _raw_spin_lock_irqsave();
                  sh-72    [000] 79209.265687: funcgraph_entry:        0.674 us   |    _raw_spin_unlock_irqrestore();
                  sh-72    [000] 79209.265692: funcgraph_exit:       + 11.219 us  |  }
                  sh-72    [000] 79209.265694: funcgraph_entry:        0.925 us   |  put_pid();
                  sh-72    [000] 79209.265702: funcgraph_entry:                   |  exit_to_usermode_loop() {
                  sh-72    [000] 79209.265705: funcgraph_entry:                   |    do_signal() {
                  sh-72    [000] 79209.265708: funcgraph_entry:                   |      get_signal() {
                  sh-72    [000] 79209.265711: funcgraph_entry:        0.753 us   |        uprobe_deny_signal();
                  sh-72    [000] 79209.265716: funcgraph_entry:        0.889 us   |        _raw_spin_lock_irq();
                  sh-72    [000] 79209.265722: funcgraph_entry:                   |        dequeue_signal() {
                  sh-72    [000] 79209.265725: funcgraph_entry:        0.927 us   |          next_signal();

Monitorando eventos
-------------------

Uma outra classe que pode ser monitorada no sistema são eventos. Um
exemplo de uso já foi anteriormente apresentado no início do tutorial,
mas alguns detalhes serão mostrados. O kernel possui um conjunto de
eventos que podem ser monitorados, com o intuito de facilitar a
depuração quando algumas coisas importantes estiverem acontecendo. Uma
lista de eventos disponíveis pode ser visualizada com:

    $ cat /sys/kernel/tracing/available_events

Para monitorar os eventos relacionados à troca de contexto de processos,
por exemplo, o seguinte comando pode ser usado:

    $ trace-cmd record -e sched:sched_switch
    $ trace-cmd report
    cpus=1
               <...>-94    [000] 77837.045742: sched_switch:         prev_comm=trace-cmd prev_pid=94 prev_prio=120 prev_state=S ==> next_comm=rcu_sched next_pid=8 next_prio=120
           rcu_sched-8     [000] 77837.046388: sched_switch:         prev_comm=rcu_sched prev_pid=8 prev_prio=120 prev_state=S ==> next_comm=ksoftirqd/0 next_pid=7 next_prio=120
         ksoftirqd/0-7     [000] 77837.046509: sched_switch:         prev_comm=ksoftirqd/0 prev_pid=7 prev_prio=120 prev_state=S ==> next_comm=rcu_sched next_pid=8 next_prio=120
           rcu_sched-8     [000] 77837.046528: sched_switch:         prev_comm=rcu_sched prev_pid=8 prev_prio=120 prev_state=S ==> next_comm=trace-cmd next_pid=95 next_prio=120
           trace-cmd-95    [000] 77837.049244: sched_switch:         prev_comm=trace-cmd prev_pid=95 prev_prio=120 prev_state=R ==> next_comm=rcu_sched next_pid=8 next_prio=120
           rcu_sched-8     [000] 77837.049277: sched_switch:         prev_comm=rcu_sched prev_pid=8 prev_prio=120 prev_state=S ==> next_comm=trace-cmd next_pid=95 next_prio=120
           trace-cmd-95    [000] 77837.050556: sched_switch:         prev_comm=trace-cmd prev_pid=95 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77837.053330: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=rcu_sched next_pid=8 next_prio=120
           rcu_sched-8     [000] 77837.053385: sched_switch:         prev_comm=rcu_sched prev_pid=8 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77837.057331: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=rcu_sched next_pid=8 next_prio=120
           rcu_sched-8     [000] 77837.057371: sched_switch:         prev_comm=rcu_sched prev_pid=8 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77837.101327: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77837.101412: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77837.305327: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77837.305455: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77837.509337: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77837.509402: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77837.713347: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77837.713412: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77837.825346: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77837.825410: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77837.917354: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77837.917419: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77838.121362: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77838.121427: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77838.325376: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77838.325441: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77838.529380: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77838.529446: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77838.733391: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77838.733455: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77838.817391: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77838.817443: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77838.937396: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77838.937460: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77839.141405: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77839.141471: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77839.345417: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77839.345483: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77839.549428: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77839.549491: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77839.753437: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77839.753504: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77839.809431: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77839.809477: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77839.957439: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77839.957503: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77840.161454: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77840.161520: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77840.365466: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77840.365531: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77840.569473: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/0:1 next_pid=21 next_prio=120
         kworker/0:1-21    [000] 77840.569537: sched_switch:         prev_comm=kworker/0:1 prev_pid=21 prev_prio=120 prev_state=S ==> next_comm=swapper/0 next_pid=0 next_prio=120
              <idle>-0     [000] 77840.615878: sched_switch:         prev_comm=swapper/0 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=kworker/u2:0 next_pid=5 next_prio=120
        kworker/u2:0-5     [000] 77840.616808: sched_switch:         prev_comm=kworker/u2:0 prev_pid=5 prev_prio=120 prev_state=S ==> next_comm=trace-cmd next_pid=94 next_prio=120

Aplicando filtros
-----------------

A ferramenta trace-cmd permite que sejam aplicados filtros sobre
funções. Além disso, pode-se apresentar uma única função, ou um conjunto
de funções. Por exemplo:

    $ trace-cmd record -p function -l 'sched_*' -n 'sched_avg_update'

A opção *-l* é a mesma que a opção *set\_ftrace\_filter* da ferramenta
Ftrace, e a opção *-n* -e a mesma que colocar o argumento no arquivo
*set\_ftrace\_notrace*. Pode-se ter mais do que uma opção *-l* ou *-n*,
uma vez que a ferramenta trace-cmd simplesmente irá colocar todos os
argumentos no arquivo correto. Um exemplo de uso que apresenta quanto
tempo as interrupções estão levando para serem tradadas pelo kernel é
mostrado a seguir:

    $ trace-cmd record -p function_graph -l do_IRQ -e irq_handler_entry sleep 10
    $ trace-cmd report
    cpus=1
              <idle>-0     [000] 80141.118172: funcgraph_entry:                   |  do_IRQ() {
              <idle>-0     [000] 80141.118270: irq_handler_entry:    irq=4 name=ttyS0
              <idle>-0     [000] 80141.118387: funcgraph_exit:       ! 150.149 us |  }
        kworker/u2:0-5     [000] 80141.118598: funcgraph_entry:                   |  do_IRQ() {
        kworker/u2:0-5     [000] 80141.118610: irq_handler_entry:    irq=4 name=ttyS0
        kworker/u2:0-5     [000] 80141.118641: funcgraph_exit:       + 37.672 us  |  }
        kworker/u2:0-5     [000] 80141.118648: funcgraph_entry:                   |  do_IRQ() {
        kworker/u2:0-5     [000] 80141.118655: irq_handler_entry:    irq=4 name=ttyS0
        kworker/u2:0-5     [000] 80141.118660: funcgraph_exit:         9.678 us   |  }
        kworker/u2:0-5     [000] 80141.118670: funcgraph_entry:                   |  do_IRQ() {
        kworker/u2:0-5     [000] 80141.118676: irq_handler_entry:    irq=4 name=ttyS0
        kworker/u2:0-5     [000] 80141.118683: funcgraph_exit:         9.999 us   |  }
        kworker/u2:0-5     [000] 80144.902474: funcgraph_entry:                   |  do_IRQ() {
        kworker/u2:0-5     [000] 80144.902515: irq_handler_entry:    irq=14 name=ata_piix
        kworker/u2:0-5     [000] 80144.902706: funcgraph_exit:       ! 215.049 us |  }
        kworker/u2:0-5     [000] 80144.903329: funcgraph_entry:                   |  do_IRQ() {
        kworker/u2:0-5     [000] 80144.903344: irq_handler_entry:    irq=14 name=ata_piix
        kworker/u2:0-5     [000] 80144.903360: funcgraph_exit:       + 25.762 us  |  }
        kworker/u2:0-5     [000] 80144.903503: funcgraph_entry:                   |  do_IRQ() {
        kworker/u2:0-5     [000] 80144.903515: irq_handler_entry:    irq=14 name=ata_piix
        kworker/u2:0-5     [000] 80144.903633: funcgraph_exit:       ! 124.486 us |  }
        kworker/u2:0-5     [000] 80144.903661: funcgraph_entry:                   |  do_IRQ() {
        kworker/u2:0-5     [000] 80144.903671: irq_handler_entry:    irq=14 name=ata_piix
        kworker/u2:0-5     [000] 80144.903783: funcgraph_exit:       ! 118.748 us |  }
        kworker/u2:0-5     [000] 80144.903809: funcgraph_entry:                   |  do_IRQ() {
        kworker/u2:0-5     [000] 80144.903819: irq_handler_entry:    irq=14 name=ata_piix
        kworker/u2:0-5     [000] 80144.903931: funcgraph_exit:       ! 117.968 us |  }
              <idle>-0     [000] 80144.904028: funcgraph_entry:                   |  do_IRQ() {
              <idle>-0     [000] 80144.904039: irq_handler_entry:    irq=14 name=ata_piix
              <idle>-0     [000] 80144.904054: funcgraph_exit:       + 22.357 us  |  }
              <idle>-0     [000] 80144.904201: funcgraph_entry:                   |  do_IRQ() {
              <idle>-0     [000] 80144.904213: irq_handler_entry:    irq=14 name=ata_piix
              <idle>-0     [000] 80144.904334: funcgraph_exit:       ! 128.392 us |  }
              <idle>-0     [000] 80144.904361: funcgraph_entry:                   |  do_IRQ() {
              <idle>-0     [000] 80144.904371: irq_handler_entry:    irq=14 name=ata_piix
              <idle>-0     [000] 80144.904511: funcgraph_entry:                   |    do_IRQ() {
              <idle>-0     [000] 80144.904522: irq_handler_entry:    irq=14 name=ata_piix
              <idle>-0     [000] 80144.904536: funcgraph_exit:       + 20.561 us  |    }
              <idle>-0     [000] 80144.904679: funcgraph_entry:                   |    do_IRQ() {
              <idle>-0     [000] 80144.904690: irq_handler_entry:    irq=14 name=ata_piix
              <idle>-0     [000] 80144.904704: funcgraph_exit:       + 21.016 us  |    }
              <idle>-0     [000] 80144.904805: funcgraph_exit:       ! 441.121 us |  }

Os eventos que ocorreram no último trace também podem ser filtrados. O
argumento *\--events* irá listar os formatos de todos os eventos
disponíveis no sistema que criou o arquivo de trace:

    $ trace-cmd report --events
    name: wakeup
    ID: 3
    format:
    	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
    	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
    	field:unsigned char common_preempt_count;	offset:3;	size:1;signed:0;
    	field:int common_pid;	offset:4;	size:4;	signed:1;

    	field:unsigned int prev_pid;	offset:8;	size:4;	signed:0;
    	field:unsigned int next_pid;	offset:12;	size:4;	signed:0;
    	field:unsigned int next_cpu;	offset:16;	size:4;	signed:0;
    	field:unsigned char prev_prio;	offset:20;	size:1;	signed:0;
    	field:unsigned char prev_state;	offset:21;	size:1;	signed:0;
    	field:unsigned char next_prio;	offset:22;	size:1;	signed:0;
    	field:unsigned char next_state;	offset:23;	size:1;	signed:0;

    print fmt: "%u:%u:%u  ==+ %u:%u:%u [%03u]", REC->prev_pid, REC->prev_prio, REC->prev_state, REC->next_pid, REC->next_prio, REC->next_state, REC->next_cpu

    name: user_stack
    ID: 12
    format:
    	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
    	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
    	field:unsigned char common_preempt_count;	offset:3;	size:1;signed:0;
    	field:int common_pid;	offset:4;	size:4;	signed:1;

    	field:unsigned int tgid;	offset:8;	size:4;	signed:0;
    	field:unsigned long caller[8];	offset:12;	size:32;	signed:0;

    print fmt: "\t=> (" "%08lx" ")\n\t=> (" "%08lx" ")\n\t=> (" "%08lx" ")\n" "\t=> (" "%08lx" ")\n\t=> (" "%08lx" ")\n\t=> (" "%08lx" ")\n" "\t=> (" "%08lx" ")\n\t=> (" "%08lx" ")\n", REC->caller[0], REC->caller[1], REC->caller[2], REC->caller[3], REC->caller[4], REC->caller[5], REC->caller[6], REC->caller[7]

    name: raw_data
    ID: 16
    format:
    	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
    	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
    	field:unsigned char common_preempt_count;	offset:3;	size:1;signed:0;
    	field:int common_pid;	offset:4;	size:4;	signed:1;

    	field:unsigned int id;	offset:8;	size:4;	signed:0;
    	field:char buf;	offset:12;	size:0;	signed:1;

    print fmt: "id:%04x %08x", REC->id, (int)REC->buf[0]

    name: print
    ID: 5
    format:
    	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
    	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
    	field:unsigned char common_preempt_count;	offset:3;	size:1;signed:0;
    	field:int common_pid;	offset:4;	size:4;	signed:1;

    	field:unsigned long ip;	offset:8;	size:4;	signed:0;
    	field:char buf;	offset:12;	size:0;	signed:1;

    print fmt: "%ps: %s", (void *)REC->ip, REC->buf

    ...

Visualização de traces
======================

== Exportando o trace para a máquina host ==

Muitas vezes pode ser necessário processar ou acessar o arquivo de trace fora do ambiente embarcado onde o mesmo foi criado. Nem sempre é possível trabalhar diretamente nesse ambiente ou simplesmente não é viável a utilização de uma ferramenta que permite uma análise mais elaborada, como o KernelShark por exemplo.

Uma forma de permitir o acesso ao trace é montar na máquina host o sistema de arquivos //rootfs// (ou um outro disco, se for o caso), copiar o trace e desmontar o sistema de arquivos. O processo de montagem, acesso e desmontagem é necessário pois nem sempre a atualização do arquivo de trace no sistema embarcado emulado é refletida no sistema de arquivos, o que fará que a máquina hospedeira não tenha acesso às atualizações do trace. A sequência é exemplificada dessa forma, e deve ser executada na máquina hospedeira:

```
$ sudo mount output/images/rootfs.ext2 /media/cdrom
$ sudo cp /media/cdrom/root/trace.dat .
$ sudo umount /media/cdrom
```

No exemplo, o sistema de arquivos //rootfs// foi montado em um ponto de montagem já conhecido ///media/cdrom// e o arquivo de trace foi gravado no sistema embarcado no diretório //root//.

== Realizando o tracing pela rede ==

Outra forma de exportar o arquivo de trace seria configurar a máquina emulada com um servidor ssh, e usar a ferramenta //scp// na máquina hospedeira para copiar o arquivo de trace da máquina emulada. Isso não resolve todos os problemas, no entanto.

Existem situações onde pode ser necessário realizar o tracing de um dispositivo embarcado ou alguma máquina com pouco espaço em disco. Provavelmente uma outra máquina possui uma quantidade grande de espaço em disco e pode ser necessário gravar o trace naquala máquina, ou simplesmente é necessário realizar um trace com o mínimo de interferência do sistema de arquivos, por exemplo. É nesse cenário que o tracing pela rede pode ser algo útil.

Para que o tracing possa ser realizado pela rede, é necessário que o ambiente emulado possua acesso à mesma. O processo de configuração do kernel e //buildroot//, assim como os parâmetros a serem usados no //qemu// e scripts, usados para automatizar o processo para que o ambiente embarcado tenha acesso à rede são apresentados em //Tutorial 1.1: Buildroot e QEMU// e //Tutorial 1.2: Configurando a rede//.

Se a ferramenta trace-cmd não estiver instalada na máquina hospedeira, é necessário:

```
$ sudo apt-get install trace-cmd
```

O processo de transferência do trace compreende na cooperação entre um servidor (máquina a receber e processar o trace) e um cliente (máquina sendo monitorada). Para configurar um servidor (na máquina hospedeira), um comando semelhante ao exemplo pode ser usado:

```
$ mkdir tracing
$ trace-cmd listen -p 12345 -D -d ./tracing -l ./tracing/logfile
```

É criado um diretório //tracing// onde os logs serão gravados e no segundo comando é configurado um servidor que irá esperar por uma conexão na porta 12345. A única opção obrigatória é a //-p//. A opção //-D// coloca a ferramenta trace-cmd em modo //daemon//, enquanto a opção //-d ./tracing// coloca os arquivos de trace obtidos a partir de conexões no diretório correspondente. A opção //-l ./tracing/logfile// apenas define que mensagens do trace-cmd não devem ser escritas para o dispositivo de saída padrão (terminal) mas sim para um arquivo.

No dispositivo emulado (ou em outra máquina onde o cliente esteja sendo usado), ao invés de se especificar um arquivo de saída no comando //trace-cmd record// a opção //-N// é usada, seguida por parâmetros contendo o host (máquina hospedeira) e a porta para conexão:

```
$ trace-cmd record -N <IP-DO-HOST>:12345 -e sched_switch -e sched_wakeup -e irq ls
```

Na máquina hospedeira um arquivo é gravado no diretório //tracing// e o formato so nome é semelhante a  "trace.<cliente>:<porta>.dat". No arquivo de log as conexões realizadas pode ser visualizadas:

```
$ cat tracing/logfile
[30209]Connected with 192.168.1.10:56326
[30209]cpus=1
[30209]pagesize=4096
```

O conteúdo do arquivo de trace pode ser processado normalmente na máquina hospedeira:

```
$ trace-cmd report tracing/trace.192.168.1.10\:56326.dat
cpus=1
           <...>-90    [000]   460.662429: softirq_raise:        vec=1 [action=TIMER]
           <...>-90    [000]   460.662450: softirq_raise:        vec=9 [action=RCU]
           <...>-90    [000]   460.662503: sched_wakeup:         trace-cmd:89 [120] success=1 CPU:000
           <...>-90    [000]   460.662512: softirq_entry:        vec=1 [action=TIMER]
           <...>-90    [000]   460.662516: softirq_exit:         vec=1 [action=TIMER]
           <...>-90    [000]   460.662518: softirq_entry:        vec=9 [action=RCU]
           <...>-90    [000]   460.662534: sched_wakeup:         rcu_sched:8 [120] success=1 CPU:000
           <...>-90    [000]   460.662540: softirq_exit:         vec=9 [action=RCU]
           <...>-90    [000]   460.662557: sched_switch:         ls:90 [120] R ==> rcu_sched:8 [120]
       rcu_sched-8     [000]   460.662593: sched_switch:         rcu_sched:8 [120] S ==> trace-cmd:89 [120]
       trace-cmd-89    [000]   460.662652: sched_switch:         trace-cmd:89 [120] S ==> ls:90 [120]
              ls-90    [000]   460.666407: softirq_raise:        vec=1 [action=TIMER]
              ls-90    [000]   460.666414: softirq_raise:        vec=9 [action=RCU]
              ls-90    [000]   460.666432: softirq_entry:        vec=1 [action=TIMER]
              ls-90    [000]   460.666435: softirq_exit:         vec=1 [action=TIMER]
              ls-90    [000]   460.666437: softirq_entry:        vec=9 [action=RCU]
              ls-90    [000]   460.666457: sched_wakeup:         rcu_sched:8 [120] success=1 CPU:000
              ls-90    [000]   460.666475: softirq_exit:         vec=9 [action=RCU]
              ls-90    [000]   460.666488: sched_switch:         ls:90 [120] R ==> rcu_sched:8 [120]
       rcu_sched-8     [000]   460.666515: sched_switch:         rcu_sched:8 [120] S ==> ls:90 [120]
              ls-90    [000]   460.669178: sched_wakeup:         trace-cmd:88 [120] success=1 CPU:000
              ls-90    [000]   460.669197: sched_switch:         ls:90 [120] x ==> trace-cmd:88 [120]
```

trace-cmd é versátil o suficiente para gerenciar sistemas heterogêneos. Toda a informação necessária para a criação e leitura de um trace são passadas do cliente para o servidor, permitindo que as máquinas possuam arquiteturas (como o conjunto de instruções, endianness, entre outros) completamente diferentes.


KernelShark
===========

Um problema real existente em situações onde traces longos precisam ser
analisados é a quantidade de informação armazenada e o tamanho destes. A
ferramenta KernelShark foi construída com o objetivo de simplificar a
análise de traces, permitindo que os mesmos possam ser visualizados
graficamente. Além disso, a ferramenta permite filtrar os traces de
diferentes formas, apresentando os resultados em uma interface bastante
simples e intuitiva.

No site da ferramenta
[KernelShark](http://rostedt.homelinux.com/kernelshark/) são
apresentados diversos casos de uso em conjunto com a ferramenta
trace-cmd. Dessa forma, é mais interessante realizar um estudo desta
referência diretamente.

Para utilizar o KernelShark é necessário que o pacote esteja instalado
na máquina hospedeira. Normalmente não é possível executar o KernelShark
diretamente em um sistema embarcado, pois é necessário que uma
quantidade maior de software seja instalada, além da disponibilidade de
uma interface gráfica. Para instalar o pacote, use o comando (no host):

    $ sudo apt-get install kernelshark

Para iniciar a ferramenta, basta executar a mesma se no diretório
corrente existir um arquivo com o nome *trace.dat*. Se não for esse o
caso, uma vez que os traces podem ter sido trazidos pela rede ao
utilizar-se o trace-cmd no modo cliente-servidor, pode-se apenas passar
o nome do trace como parâmetro:

    $ kernelshark tracing/trace.192.168.1.10\:56326.dat

Conclusão
=========

Nesse tutorial aprendemos como utilizar a ferramenta trace-cmd com o
objetivo de complementar as funcionalidades já providas pela ferramenta
Ftrace. A simplificação da interface para depuração pode ajudar em
situações onde uma análise mais complexa sobre o fluxo de execução dos
sistemas do kernel torna-se necessária. A ferramenta trace-cmd pode ser
usada em conjunto com a ferramenta KernelShark, e dessa forma torna-se
possível realizar uma análise visual e rica em detalhes. Chamadas de
sistema, eventos, execução de tarefas e até mesmo um sistema complexo de
escalonamento de tarefas como implementado no kernel Linux pode ser
depurado com o uso de tais ferramentas, o que possibilita visualizar
graficamente o comportamento do sistema.

Atividade 1
===========

Refaça a Atividade 2 proposta no último tutorial e utilize as
ferramentas trace-cmd e KernelShark pra visualizar o comportamento da
chamada de sistema em uma aplicação exemplo.

Atividade 2
===========

Compile a aplicação abaixo e a inclua em sua distribuição com o objetivo
de verificar o comportamento do escalonador de tarefas. Para isso,
utilize o trace-cmd (incluindo a flag *-e sched\_switch*) e o
KernelShark.

    #include <stdio.h>
    #include <pthread.h>
    #include <semaphore.h>

    #define THREADS		5

    void *task(void *arg){
    	int tid;

    	tid = (int)(long int)arg;

    	while(1){
    		putchar('a' + tid);
    	}
    }

    int main(void){
    	long int i;
    	pthread_t threads[THREADS];

    	for(i = 0; i < THREADS; i++)
    		pthread_create(&threads[i], NULL, task, (void *)i);

    	pthread_exit(NULL);

    	return 0;
    }