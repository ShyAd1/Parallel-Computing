#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    // Leer archivo de texto y sacar el numero de columnas para crear un arreglo de procesos hijos
    // El archivo es una matriz cuyas columnas estan separadas por comas y filas por saltos de linea
    // Cada columna representa un proceso hijo
    FILE *archivo;
    char linea[100];
    FILE *archivo_resultados;
    int columnas = 0;

    archivo = fopen("matriz.txt", "r");

    if (archivo == NULL)
    {
        perror("Error al abrir el archivo");
        exit(-1);
    }
    else
    {
        fgets(linea, 100, archivo);
        for (int i = 0; linea[i] != '\0'; i++)
        {
            if (linea[i] == ',')
            {
                columnas++;
            }
        }
        columnas++; // Para contar la ultima columna
        fclose(archivo);
    }

    int procesos_hijos[columnas];

    // Crear procesos hijos por columna del archivo
    for (int i = 0; i < columnas; i++)
    {
        procesos_hijos[i] = fork();
        if (procesos_hijos[i] == -1)
        {
            perror("Error al crear el proceso hijo");
            exit(-1);
        }
        // Cada proceso hijo debe de abrir el archivo y leer la columna correspondiente a su numero de proceso
        // Cada proceso hijo debe multiplicar los elementos de la columna que le tocó y el resultado se debe
        // guardar en el archivo de resultados
        if (procesos_hijos[i] == 0)
        {
            // Proceso hijo
            FILE *archivo_hijo;
            char linea_hijo[100];
            int resultado = 1;

            archivo_hijo = fopen("matriz.txt", "r");

            if (archivo_hijo == NULL)
            {
                perror("Error al abrir el archivo");
                exit(-1);
            }
            else
            {
                while (fgets(linea_hijo, 100, archivo_hijo) != NULL)
                {
                    char *token = strtok(linea_hijo, ",");
                    for (int j = 0; j < columnas; j++)
                    {
                        if (j == i)
                        {
                            resultado *= atoi(token);
                            break;
                        }
                        token = strtok(NULL, ",");
                    }
                }
                fclose(archivo_hijo);

                // Guardar resultado en el archivo de resultados
                archivo_resultados = fopen("resultados.txt", "a");
                if (archivo_resultados == NULL)
                {
                    perror("Error al abrir el archivo de resultados");
                    exit(-1);
                }

                fprintf(archivo_resultados, "Columna %d: %d\n", i + 1, resultado);
                fclose(archivo_resultados);
            }

            exit(0);
        }
    }

    for (int i = 0; i < columnas; i++)
    {
        wait(NULL);
    }

    return 0;
}