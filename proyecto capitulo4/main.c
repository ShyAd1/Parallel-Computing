#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#define PI 3.14159265358979323846
#define PERMISOS 0666
#define PASO 0.1
#define A0_1 -(pow(PI, 2) / 3)
#define A0_2 ((6 - 4 * pow(PI, 2)) / 6)
#define A0_3 -(1.0 / 4.0)
#define A0_4 (pow(PI, 4) / 5)

pthread_mutex_t mutex_archivo = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condicion_orden = PTHREAD_COND_INITIALIZER;
int siguiente_indice_escritura = 0;

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

double obtener_a0(int opcion_funcion)
{
    switch (opcion_funcion)
    {
    case 0:
        return A0_1;
    case 1:
        return A0_2;
    case 2:
        return A0_3;
    case 3:
        return A0_4;
    default:
        return 0.0;
    }
}

typedef struct
{
    int indice;
    int opcion_funcion;
    int numero_armonicos;
    double x;
    double *resultados_locales;
} DatosHilo;

void *tarea_abrir_archivo_resultados(void *arg)
{
    DatosHilo *datos = (DatosHilo *)arg;
    double suma_armonicos = 0.0;
    double f_total = 0.0;

    for (int j = 0; j < datos->numero_armonicos; j++)
    {
        suma_armonicos += datos->resultados_locales[j];
    }
    f_total = obtener_a0(datos->opcion_funcion) + suma_armonicos;

    pthread_mutex_lock(&mutex_archivo);
    while (datos->indice != siguiente_indice_escritura)
    {
        pthread_cond_wait(&condicion_orden, &mutex_archivo);
    }

    // Abrir el archivo de resultados para escribir los resultados locales calculados por el hilo
    FILE *archivo = abrir_archivo_resultados(datos->opcion_funcion);

    fprintf(archivo, "%lf", datos->x);

    // Escribir los resultados locales calculados por el hilo en el archivo de resultados
    for (int j = 0; j < datos->numero_armonicos; j++)
    {
        fprintf(archivo, ",%lf", datos->resultados_locales[j]);
    }
    fprintf(archivo, ",%lf\n", f_total);

    fclose(archivo);

    siguiente_indice_escritura++;
    pthread_cond_broadcast(&condicion_orden);
    pthread_mutex_unlock(&mutex_archivo);

    free(datos->resultados_locales);
    free(datos);
    return NULL;
}

int calcular_hilos(double pi, double paso)
{
    return (int)ceil(2 * pi / paso) + 1; // Se suma 1 para incluir el valor de x = pi
}

int main()
{
    // Cantidad de armonicos a calcular
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
    siguiente_indice_escritura = 0;

    // Calcular cuantos hilos se necesitan
    int numero_hilos = calcular_hilos(PI, PASO);

    // Crear hilos para calcular los resultados de la funcion de fourier y guardarlos en el archivo de resultados
    pthread_t hilos[numero_hilos];

    // Cada hilo se encargara de calcular los valores del armonico n para un valor de x y guardarlos en el archivo de resultados
    for (int i = 0; i < numero_hilos; i++)
    {
        double x = -PI + i * PASO;
        if (x > PI)
        {
            x = PI; // Asegurarse de que el ultimo valor de x sea exactamente PI
        }
        double *resultados_locales = malloc(numero_armonicos * sizeof(double));
        if (resultados_locales == NULL)
        {
            perror("Error al reservar memoria para resultados locales");
            exit(-1);
        }
        for (int j = 1; j <= numero_armonicos; j++)
        {
            resultados_locales[j - 1] = funcion_fourier(x, j, opcion_funcion);
        }

        DatosHilo *datos_hilo = malloc(sizeof(DatosHilo));
        if (datos_hilo == NULL)
        {
            perror("Error al reservar memoria para datos del hilo");
            free(resultados_locales);
            exit(-1);
        }
        datos_hilo->indice = i;
        datos_hilo->opcion_funcion = opcion_funcion;
        datos_hilo->numero_armonicos = numero_armonicos;
        datos_hilo->x = x;
        datos_hilo->resultados_locales = resultados_locales;

        if (pthread_create(&hilos[i], NULL, tarea_abrir_archivo_resultados, datos_hilo) != 0)
        {
            perror("Error al crear hilo");
            free(datos_hilo->resultados_locales);
            free(datos_hilo);
            exit(-1);
        }
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < numero_hilos; i++)
    {
        if (pthread_join(hilos[i], NULL) != 0)
        {
            perror("Error al esperar a que termine el hilo");
            exit(-1);
        }
    }

    return 0;
}