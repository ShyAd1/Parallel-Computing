#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    int pid1, pid2;
    pid1 = fork();

    if (pid1 == -1)
    {
        perror("Error al crear el primer proceso hijo");
        exit(-1);
    }

    if (pid1 == 0)
    {
        printf("\n\nHola soy el primer proceso hijo con PID: %d\n\n", getpid());
        printf("\n\nMi padre es: %d\n\n", getppid());
        sleep(20);
        exit(0);
    }

    pid2 = fork();

    if (pid2 == -1)
    {
        perror("Error al crear el segundo proceso hijo");
        exit(-1);
    }

    if (pid2 == 0)
    {
        printf("\n\nHola soy el segundo proceso hijo con PID: %d\n\n", getpid());
        printf("\n\nMi padre es: %d\n\n", getppid());
        sleep(20);
        exit(0);
    }

    sleep(20);
    return 0;
}