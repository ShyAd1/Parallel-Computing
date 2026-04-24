#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void *Hilo1(void *argumentos)
{
   int i;
   for (i = 0; i < 8; i++)
   {
      printf("\nDentro del hilo 1...\n");
      sleep(1);
   }
   pthread_exit(NULL);
}

void *Hilo2(void *argumentos)
{
   int i;
   for (i = 0; i < 8; i++)
   {
      printf("\nDentro del hilo 2...\n");
      sleep(1);
   }
   pthread_exit(NULL);
}

int main()
{
   pthread_t id_hilo1, id_hilo2;

   printf("\nCreacion de los hilos...\n");

   pthread_create(&id_hilo1, NULL, Hilo1, NULL);
   pthread_create(&id_hilo2, NULL, Hilo2, NULL);

   sleep(3);

   printf("\nHilos creados. Esperando su finalizacion...\n");

   pthread_join(id_hilo1, NULL);
   pthread_join(id_hilo2, NULL);

   printf("\nHilos finalizados...\n");
   return 0;
}
