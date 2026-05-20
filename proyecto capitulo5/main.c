#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846
#define PASO 0.1
#define A0_1 -(pow(PI, 2) / 3)
#define A0_2 ((6 - 4 * pow(PI, 2)) / 6)
#define A0_3 -(1.0 / 4.0)
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

int calcular_procesos(double pi, double paso)
{
    return (int)ceil(2 * pi / paso) + 1; // Se suma 1 para incluir el valor de x = pi
}

int main(int argc, char *argv[])
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

    // Calcular cuantos procesos se necesitan
    int numero_procesos = calcular_procesos(PI, PASO);

    // Crear procesos para calcular la funcion de fourier en cada valor de x
    int pid, procesos, i;

    // Inicializar MPI y obtener el id del proceso y el numero total de procesos
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &procesos);

    // Divir por 8 procesos ya que MPI usa procesos del procesador y el procesador tiene 8 nucleos, asi se aprovecha al maximo el procesador
    // Por lo que tomara de 8 en 8 valores de x para calcular la funcion de fourier, es decir, el proceso 0 calculara los valores de
    // x = -3.14, -3.04, -2.94, -2.84, -2.74, -2.64, -2.54, -2.44 y asi sucevamente hasta llegar a x = 3.14
}