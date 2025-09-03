#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>

#define __NR_listSleepProcesses 386  // número da syscall definido no tbl

int main() {
    char buf[2048];
    int ret;

    ret = syscall(__NR_listSleepProcesses, buf, sizeof(buf));
    if (ret < 0) {
        perror("syscall");
        return 1;
    }

    buf[ret] = '\0'; // garante string terminada
    printf("Processos em sleep:\n%s\n", buf);

    return 0;
}
