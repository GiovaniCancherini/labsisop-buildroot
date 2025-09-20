#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 256

int main() {
    int fd;
    char receive[BUFFER_LENGTH];
    char stringToSend[BUFFER_LENGTH];

    printf("Starting device test...\n");

    fd = open("/dev/simple_driver_atividade2", O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device...");
        return errno;
    }

    while (1) {
        printf("\nDigite uma string para enviar (ou 'exit' para sair):\n> ");
        if (!fgets(stringToSend, BUFFER_LENGTH, stdin))
            break;

        // remove \n do final
        stringToSend[strcspn(stringToSend, "\n")] = '\0';

        if (strcmp(stringToSend, "exit") == 0)
            break;

        // escreve no driver
        if (write(fd, stringToSend, strlen(stringToSend)) < 0) {
            perror("Erro ao escrever no device");
            break;
        }
        printf("Mensagem enviada!\n");

        // lê de volta
        int ret = read(fd, receive, BUFFER_LENGTH - 1);
        if (ret < 0) {
            perror("Erro ao ler do device");
            break;
        }
        receive[ret] = '\0';
        printf("Mensagem recebida: [%s]\n", receive);
    }

    close(fd);
    printf("End of program.\n");
    return 0;
}
