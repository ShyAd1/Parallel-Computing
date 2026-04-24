#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

void down(int semid)
{
    struct sembuf op_p[] = {0, -1, 0};
    semop(semid, op_p, 1);
}

void up(int semid)
{
    struct sembuf op_v[] = {0, +1, 0};
    semop(semid, op_v, 1);
}

int main()
{
    int semaforo_fila1 = semget(ftok("Archivo", '1'), 1, 0);
    down(semaforo_fila1); // Espera a que el proceso principal haga 'up'

    int shmid1, shmid2;
    int *memoria_compartida, *resultado;
    key_t llave1, llave2;
    pid_t pid;
    llave1 = ftok("Archivo", 'k');
    llave2 = ftok("Archivo", 'q');
    shmid1 = shmget(llave1, 9 * sizeof(int), 0666); // Crea la memoria compartida para una fila (columnas) y se reutiliza para las otras filas
    shmid2 = shmget(llave2, 3 * sizeof(int), 0666); // Crea la memoria compartida para los resultados de las filas
    memoria_compartida = (int *)shmat(shmid1, 0, 0);
    resultado = (int *)shmat(shmid2, 0, 0);
    int semaforo_vacio = semget(ftok("Archivo", 'v'), 1, 0); // Se obtiene el semáforo vacío
    int semaforo_lleno = semget(ftok("Archivo", 'l'), 1, 0); // Se obtiene el semáforo lleno
    int semaforo_mutex = semget(ftok("Archivo", 'm'), 1, 0); // Se obtiene el semáforo mutex

    for (int i = 0; i < 9; i++)
    {
        down(semaforo_lleno); // Espera un elemento lleno
        down(semaforo_mutex); // Entra a la sección crítica
        resultado[1] *= memoria_compartida[i];
        // printf("\nEl resultado parcial de la segunda fila es: %d\n", resultado[1]);
        up(semaforo_mutex); // Sale de la sección crítica
        up(semaforo_vacio); // Libera un espacio vacío
        sleep(1);
    }
    // Imprime el resultado con un PID xxxx y creado en la marca de tiempo xxxx
    printf("\nEl resultado de la segunda fila con PID %d de la multiplicacion es: %d\n", getpid(), resultado[1]);
    time_t tiempo_final = time(NULL);
    printf("\nEl proceso ha terminado en el tiempo: %s\n", ctime(&tiempo_final));

    return 0;
}