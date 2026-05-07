#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Crear un csv dinamico de la matriz
void crear_csv()
{
    FILE *archivo = NULL;
    archivo = fopen("matriz.csv", "w");
    if (archivo == NULL)
    {
        perror("Error al crear el archivo de resultados");
        exit(-1);
    }
    for (int i = 1; i <= 3; i++)
    {
        if (i == 3)
        {
            fprintf(archivo, "%d", i);
        }
        else
        {
            fprintf(archivo, "%d,", i);
        }
    }
    fprintf(archivo, "\n");
    for (int i = 4; i <= 6; i++)
    {
        if (i == 6)
        {
            fprintf(archivo, "%d", i);
        }
        else
        {
            fprintf(archivo, "%d,", i);
        }
    }
    fprintf(archivo, "\n");
    for (int i = 7; i <= 9; i++)
    {
        if (i == 9)
        {
            fprintf(archivo, "%d", i);
        }
        else
        {
            fprintf(archivo, "%d,", i);
        }
    }
    fprintf(archivo, "\n");
    for (int i = -6; i >= -12; i -= 3)
    {
        if (i == -12)
        {
            fprintf(archivo, "%d", i);
        }
        else
        {
            fprintf(archivo, "%d,", i);
        }
    }
    fprintf(archivo, "\n");
    for (int i = 5; i <= 15; i += 5)
    {
        if (i == 15)
        {
            fprintf(archivo, "%d", i);
        }
        else
        {
            fprintf(archivo, "%d,", i);
        }
    }
    fprintf(archivo, "\n");
    for (int i = 22; i <= 36; i += 7)
    {
        if (i == 36)
        {
            fprintf(archivo, "%d", i);
        }
        else
        {
            fprintf(archivo, "%d,", i);
        }
    }
    fprintf(archivo, "\n");
    for (int i = 8; i <= 24; i += 8)
    {
        if (i == 24)
        {
            fprintf(archivo, "%d", i);
        }
        else
        {
            fprintf(archivo, "%d,", i);
        }
    }
    fprintf(archivo, "\n");
    fclose(archivo);
}

int contar_filas(FILE *archivo)
{
    int filas = 0;
    char linea[100];
    while (fgets(linea, sizeof(linea), archivo) != NULL)
    {
        filas++;
    }
    return filas;
}

int operacion_por_fila(int fila)
{
    FILE *archivo = NULL;
    archivo = fopen("matriz.csv", "r");
    char linea[100];
    int valor;
    int resultado = 1;

    if (archivo == NULL)
    {
        perror("Error al abrir el archivo");
        exit(-1);
    }
    else
    {
        // obtener la fila correspondiente y realizar una operacion por cada fila
        for (int i = 0; i <= fila; i++)
        {
            if (fgets(linea, sizeof(linea), archivo) == NULL)
            {
                perror("Error al leer una fila del archivo");
                fclose(archivo);
                exit(-1);
            }
        }
        fclose(archivo);
        char *token = strtok(linea, ",");

        // Pasar token a arreglo para poder hacer operaciones mas complejas
        // Al ser 3 valores por fila, se puede hacer a - b + c o a * b - b * c o cualquier combinacion de operaciones
        int valores[3];
        int i = 0;
        while (token != NULL && i < 3)
        {
            valor = atoi(token);
            valores[i] = valor;
            token = strtok(NULL, ",");
            i++;
        }
        if (fila == 0)
        {
            resultado = valores[0] + valores[1] + valores[2];
        }
        else if (fila == 1)
        {
            resultado = valores[0] - valores[1] + valores[2];
        }
        else if (fila == 2)
        {
            resultado = valores[0] * valores[1] * valores[2];
        }
        else if (fila == 3)
        {
            resultado = valores[0] * (valores[1] + valores[2]);
        }
        else if (fila == 4)
        {
            resultado = valores[0] * valores[1] - valores[1] * valores[2];
        }
        else if (fila == 5)
        {
            resultado = valores[2] * ((valores[1] + valores[2]) * (valores[1] + valores[2]));
        }
        else
        {
            resultado = (valores[0] * valores[0]) - (valores[1] * valores[1]) + (valores[2] * valores[2] * valores[2]);
        }
    }
    return resultado;
}

// FUncion main para crear un proceso por cada fila de la matriz y realizar una operacion por cada fila, luego guardar los resultados en un archivo csv
int main(int argc, char *argv[])
{
    int pid, procesos, filas, i;
    int resultado_local = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &procesos);

    if (pid == 0)
    {
        crear_csv();
    }

    MPI_Barrier(MPI_COMM_WORLD);

    FILE *archivo = fopen("matriz.csv", "r");
    if (archivo == NULL)
    {
        perror("Error al abrir el archivo");
        MPI_Finalize();
        exit(-1);
    }

    filas = contar_filas(archivo);
    fclose(archivo);

    if (procesos != filas)
    {
        if (pid == 0)
        {
            fprintf(stderr, "Error: se requieren %d procesos MPI para calcular %d filas.\n", filas, filas);
        }
        MPI_Finalize();
        return 1;
    }

    int resultados[filas];
    for (i = 0; i < filas; i++)
    {
        resultados[i] = 0;
    }

    if (pid < filas)
    {
        resultado_local = operacion_por_fila(pid);
    }

    MPI_Gather(&resultado_local, 1, MPI_INT, resultados, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (pid == 0)
    {
        FILE *archivo_resultados = fopen("resultados.csv", "w");
        if (archivo_resultados == NULL)
        {
            perror("Error al crear el archivo de resultados");
            MPI_Finalize();
            exit(-1);
        }

        for (i = 0; i < filas; i++)
        {
            if (i == filas - 1)
            {
                fprintf(archivo_resultados, "%d", resultados[i]);
            }
            else
            {
                fprintf(archivo_resultados, "%d,", resultados[i]);
            }
        }
        fprintf(archivo_resultados, "\n");
        fclose(archivo_resultados);
    }

    MPI_Finalize();
    return 0;
}