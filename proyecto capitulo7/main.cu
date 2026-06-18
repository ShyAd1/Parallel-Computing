#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cuda_runtime.h>

#define PI 3.14159265358979323846
#define PASO 0.1
#define A0_1 -(pow(PI, 2) / 3)
#define A0_2 ((6 - 4 * pow(PI, 2)) / 6)
#define A0_3 -(1.0 / 4.0)
#define A0_4 (pow(PI, 4) / 5)

// ---------------------------------------------------------------------
// Macro para comprobar errores de CUDA. Cualquier llamada a la API de
// CUDA se envuelve con esto para detectar fallos inmediatamente
// (por ejemplo, falta de memoria en la GPU al pedir demasiados armonicos).
// ---------------------------------------------------------------------
#define CUDA_CHECK(llamada)                                                       \
    do                                                                            \
    {                                                                             \
        cudaError_t err = (llamada);                                              \
        if (err != cudaSuccess)                                                   \
        {                                                                         \
            fprintf(stderr, "Error de CUDA en %s:%d -> %s\n", __FILE__, __LINE__, \
                    cudaGetErrorString(err));                                     \
            exit(EXIT_FAILURE);                                                   \
        }                                                                         \
    } while (0)

// ---------------------------------------------------------------------
// Version device de la funcion de fourier (se ejecuta en la GPU).
// ---------------------------------------------------------------------
__device__ double funcion_fourier_device(double x, int n, int opcion_funcion)
{
    switch (opcion_funcion)
    {
    case 0:
        return -(4 * pow(-1.0, n) / pow((double)n, 2)) * cos(n * x) - (2 * pow(-1.0, n) / n) * sin(n * x);
    case 1:
        return -((8 / pow((double)n, 2)) * cos(n * PI) * cos(n * x)) + ((2.0 / n) * cos(n * PI) * sin(n * x));
    case 2:
        return ((pow(-1.0, n + 1)) / n) * sin(n * x);
    case 3:
        return ((8 * pow(-1.0, n) * (pow(n * PI, 2) - 6)) / pow((double)n, 4)) * cos(n * x) + ((2 * pow(-1.0, n)) / n) * sin(n * x);
    default:
        return 0.0;
    }
}

__device__ double obtener_a0_device(int opcion_funcion)
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

// ---------------------------------------------------------------------
// Kernel principal. Cada thread calcula UNA fila completa (un valor de x):
// el valor de x, todos sus armonicos individuales (columnas 1..n) y la
// suma final F.
// ---------------------------------------------------------------------
__global__ void kernel_fourier(double *resultados, int numero_procesos, int numero_armonicos,
                               int columnas, int opcion_funcion)
{
    int fila = blockIdx.x * blockDim.x + threadIdx.x;
    if (fila >= numero_procesos)
    {
        return;
    }

    double x = -PI + fila * PASO;
    if (x > PI)
    {
        x = PI; // Asegurar que el ultimo valor de x sea exactamente PI
    }

    double resultado = obtener_a0_device(opcion_funcion);
    long long base = (long long)fila * columnas;

    resultados[base] = x;
    for (int n = 1; n <= numero_armonicos; n++)
    {
        double termino = funcion_fourier_device(x, n, opcion_funcion);
        resultado += termino;
        resultados[base + n] = termino;
    }
    resultados[base + columnas - 1] = resultado;
}

// ---------------------------------------------------------------------
// Funciones de la CPU (host): manejo de archivos. La GPU nunca toca disco;
// la CPU crea el archivo con su encabezado y, despues de que la GPU
// termine todo el calculo, vuelca los resultados.
// ---------------------------------------------------------------------
const char *nombre_archivo(int opcion_funcion)
{
    switch (opcion_funcion)
    {
    case 0:
        return "resultados_funcion1.csv";
    case 1:
        return "resultados_funcion2.csv";
    case 2:
        return "resultados_funcion3.csv";
    case 3:
        return "resultados_funcion4.csv";
    default:
        return NULL;
    }
}

void crear_csv(int n, int opcion_funcion)
{
    const char *nombre = nombre_archivo(opcion_funcion);
    FILE *archivo = fopen(nombre, "w");
    if (archivo == NULL)
    {
        perror("Error al crear el archivo de resultados");
        exit(EXIT_FAILURE);
    }

    // Encabezado: x,1,2,3,...,n,F
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
    const char *nombre = nombre_archivo(opcion_funcion);
    FILE *archivo = fopen(nombre, "a");
    if (archivo == NULL)
    {
        perror("Error al abrir el archivo de resultados");
        exit(EXIT_FAILURE);
    }
    return archivo;
}

int calcular_procesos(double pi, double paso)
{
    return (int)ceil(2 * pi / paso) + 1; // Se suma 1 para incluir el valor de x = pi
}

// ---------------------------------------------------------------------
// Calcula, segun la memoria libre real de la GPU, el maximo numero de
// armonicos que se pueden procesar sin quedarse sin memoria. Se deja un
// margen de seguridad (no se usa el 100% de la memoria libre) porque el
// driver y el contexto de CUDA tambien reservan memoria.
// ---------------------------------------------------------------------
int max_armonicos_soportados(int numero_procesos)
{
    size_t libre, total;
    CUDA_CHECK(cudaMemGetInfo(&libre, &total));

    // Margen de seguridad: usar solo el 90% de la memoria libre reportada.
    size_t memoria_usable = (size_t)(libre * 0.90);

    // total_doubles = numero_procesos * (armonicos + 2)
    // memoria_usable >= total_doubles * sizeof(double)
    // armonicos <= memoria_usable / (numero_procesos * sizeof(double)) - 2
    double max_doubles_por_fila = (double)memoria_usable / ((double)numero_procesos * sizeof(double));
    int max_armonicos = (int)max_doubles_por_fila - 2;

    if (max_armonicos < 1)
    {
        max_armonicos = 0;
    }
    return max_armonicos;
}

int main(void)
{
    int numero_armonicos;
    int opcion_funcion;

    // -------------------------------------------------------------
    // Todo lo que sigue (lectura de teclado, validacion) corre en la CPU.
    // -------------------------------------------------------------
    printf("Ingrese el numero de armonicos a calcular: ");
    fflush(stdout);
    scanf("%d", &numero_armonicos);
    while (numero_armonicos <= 0)
    {
        printf("El numero de armonicos debe ser mayor a 0. Ingrese nuevamente: ");
        fflush(stdout);
        scanf("%d", &numero_armonicos);
    }

    printf("Funciones de Fourier disponibles:\n");
    printf("0. Funcion de Fourier de -x^2 - x\n");
    printf("1. Funcion de Fourier de -2x^2 - x + 1\n");
    printf("2. Funcion de Fourier de -x/2 - 1/4\n");
    printf("3. Funcion de Fourier de x^4 - x\n");
    printf("Ingrese el numero de la funcion de fourier a calcular: ");
    fflush(stdout);
    scanf("%d", &opcion_funcion);
    while (opcion_funcion < 0 || opcion_funcion > 3)
    {
        printf("La funcion debe estar entre 0 y 3. Ingrese nuevamente: ");
        fflush(stdout);
        scanf("%d", &opcion_funcion);
    }

    int numero_procesos = calcular_procesos(PI, PASO); // numero de filas (valores de x)

    // -------------------------------------------------------------
    // Comprobar cuantos armonicos soporta la memoria de la GPU y
    // recortar si el usuario pidio mas de lo que cabe, avisandole.
    // -------------------------------------------------------------
    int max_armonicos = max_armonicos_soportados(numero_procesos);
    if (max_armonicos <= 0)
    {
        fprintf(stderr, "No hay suficiente memoria en la GPU ni para una sola fila de resultados.\n");
        return EXIT_FAILURE;
    }
    if (numero_armonicos > max_armonicos)
    {
        printf("Aviso: pidio %d armonicos, pero la memoria libre de la GPU solo permite "
               "calcular hasta %d armonicos con %d filas (valores de x). "
               "Se usara %d.\n",
               numero_armonicos, max_armonicos, numero_procesos, max_armonicos);
        numero_armonicos = max_armonicos;
    }

    int columnas = numero_armonicos + 2; // x + armonicos + F
    long long total = (long long)numero_procesos * columnas;
    size_t bytes = (size_t)total * sizeof(double);

    // -------------------------------------------------------------
    // CPU crea el archivo de resultados con su encabezado, ANTES de
    // que la GPU calcule nada.
    // -------------------------------------------------------------
    crear_csv(numero_armonicos, opcion_funcion);

    // -------------------------------------------------------------
    // Reservar memoria en la GPU y lanzar el kernel. Todo el calculo
    // matematico (los 4 tipos de funcion de fourier, para cualquier
    // cantidad de armonicos) ocurre exclusivamente en la GPU.
    // -------------------------------------------------------------
    double *d_resultados = NULL;
    CUDA_CHECK(cudaMalloc((void **)&d_resultados, bytes));
    CUDA_CHECK(cudaMemset(d_resultados, 0, bytes));

    int hilos_por_bloque = 256;
    int bloques = (numero_procesos + hilos_por_bloque - 1) / hilos_por_bloque;

    kernel_fourier<<<bloques, hilos_por_bloque>>>(d_resultados, numero_procesos, numero_armonicos,
                                                  columnas, opcion_funcion);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize()); // Esperar a que la GPU termine todo el calculo

    // -------------------------------------------------------------
    // Traer los resultados de vuelta a la CPU para guardarlos en disco.
    // -------------------------------------------------------------
    double *h_resultados = (double *)malloc(bytes);
    if (h_resultados == NULL)
    {
        perror("malloc h_resultados");
        cudaFree(d_resultados);
        return EXIT_FAILURE;
    }
    CUDA_CHECK(cudaMemcpy(h_resultados, d_resultados, bytes, cudaMemcpyDeviceToHost));
    cudaFree(d_resultados);

    // -------------------------------------------------------------
    // CPU guarda los resultados ya calculados por la GPU.
    // -------------------------------------------------------------
    FILE *out = abrir_archivo_resultados(opcion_funcion);
    for (int fila = 0; fila < numero_procesos; fila++)
    {
        long long base = (long long)fila * columnas;
        fprintf(out, "%lf", h_resultados[base]);
        for (int n = 1; n <= numero_armonicos; n++)
        {
            fprintf(out, ",%lf", h_resultados[base + n]);
        }
        fprintf(out, ",%lf\n", h_resultados[base + columnas - 1]);
    }
    fclose(out);

    free(h_resultados);

    printf("Calculo completado. Resultados guardados en %s\n", nombre_archivo(opcion_funcion));

    return 0;
}
