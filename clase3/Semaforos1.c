#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>

#define PERMISOS 0644
#define N 5

int Crea_semaforo(key_t llave, int valor_inicial)
{
   int semid = semget(llave, 1, IPC_CREAT | PERMISOS);
   if (semid == -1)
   {
      return -1;
   }
   semctl(semid, 0, SETVAL, valor_inicial);
   return semid;
}

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

int RegionNoCritica()
{
   int pid;
   printf("\nEstoy generando mi identificador: %d\n", getpid());
   pid = getpid();
   sleep(5);
   return pid;
}

void RegionCritica(int dato)
{
   int id_memoria;
   int *dato_memoria;
   key_t llave_memoria;
   llave_memoria = ftok("Archivo_memoria", 'm');
   id_memoria = shmget(llave_memoria, sizeof(int), IPC_CREAT | PERMISOS);
   dato_memoria = shmat(id_memoria, 0, 0);
   printf("\nEstoy en la region critica y mi pid es: %d\n", getpid());
   printf("\nEl dato guardado por el proceso anterior es: %d\n", *dato_memoria);
   *dato_memoria = dato;
   printf("\nNuevo dato guardado en la memoria compartida: %d\n", *dato_memoria);
   sleep(4);
}

int main()
{
   int dato;
   int semaforo_mutex;
   key_t llave;

   llave = ftok("Archivo", 'k');
   semaforo_mutex = Crea_semaforo(llave, 1);

   while (1)
   {
      dato = RegionNoCritica();
      down(semaforo_mutex);
      RegionCritica(dato);
      up(semaforo_mutex);
   }
}
