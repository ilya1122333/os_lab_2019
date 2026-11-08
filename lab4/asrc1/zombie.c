#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    pid_t child_pid = fork();

    if (child_pid > 0) {
        printf("Родительский процесс (PID: %d) запущен.\n", getpid());
        printf("Создан дочерний процесс с PID: %d\n", child_pid);

        sleep(30); 
        
        printf("Родительский процесс завершает работу.\n");
    } 
    else if (child_pid == 0) {
        printf("Дочерний процесс (PID: %d) запущен и немедленно завершается.\n", getpid());
        exit(0);
    } 
    else {
        perror("Ошибка fork");
        return 1;
    }

    return 0;
}