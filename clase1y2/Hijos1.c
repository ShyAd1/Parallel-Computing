#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    int pid;
    pid = fork();

    if (pid == -1)
    {
        perror("Error al crear el proceso hijo");
        exit(-1);
    }

    if (pid == 0)
    {
        printf("\n\nHola soy el proceso hijo con PID: %d\n\n", getpid());
        printf("\n\nMi padre es: %d\n\n", getppid());
        sleep(20);
        exit(0);
    }
    else
    {
        printf("\n\nHola soy el proceso padre con PID: %d\n\n", getpid());
        printf("\n\nMi padre es: %d\n\n", getppid());
        sleep(20);
    }
    return 0;
}