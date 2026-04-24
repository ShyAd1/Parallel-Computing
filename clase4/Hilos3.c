#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

void *Hilo1(void *argumentos)
{
   int i;
   int *parametros=(int*)argumentos;
   printf("\nEstamos en el hilo...\n");
   printf("\nValor 1: %i\nValor 2: %i\n",*parametros,*(parametros+1));
   *parametros=7;
   *(parametros+1)=8;
   for(i=0;i<3;i++)
   {
      sleep(1);
   }
   pthread_exit(NULL);
}

int main ()
{
   pthread_t id_hilo;
   
   int argumentos[2]={2,3};
   
   printf("\nCreacion del hilo...\n");
   
   pthread_create(&id_hilo,NULL,Hilo1,(void*)argumentos);
   
   printf("\nHilo creado. Esperando su finalizacion...\n");
   
   pthread_join(id_hilo, NULL);
   
   printf("\nHilo finalizado...\nvalor 1: %i\nvalor 2: %i\n",*argumentos,*(argumentos+1));
}


