#include <mpi.h>
#include <stdio.h>

#define N 10

int main (int argc, char **argv)
{
   int pid,procesos,origen,destino,datos;
   int arreglo[N],i;
   MPI_Status estatus;
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD,&pid);
   MPI_Comm_size(MPI_COMM_WORLD,&procesos);
   
   for(i=0;i<N;i++)
      arreglo[i] = 0;
   
   if(pid==0)
   {
      for(i=0;i<N;i++)
         arreglo[i] = i;
      destino=1;
      MPI_Send(&arreglo[0],N,MPI_INT,destino,0,MPI_COMM_WORLD);
      for(i=0;i<N;i++)
         arreglo[i] = i*3;
      destino=2;
      MPI_Send(&arreglo[0],N,MPI_INT,destino,0,MPI_COMM_WORLD);
      for(i=0;i<N;i++)
         arreglo[i] = i*9;
      destino=3;
      MPI_Send(&arreglo[0],N,MPI_INT,destino,10,MPI_COMM_WORLD);
   }
   else if(pid==1)
   {
      printf("\nValor del arreglo en Procesador 1 antes de recibir datos\n");
      for(i=0;i<N;i++) 
         printf("%4d",arreglo[i]);
      printf("\n");
      origen=0; 
      MPI_Recv(&arreglo[0],N,MPI_INT,origen,0,MPI_COMM_WORLD,&estatus);
      MPI_Get_count(&estatus,MPI_INT,&datos);
      printf("Procesador 1 recibe el arreglo del Procesador %d: etiqueta %d, datos %d\n",
      		estatus.MPI_SOURCE,estatus.MPI_TAG,datos);
      for (i=0;i<datos;i++)
         printf("%4d",arreglo[i]);
      printf("\n");
   }
   else if(pid==2)
   {
      printf("\nValor del arreglo en Procesador 2 antes de recibir datos\n");
      for(i=0;i<N;i++) 
         printf("%4d",arreglo[i]);
      printf("\n");
      origen=0; 
      MPI_Recv(&arreglo[0],N,MPI_INT,origen,0,MPI_COMM_WORLD,&estatus);
      MPI_Get_count(&estatus,MPI_INT,&datos);
      printf("Procesador 2 recibe el arreglo del Procesador %d: etiqueta %d, datos %d\n",
      		estatus.MPI_SOURCE,estatus.MPI_TAG,datos);
      for (i=0;i<datos;i++)
         printf("%4d",arreglo[i]);
      printf("\n");
   }
   else if(pid==3)
   {
      printf("\nValor del arreglo en Procesador 3 antes de recibir datos\n");
      for(i=0;i<N;i++) 
         printf("%4d",arreglo[i]);
      printf("\n");
      origen=0; 
      MPI_Recv(&arreglo[0],N,MPI_INT,origen,10,MPI_COMM_WORLD,&estatus);
      MPI_Get_count(&estatus,MPI_INT,&datos);
      printf("Procesador 3 recibe el arreglo del Procesador %d: etiqueta %d, datos %d\n",
      		estatus.MPI_SOURCE,estatus.MPI_TAG,datos);
      for (i=0;i<datos;i++)
         printf("%4d",arreglo[i]);
      printf("\n");
   }
   MPI_Finalize();
} 

