#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
 
#define MAX_THREADS 10

void Hilo1(void)
{
   printf("\nHilo: %ld\n",pthread_self());
   pthread_exit(0);
}

int main()
{
   int j;
   
   pthread_attr_t atributos;
   
   pthread_t identificadores[MAX_THREADS];
   
   pthread_attr_init(&atributos);
   
   pthread_attr_setdetachstate(&atributos,PTHREAD_CREATE_DETACHED);
   
   for(j=0;j<MAX_THREADS;j++)
   {
      pthread_create(&identificadores[j],&atributos,(void*)Hilo1,NULL);
   }
   
   sleep(1);
   return 0;
}
