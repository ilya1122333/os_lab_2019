#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("Fork failed");
        return 1;
    } else if (pid == 0) {
        argv[0] = "./sequential_min_max";

        execv(argv[0], argv);

        perror("Ошибка запуска execv");
        return 1;
    } else {
        int status;
        
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Дочерний процесс успешно завершился\n");
        } else {
            printf("Дочерний процесс завершился с ошибкой.\n");
        }
    }

    return 0;
}