#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define PI 3.14159265358979323846
#define PERMISOS 0666
#define PASO 0.1
#define A0_1 -(pow(PI, 2) / 3)
#define A0_2 ((6 - 4 * pow(PI, 2)) / 6)
#define A0_3 -(1 / 4)
#define A0_4 (pow(PI, 4) / 5)

double funcion_fourier(double x, int n, int opcion_funcion)
{
    switch (opcion_funcion)
    {
    case 0:
        return -(4 * pow(-1, n) / pow(n, 2)) * cos(n * x) - (2 * pow(-1, n) / n) * sin(n * x);
        break;
    case 1:
        return -((8 / pow(n, 2)) * cos(n * PI) * cos(n * x)) + ((2 / n) * cos(n * PI) * sin(n * x));
        break;
    case 2:
        return ((pow(-1, n + 1)) / n) * sin(n * x);
        break;
    case 3:
        return ((8 * pow(-1, n) * (pow(n * PI, 2) - 6)) / pow(n, 4)) * cos(n * x) + ((2 * pow(-1, n)) / n) * sin(n * x);
    default:
        return 0.0;
    }
}

void crear_csv(int n, int opcion_funcion)
{
    // archivo de csv dinamico para guardar los resultados de la funcion de fourier
    FILE *archivo = NULL;
    switch (opcion_funcion)
    {
    case 0:
        archivo = fopen("resultados_funcion1.csv", "w");
        break;
    case 1:
        archivo = fopen("resultados_funcion2.csv", "w");
        break;
    case 2:
        archivo = fopen("resultados_funcion3.csv", "w");
        break;
    case 3:
        archivo = fopen("resultados_funcion4.csv", "w");
        break;
    default:
        break;
    }
    // Comprobar si el archivo se creo correctamente
    if (archivo == NULL)
    {
        perror("Error al crear el archivo de resultados");
        exit(-1);
    }
    // Escribir el encabezado del archivo de resultados
    // Primera fila se pondra x y luego el numero de armonicos
    // Exactamente de esta forma x,1,2,3,...,n,f
    fprintf(archivo, "x");
    for (int i = 1; i <= n; i++)
    {
        fprintf(archivo, ",%d", i);
    }
    fprintf(archivo, ",F\n");
    fclose(archivo);
}

FILE *abrir_archivo_resultados(int opcion_funcion)
{
    FILE *archivo = NULL;
    switch (opcion_funcion)
    {
    case 0:
        archivo = fopen("resultados_funcion1.csv", "a");
        break;
    case 1:
        archivo = fopen("resultados_funcion2.csv", "a");
        break;
    case 2:
        archivo = fopen("resultados_funcion3.csv", "a");
        break;
    case 3:
        archivo = fopen("resultados_funcion4.csv", "a");
        break;
    default:
        break;
    }
    // Comprobar si el archivo se abrio correctamente
    if (archivo == NULL)
    {
        perror("Error al abrir el archivo de resultados");
        exit(-1);
    }
    return archivo;
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
    struct sembuf op_p[] = {{0, -1, 0}};
    semop(semid, op_p, 1);
}

void up(int semid)
{
    struct sembuf op_v[] = {{0, +1, 0}};
    semop(semid, op_v, 1);
}

int calcular_procesos_hijos(double pi, double paso)
{
    return (int)ceil(2 * pi / paso) + 1; // Se suma 1 para incluir el valor de x = pi
}

int main()
{
    // Region no critica para crear el archivo de la matriz,
    // calcular el numero de procesos hijos, crear la memoria compartida y los semaforos
    int numero_armonicos;
    printf("Ingrese el numero de armonicos a calcular: ");
    scanf("%d", &numero_armonicos);
    if (numero_armonicos <= 0)
    {
        while (1)
        {
            printf("El numero de armonicos debe ser mayor a 0. Ingrese nuevamente: ");
            scanf("%d", &numero_armonicos);
            if (numero_armonicos > 0)
            {
                break;
            }
        }
    }
    if (numero_armonicos > 25)
    {
        while (1)
        {
            printf("El numero de armonicos debe ser menor o igual a 25. Ingrese nuevamente: ");
            scanf("%d", &numero_armonicos);
            if (numero_armonicos <= 25)
            {
                break;
            }
        }
    }

    // Ingresar 0-3 para elegir la funcion de fourier a calcular, en este caso solo se tiene una funcion que es la de fourier, asi que se ingresa 0
    int opcion_funcion;
    printf("Funciones de Fourier disponibles:\n");
    printf("0. Funcion de Fourier de -x² - x\n");
    printf("1. Funcion de Fourier de -2x² - x + 1\n");
    printf("2. Funcion de Fourier de -x/2 - 1/4\n");
    printf("3. Funcion de Fourier de x⁴ - x\n");
    printf("Ingrese el numero de la funcion de fourier a calcular: ");
    scanf("%d", &opcion_funcion);
    if (opcion_funcion < 0)
    {
        while (1)
        {
            printf("El numero de la funcion de fourier debe ser mayor o igual a 0. Ingrese nuevamente: ");
            scanf("%d", &opcion_funcion);
            if (opcion_funcion >= 0)
            {
                break;
            }
        }
    }
    if (opcion_funcion > 3)
    {
        while (1)
        {
            printf("El numero de la funcion de fourier debe ser menor o igual a 3. Ingrese nuevamente: ");
            scanf("%d", &opcion_funcion);
            if (opcion_funcion <= 3)
            {
                break;
            }
        }
    }

    crear_csv(numero_armonicos, opcion_funcion);
    FILE *archivo = NULL;

    // Calcular cuantos procesos hijos se necesitan
    int numero_procesos = calcular_procesos_hijos(PI, PASO);

    // Crear matriz en memoria compartida de tamaño fila x columna (filas = numero_procesos, columnas = numero_armonicos)
    key_t llave1, llave2, llave_mutex;
    llave1 = ftok("Archivo", 'k');
    llave2 = ftok("Archivo", 'q');
    llave_mutex = ftok("Archivo", 'm');
    int shmid1 = shmget(llave1, numero_procesos * (numero_armonicos + 2) * sizeof(double), IPC_CREAT | 0777); // Crea la memoria compartida para la matriz de resultados
    int shmid2 = shmget(llave2, sizeof(double), IPC_CREAT | 0777);                                            // Crea la memoria compartida para el valor de a0
    double *memoria_compartida_resultados = (double *)shmat(shmid1, 0, 0);
    double *memoria_compartida_a0 = (double *)shmat(shmid2, 0, 0);
    if (memoria_compartida_resultados == (void *)-1 || memoria_compartida_a0 == (void *)-1)
    {
        perror("Error al crear la memoria compartida");
        exit(-1);
    }

    int semaforo_mutex = crea_semaforo(llave_mutex, 1); // Inicializa en 1 para permitir el acceso a la memoria compartida
    if (semaforo_mutex == -1)
    {
        perror("Error al crear el semaforo");
        exit(-1);
    }

    // Guardar el valor de a0 en la memoria compartida
    switch (opcion_funcion)
    {
    case 0:
        *memoria_compartida_a0 = A0_1;
        break;
    case 1:
        *memoria_compartida_a0 = A0_2;
        break;
    case 2:
        *memoria_compartida_a0 = A0_3;
        break;
    case 3:
        *memoria_compartida_a0 = A0_4;
        break;
    }

    // Crear procesos hijos con memoria compartida y semaforos para calcular los resultados de la funcion de fourier y guardarlos en la memoria compartida
    for (int i = 0; i < numero_procesos; i++)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Error al crear el proceso hijo");
            exit(-1);
        }
        if (pid == 0)
        {
            // Logica de cada proceso hijo
            double x = -PI + i * PASO;
            if (x > PI)
            {
                x = PI; // Asegurarse de que el ultimo valor de x sea exactamente PI
            }
            double resultados_locales[numero_armonicos];

            // calcular sin bloqueo
            for (int j = 1; j <= numero_armonicos; j++)
            {
                resultados_locales[j - 1] = funcion_fourier(x, j, opcion_funcion);
            }

            down(semaforo_mutex); // Entra a la sección crítica para escribir en la memoria compartida

            memoria_compartida_resultados[i * (numero_armonicos + 2)] = x; // Guardar el valor de x en la primera columna de la matriz de resultados

            for (int j = 1; j <= numero_armonicos; j++)
            {
                memoria_compartida_resultados[i * (numero_armonicos + 2) + j] = resultados_locales[j - 1]; // Guardar el resultado de la funcion de fourier en la matriz de resultados
            }

            up(semaforo_mutex); // Sale de la sección crítica
            exit(0);
        }
    }

    // Esperar a que todos los procesos hijos terminen
    for (int i = 0; i < numero_procesos; i++)
    {
        wait(NULL);
    }

    // Proceso padre el cual calcula el valor final de la funcion de fourier
    // para cada valor de x sumando el valor de a0 con los resultados de
    // cada armonico y guardando el resultado final en la ultima columna
    // de la matriz de resultados

    for (int i = 0; i < numero_procesos; i++)
    {
        double f = *memoria_compartida_a0; // Lee el valor de a0 de la memoria compartida
        for (int j = 1; j <= numero_armonicos; j++)
        {
            f += memoria_compartida_resultados[i * (numero_armonicos + 2) + j]; // Suma el resultado de cada armonico al valor de a0 para obtener el valor de la funcion de fourier
        }
        // printf("El valor de la funcion de fourier para x = %lf es: %lf\n", memoria_compartida_resultados[i * (numero_armonicos + 2)], f);
        memoria_compartida_resultados[i * (numero_armonicos + 2) + numero_armonicos + 1] = f; // Guarda el resultado final de la funcion de fourier en la ultima columna de la matriz de resultados
    }

    // Ultima parte para escribir los resultados en el archivo de resultados
    // Checar los resultados en la memoria compartida
    archivo = abrir_archivo_resultados(opcion_funcion);

    // Saltamos la primera fila que es el encabezado y escribimos los resultados de cada proceso hijo en el archivo de resultados
    for (int i = 0; i < numero_procesos; i++)
    {
        fprintf(archivo, "%lf", memoria_compartida_resultados[i * (numero_armonicos + 2)]); // Escribir el valor de x
        for (int j = 1; j <= numero_armonicos; j++)
        {
            fprintf(archivo, ",%lf", memoria_compartida_resultados[i * (numero_armonicos + 2) + j]); // Escribir los resultados de la funcion de fourier
        }
        fprintf(archivo, ",%lf", memoria_compartida_resultados[i * (numero_armonicos + 2) + numero_armonicos + 1]); // Escribir el valor final F
        fprintf(archivo, "\n");
    }
    fclose(archivo);

    // Limpieza al final
    shmdt(memoria_compartida_resultados);
    shmdt(memoria_compartida_a0);
    shmctl(shmid1, IPC_RMID, 0);
    shmctl(shmid2, IPC_RMID, 0);
    semctl(semaforo_mutex, 0, IPC_RMID, 0);

    return 0;
}