#include <stdio.h>
#include <mpi.h>

int main(argc, argv)
int argc;
char **argv;
{
    int numero_procesos;
    int identificador;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numero_procesos);
    MPI_Comm_rank(MPI_COMM_WORLD, &identificador);

    printf("Hola mundo desde el proceso %d de %d\n", identificador, numero_procesos);

    MPI_Finalize();
    return 0;
}