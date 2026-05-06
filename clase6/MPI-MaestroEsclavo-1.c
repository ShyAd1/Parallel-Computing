#include <stdio.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char **argv)
{
	int identificador, cuenta;
	char mensaje[50];
	MPI_Status estatus;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &identificador);

	if (identificador == 0)
	{
		printf("Nodo maestro. Mandando el mensaje.\n\n");
		strcpy(mensaje, "Hola mundo desde el nodo maestro\n\n");
		for (int i = 1; i < 8; i++)
		{
			MPI_Send(mensaje, sizeof(mensaje), MPI_CHAR, i, 100, MPI_COMM_WORLD);
		}
	}
	else
	{
		printf("Nodo esclavo %d. Recibiendo el mensaje.\n", identificador);
		MPI_Recv(mensaje, sizeof(mensaje), MPI_CHAR, 0, 100, MPI_COMM_WORLD, &estatus);
		printf("El mensaje es: %s\n", mensaje);
		MPI_Get_count(&estatus, MPI_CHAR, &cuenta);
		printf("Recibidos %d caracteres de %d con tag= %d\n",
			   cuenta,
			   estatus.MPI_SOURCE,
			   estatus.MPI_TAG);
	}
	MPI_Finalize();
}
