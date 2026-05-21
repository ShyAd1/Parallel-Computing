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
    MPI_Init(&argc, &argv);
    // Inicializar MPI y obtener el id del proceso y el numero total de procesos
    int pid, procesos, i;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &procesos);

    int numero_armonicos;
    int opcion_funcion;

    if (pid == 0)
    {
        // Cantidad de armonicos a calcular
        printf("Ingrese el numero de armonicos a calcular: ");
        fflush(stdout);
        scanf("%d", &numero_armonicos);
        if (numero_armonicos <= 0)
        {
            while (1)
            {
                printf("El numero de armonicos debe ser mayor a 0. Ingrese nuevamente: ");
                fflush(stdout);
                scanf("%d", &numero_armonicos);
                if (numero_armonicos > 0)
                {
                    break;
                }
            }
        }

        // Ingresar 0-3 para elegir la funcion de fourier a calcular, en este caso solo se tiene una funcion que es la de fourier, asi que se ingresa 0
        printf("Funciones de Fourier disponibles:\n");
        printf("0. Funcion de Fourier de -x² - x\n");
        printf("1. Funcion de Fourier de -2x² - x + 1\n");
        printf("2. Funcion de Fourier de -x/2 - 1/4\n");
        printf("3. Funcion de Fourier de x⁴ - x\n");
        printf("Ingrese el numero de la funcion de fourier a calcular: ");
        fflush(stdout);
        scanf("%d", &opcion_funcion);
        if (opcion_funcion < 0)
        {
            while (1)
            {
                printf("El numero de la funcion de fourier debe ser mayor o igual a 0. Ingrese nuevamente: ");
                fflush(stdout);
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
                fflush(stdout);
                scanf("%d", &opcion_funcion);
                if (opcion_funcion <= 3)
                {
                    break;
                }
            }
        }
    }

    MPI_Bcast(&numero_armonicos, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&opcion_funcion, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (pid == 0)
    {
        crear_csv(numero_armonicos, opcion_funcion);
    }

    // Calcular cuantos procesos se necesitan
    int numero_procesos = calcular_procesos(PI, PASO);

    // Cada proceso llena una tabla local de filas.
    // Luego se usa MPI_Reduce para juntar todas las tablas en el proceso 0.
    // La suma funciona porque cada fila la llena un solo proceso y las demás quedan en cero.
    int columnas = numero_armonicos + 2; // x + armónicos + F
    int total = numero_procesos * columnas;

    double *local = (double *)malloc((size_t)total * sizeof(double));
    double *global = NULL;
    if (!local)
    {
        perror("malloc local");
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
    for (int k = 0; k < total; k++)
    {
        local[k] = 0.0;
    }

    if (pid == 0)
    {
        global = (double *)malloc((size_t)total * sizeof(double));
        if (!global)
        {
            perror("malloc global");
            free(local);
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        for (int k = 0; k < total; k++)
        {
            global[k] = 0.0;
        }
    }

    // Cada proceso calcula sus filas y guarda los armónicos individuales y la suma.
    for (i = pid; i < numero_procesos; i += procesos)
    {
        double x = -PI + i * PASO;
        if (x > PI)
        {
            x = PI; // Asegurarse de que el ultimo valor de x sea exactamente PI
        }
        double resultado = obtener_a0(opcion_funcion);
        int base = i * columnas;

        local[base] = x;
        for (int n = 1; n <= numero_armonicos; n++)
        {
            double termino = funcion_fourier(x, n, opcion_funcion);
            resultado += termino;
            local[base + n] = termino;
        }
        local[base + columnas - 1] = resultado;
    }

    MPI_Reduce(local, global, total, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (pid == 0)
    {
        FILE *out = abrir_archivo_resultados(opcion_funcion);
        for (int i = 0; i < numero_procesos; i++)
        {
            int base = i * columnas;
            fprintf(out, "%lf", global[base]);
            for (int n = 1; n <= numero_armonicos; n++)
            {
                fprintf(out, ",%lf", global[base + n]);
            }
            fprintf(out, ",%lf\n", global[base + columnas - 1]);
        }
        fclose(out);
        free(global);
    }

    free(local);

    MPI_Finalize();

    return 0;
}