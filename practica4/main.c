#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

// Funcion para crear un csv dinamico de la matriz
void crear_csv()
{
    // archivo de csv dinamico para crear la matriz de datos
    FILE *archivo = NULL;
    archivo = fopen("matriz.csv", "w");
    // Comprobar si el archivo se creo correctamente
    if (archivo == NULL)
    {
        perror("Error al crear el archivo de resultados");
        exit(-1);
    }
    // Escribir los valores en la matriz en el archivo csv
    for (int i = 1; i <= 9; i++)
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
    for (int i = 1; i <= 17; i += 2)
    {
        if (i == 17)
        {
            fprintf(archivo, "%d", i);
        }
        else
        {
            fprintf(archivo, "%d,", i);
        }
    }
    fprintf(archivo, "\n");
    for (int i = 2; i <= 18; i += 2)
    {
        if (i == 18)
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

// Funcion para contar el numero de filas de la matriz
int contar_filas(FILE *archivo)
{
    // Leer archivo de texto y sacar el numero de filas para crear un arreglo de resultados
    // El archivo es una matriz cuyas columnas estan separadas por comas y filas por saltos de linea
    // Cada fila representa un valor de arreglo de resultados
    char linea[100];
    int filas = 0;

    archivo = fopen("matriz.csv", "r");

    if (archivo == NULL)
    {
        perror("Error al abrir el archivo");
        exit(-1);
    }
    else
    {
        while (fgets(linea, 100, archivo) != NULL)
        {
            filas++;
        }
        fclose(archivo);
    }
    return filas;
}

// Estructura para pasar los datos a cada hilo
typedef struct
{
    int fila;
    int *resultados;
} datos_hilo_t;

// Funcion para multiplicar los valores de cada fila de la matriz y guardar el resultado en el arreglo de resultados
void *multiplicar_fila(void *arg)
{
    // El argumento es un puntero a un entero que representa el numero de fila a multiplicar
    datos_hilo_t *datos = (datos_hilo_t *)arg;
    int fila = datos->fila;
    int resultado = 1;

    FILE *archivo = NULL;
    archivo = fopen("matriz.csv", "r");
    char linea[100];
    int valor;

    if (archivo == NULL)
    {
        perror("Error al abrir el archivo");
        exit(-1);
    }
    else
    {
        for (int i = 0; i <= fila; i++)
        {
            fgets(linea, 100, archivo);
        }
        fclose(archivo);
        char *token = strtok(linea, ",");
        while (token != NULL)
        {
            valor = atoi(token);
            resultado *= valor;
            token = strtok(NULL, ",");
        }
    }
    datos->resultados[fila] = resultado;
    return NULL;
}

int main()
{
    crear_csv();
    FILE *archivo_matriz = fopen("matriz.csv", "r");
    int filas;
    filas = contar_filas(archivo_matriz);
    // printf("El numero de filas de la matriz es: %d\n", filas);

    // Crear un arreglo de resultados de tamaño igual al numero de filas de la matriz
    int resultados[filas];

    // Crear un arreglo de hilos de tamaño igual al numero de filas de la matriz
    pthread_t hilos[filas];

    // Crear arreglo con los datos que recibira cada hilo
    datos_hilo_t datos_hilo[filas];

    // Crear un hilo para cada fila de la matriz y pasar el numero de fila a multiplicar a cada hilo
    for (int i = 0; i < filas; i++)
    {
        datos_hilo[i].fila = i;
        datos_hilo[i].resultados = resultados;
        if (pthread_create(&hilos[i], NULL, multiplicar_fila, (void *)&datos_hilo[i]) != 0)
        {
            perror("Error al crear el hilo");
            exit(-1);
        }
    }
    // Esperar a que todos los hilos terminen y guardar el resultado de cada hilo en el arreglo de resultados
    for (int i = 0; i < filas; i++)
    {
        if (pthread_join(hilos[i], NULL) != 0)
        {
            perror("Error al unir el hilo");
            exit(-1);
        }
    }

    // Imprimir el resultado de cada hilo
    for (int i = 0; i < filas; i++)
    {
        printf("El resultado de multiplicar la fila %d es: %d\n", i + 1, resultados[i]);
    }

    // Guardar los resultados en un archivo csv
    FILE *archivo_resultados = fopen("resultados.csv", "w");
    if (archivo_resultados == NULL)
    {
        perror("Error al crear el archivo de resultados");
        exit(-1);
    }
    fprintf(archivo_resultados, "Fila,Resultado\n");
    for (int i = 0; i < filas; i++)
    {
        fprintf(archivo_resultados, "%d,%d\n", i + 1, resultados[i]);
    }
    fclose(archivo_resultados);

    return 0;
}