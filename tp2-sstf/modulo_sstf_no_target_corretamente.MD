### Guia Rápido: Como Testar o Escalador SSTF (TP2)

Este guia documenta os passos essenciais para carregar seu módulo de kernel e ativar o escalonador de disco SSTF em um ambiente Buildroot.

#### 1. Verificando o Módulo e sua Localização
Após a compilação do seu módulo (`make`), o arquivo `sstf-iosched.ko` é gerado e copiado para o sistema de arquivos de destino do Buildroot. O primeiro passo é verificar se ele está no local correto.

* **Comando:** `find /lib/modules/$(uname -r) -name "sstf-iosched.ko"`
* **Saída Esperada:** `/lib/modules/4.13.0/extra/sstf-iosched.ko`
* **O que isso significa:** `uname -r` retorna a versão do kernel em uso, e este comando confirma que o módulo (`.ko`) foi instalado corretamente na pasta de módulos do sistema.

#### 2. Carregando o Módulo do Kernel
Agora que você sabe que o módulo está no lugar certo, você pode carregá-lo no kernel usando o comando `modprobe`.

* **Comando:** `modprobe sstf-iosched`
* **O que isso significa:** Este comando carrega o módulo `sstf-iosched.ko` na memória do kernel, tornando seu escalonador de disco disponível para o sistema.

#### 3. Verificando os Escalonadores Disponíveis
Após carregar o módulo, o nome do seu escalonador deve aparecer na lista de escalonadores disponíveis para o seu disco (`/dev/sdb`). O nome entre colchetes (`[]`) indica qual é o escalonador atualmente ativo.

* **Comando:** `cat /sys/block/sdb/queue/scheduler`
* **Saída (antes de ativar o SSTF):** `noop deadline [cfq] sstf`
* **O que isso significa:** Seu escalonador `sstf` está disponível, mas o `cfq` ainda é o escalonador padrão.

#### 4. Ativando o Escalador SSTF
Para usar a lógica de escalonamento que você implementou, você precisa instruir o sistema a usar o seu escalonador para o disco.

* **Comando:** `echo sstf > /sys/block/sdb/queue/scheduler`
* **O que isso significa:** Este comando altera o escalonador ativo para o seu escalonador `sstf`.

#### 5. Confirmando a Ativação
Por fim, verifique novamente a lista de escalonadores para confirmar que seu escalonador `sstf` foi ativado com sucesso.

* **Comando:** `cat /sys/block/sdb/queue/scheduler`
* **Saída (após ativar o SSTF):** `noop deadline cfq [sstf]`
* **O que isso significa:** O `[sstf]` indica que seu escalonador de disco agora é o padrão para o dispositivo `/dev/sdb`. A partir deste ponto, qualquer requisição de I/O para este disco passará pelo seu módulo.