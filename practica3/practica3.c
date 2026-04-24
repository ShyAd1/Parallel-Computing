#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PERMISOS 0666

void crear_archivo_matriz(FILE *archivo)
{
    // Crear archivo de texto de la matriz
    archivo = fopen("matriz.txt", "w");
    if (archivo == NULL)
    {
        perror("Error al crear el archivo de la matriz");
        exit(-1);
    }
    fprintf(archivo, "1,2,3,4,5,6,7,8,9\n1,3,5,7,9,11,13,15,17\n2,4,6,8,10,12,14,16,18\n");
    fclose(archivo);
}

int contar_filas(FILE *archivo)
{
    // Leer archivo de texto y sacar el numero de filas para crear un arreglo de resultados
    // El archivo es una matriz cuyas columnas estan separadas por comas y filas por saltos de linea
    // Cada fila representa un valor de arreglo de resultados
    char linea[100];
    int filas = 0;

    archivo = fopen("matriz.txt", "r");

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

int contar_columnas(FILE *archivo)
{
    // Leer archivo de texto y sacar el numero de columnas para crear un arreglo de resultados
    // El archivo es una matriz cuyas columnas estan separadas por comas y filas por saltos de linea
    // Cada fila representa un valor de arreglo de resultados
    char linea[100];
    int columnas = 0;

    archivo = fopen("matriz.txt", "r");

    if (archivo == NULL)
    {
        perror("Error al abrir el archivo");
        exit(-1);
    }
    else
    {
        if (fgets(linea, 100, archivo) != NULL)
        {
            char *token = strtok(linea, ",");
            while (token != NULL)
            {
                columnas++;
                token = strtok(NULL, ",");
            }
        }
        fclose(archivo);
    }
    return columnas;
}

int crea_semaforo(key_t llave, int valor_inicial)
{
    int semid = semget(llave, 1, IPC_CREAT | PERMISOS);
    if (semid == -1)
    {
        return -1;
    }
    semctl(semid, 0, SETVAL, valor_inicial);
    return semid;
}

void down(int semid)
{
    struct sembuf op_p[] = {0, -1, 0};
    semop(semid, op_p, 1);
}

void up(int semid)
{
    struct sembuf op_v[] = {0, +1, 0};
    semop(semid, op_v, 1);
}

int region_no_critica(int i, int fila, int columnas, FILE *archivo)
{
    // Lee la fila indicada y devuelve el valor de la columna i para que se multipleque en otro archivo
    char linea[100];
    int arreglo[columnas];

    archivo = fopen("matriz.txt", "r");

    if (archivo == NULL)
    {
        perror("Error al abrir el archivo");
        exit(-1);
    }
    else
    {
        int contador_filas = 0;
        while (fgets(linea, 100, archivo) != NULL)
        {
            if (contador_filas == fila)
            {
                char *token = strtok(linea, ",");
                int j = 0;
                while (token != NULL && j < columnas)
                {
                    arreglo[j] = atoi(token);
                    token = strtok(NULL, ",");
                    j++;
                }
                break;
            }
            contador_filas++;
        }
        fclose(archivo);
    }

    return arreglo[i];
}

int main()
{
    // Crear archivo de texto de la matriz
    FILE *archivo_matriz;
    crear_archivo_matriz(archivo_matriz);

    // Contar el numero de filas y columnas de la matriz
    int filas = contar_filas(archivo_matriz);

    int columnas = contar_columnas(archivo_matriz);

    int dato[columnas];
    int semaforo_vacio, semaforo_lleno, semaforo_mutex, semaforo_fila1, semaforo_fila2;
    key_t llave_vacio, llave_lleno, llave_mutex, llave_fila1, llave_fila2, llave_final;

    llave_vacio = ftok("Archivo", 'v');
    llave_lleno = ftok("Archivo", 'l');
    llave_mutex = ftok("Archivo", 'm');
    llave_fila1 = ftok("Archivo", '1');
    llave_fila2 = ftok("Archivo", '2');
    semaforo_vacio = crea_semaforo(llave_vacio, columnas); // Inicializa con el número de columnas para permitir llenar la memoria compartida
    semaforo_lleno = crea_semaforo(llave_lleno, 0);        // Inicializa en 0 porque al principio no hay datos en la memoria compartida
    semaforo_mutex = crea_semaforo(llave_mutex, 1);        // Inicializa en 1 para permitir el acceso a la memoria compartida
    semaforo_fila1 = crea_semaforo(llave_fila1, 0);
    semaforo_fila2 = crea_semaforo(llave_fila2, 0);

    // Crea la memoria compartida una vez
    key_t llave1, llave2;
    llave1 = ftok("Archivo", 'k');
    llave2 = ftok("Archivo", 'q');
    int shmid1 = shmget(llave1, columnas * sizeof(int), IPC_CREAT | 0777); // Crea la memoria compartida para una fila (columnas) y se reutiliza para las otras filas
    int shmid2 = shmget(llave2, filas * sizeof(int), IPC_CREAT | 0777);    // Crea la memoria compartida para los resultados de las filas
    int *memoria_compartida = (int *)shmat(shmid1, 0, 0);
    int *resultado = (int *)shmat(shmid2, 0, 0);
    for (int i = 0; i < filas; i++)
    {
        resultado[i] = 1; // Inicializa una vez
    }

    for (int i = 0; i < columnas; i++)
    {
        dato[i] = region_no_critica(i, 0, columnas, archivo_matriz);
    }
    for (int i = 0; i < columnas; i++)
    {
        down(semaforo_vacio); // Espera un espacio vacío
        down(semaforo_mutex); // Entra a la sección crítica
        // Escribe directamente en la memoria compartida existente
        memoria_compartida[i] = dato[i];
        // printf("\nEl proceso principal ha escrito el valor %d en la memoria compartida para la fila 0\n", dato[i]);
        up(semaforo_mutex); // Sale de la sección crítica
        up(semaforo_lleno); // Indica que hay un elemento lleno
        sleep(1);
    }
    up(semaforo_fila1); // Despierta al proceso de la fila 1
    for (int i = 0; i < columnas; i++)
    {
        dato[i] = region_no_critica(i, 1, columnas, archivo_matriz);
    }
    for (int i = 0; i < columnas; i++)
    {
        down(semaforo_vacio); // Espera un espacio vacío
        down(semaforo_mutex); // Entra a la sección crítica
        // Escribe directamente en la memoria compartida existente
        memoria_compartida[i] = dato[i];
        // printf("\nEl proceso principal ha escrito el valor %d en la memoria compartida para la fila 1\n", dato[i]);
        up(semaforo_mutex); // Sale de la sección crítica
        up(semaforo_lleno); // Indica que hay un elemento lleno
        sleep(1);
    }
    up(semaforo_fila2); // Despierta al proceso de la fila 2
    for (int i = 0; i < columnas; i++)
    {
        dato[i] = region_no_critica(i, 2, columnas, archivo_matriz);
    }
    for (int i = 0; i < columnas; i++)
    {
        down(semaforo_vacio); // Espera un espacio vacío
        down(semaforo_mutex); // Entra a la sección crítica
        // Escribe directamente en la memoria compartida existente
        memoria_compartida[i] = dato[i];
        // printf("\nEl proceso principal ha escrito el valor %d en la memoria compartida para la fila 2\n", dato[i]);
        up(semaforo_mutex); // Sale de la sección crítica
        up(semaforo_lleno); // Indica que hay un elemento lleno
        sleep(1);
    }

    printf("\nEl proceso principal ha terminado de escribir los datos en la memoria compartida.\n");
    for (int i = 0; i < filas; i++)
    {
        printf("Resultado fila %d: %d\n", i + 1, resultado[i]);
    }

    // Guardar resultado en un archivo de texto resultados.txt
    FILE *archivo_resultados = fopen("resultados.txt", "w");
    if (archivo_resultados == NULL)
    {
        perror("Error al abrir el archivo de resultados");
        exit(1);
    }
    for (int i = 0; i < filas; i++)
    {
        fprintf(archivo_resultados, "Resultado fila %d: %d\n", i + 1, resultado[i]);
    }
    fclose(archivo_resultados);

    // Limpieza al final
    shmdt(memoria_compartida);
    shmdt(resultado);
    shmctl(shmid1, IPC_RMID, 0);
    shmctl(shmid2, IPC_RMID, 0);

    return 0;
}