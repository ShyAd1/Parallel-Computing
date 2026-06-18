#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<cuda_runtime.h>
#include<device_launch_parameters.h>

#define Tam 10

__global__ void MultiplicaMatriz(int *A, int *B, int *C, int tam)
{
   printf("\nRealizando trabajo en el hilo (%d, %d) del bloque (%d, %d)",threadIdx.x,threadIdx.y,blockIdx.x,blockIdx.y);

   int fila = blockIdx.y*blockDim.y+threadIdx.y;
   int col  = blockIdx.x*blockDim.x+threadIdx.x;

   if(fila<tam && col<tam)
   {
      int suma=0;
      int k;
      for(k=0;k<tam;k++)
      {
         suma += A[fila*tam+k]*B[k*tam+col];
      }
      C[fila*tam+col]=suma;
   }
}

int main()
{
   int i,j;
   int A[Tam][Tam];
   int B[Tam][Tam];
   int C[Tam][Tam];
   int *devA, *devB, *devC;

   //Cada bloque tendra 5x5 hilos, y se usaran 2x2 bloques para cubrir la matriz 10x10
   dim3 HilosPorBloque(5,5);
   dim3 BloquesPorGrid(Tam/HilosPorBloque.x,Tam/HilosPorBloque.y);

   srand(time(NULL));

   printf("\nInicializando datos desde la CPU\n\n");

   //Se llenan las matrices A y B con numeros aleatorios entre 0 y 9
   for(i=0;i<Tam;i++)
   {
      for(j=0;j<Tam;j++)
      {
         A[i][j]=rand()%10;
         B[i][j]=rand()%10;
         C[i][j]=0;
      }
   }

   printf("Matriz A:\n\n");
   for(i=0;i<Tam;i++)
   {
      for(j=0;j<Tam;j++)
      {
         printf("%d\t",A[i][j]);
      }
      printf("\n");
   }

   printf("\nMatriz B:\n\n");
   for(i=0;i<Tam;i++)
   {
      for(j=0;j<Tam;j++)
      {
         printf("%d\t",B[i][j]);
      }
      printf("\n");
   }

   printf("\nMatriz C (inicial):\n\n");
   for(i=0;i<Tam;i++)
   {
      for(j=0;j<Tam;j++)
      {
         printf("%d\t",C[i][j]);
      }
      printf("\n");
   }

   cudaMalloc((void**)&devA,Tam*Tam*sizeof(int));
   cudaMalloc((void**)&devB,Tam*Tam*sizeof(int));
   cudaMalloc((void**)&devC,Tam*Tam*sizeof(int));

   printf("\nCopiando datos al dispositivo\n");

   cudaMemcpy(devA,A,Tam*Tam*sizeof(int),cudaMemcpyHostToDevice);
   cudaMemcpy(devB,B,Tam*Tam*sizeof(int),cudaMemcpyHostToDevice);
   cudaMemcpy(devC,C,Tam*Tam*sizeof(int),cudaMemcpyHostToDevice);

   printf("\nIniciando procesamiento paralelo en el dispositivo\n");

   MultiplicaMatriz<<<BloquesPorGrid,HilosPorBloque>>>(devA,devB,devC,Tam);

   cudaDeviceSynchronize();

   cudaMemcpy(C,devC,Tam*Tam*sizeof(int),cudaMemcpyDeviceToHost);

   printf("\n\nMostrando resultado de la multiplicacion (Matriz C) desde la CPU\n\n");

   for(i=0;i<Tam;i++)
   {
      for(j=0;j<Tam;j++)
      {
         printf("%d\t",C[i][j]);
      }
      printf("\n");
   }

   cudaFree(devA);
   cudaFree(devB);
   cudaFree(devC);

   cudaDeviceReset();

   return 0;
}
