#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
   int shmid1, shmid2, i;
   int *arreglo, *turno;
   key_t llave1, llave2;
   pid_t pid;
   llave1 = ftok("Archivo1", 'k');
   llave2 = ftok("Archivo2", 'k');
   shmid1 = shmget(llave1, 10 * sizeof(int), IPC_CREAT | 0777);
   shmid2 = shmget(llave2, sizeof(int), IPC_CREAT | 0777);
   arreglo = (int *)shmat(shmid1, 0, 0);
   turno = (int *)shmat(shmid2, 0, 0);

   for (i = 0; i < 10; i++)
   {
      while (*turno != 0)
         ;
      printf("\nEl proceso lector dice: %d", arreglo[i]);
      *turno = 1;
      sleep(2);
   }

   printf("\n\nEl lector ha terminado\n\n");
   shmdt(&arreglo);
   shmdt(&turno);
   shmctl(shmid1, IPC_RMID, 0);
   shmctl(shmid2, IPC_RMID, 0);
   return 0;
}
