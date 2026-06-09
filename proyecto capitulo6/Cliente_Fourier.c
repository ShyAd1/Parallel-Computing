/*
 * Cliente_Fourier.c  —  Cliente UDP para Serie de Fourier Distribuida
 *
 * Compilar:  gcc Cliente_Fourier.c -o Cliente_F -lm
 *
 * Uso:  ./Cliente_F <IP_servidor> <func_opt 0-3> <segmento 0-3> <n_armonicos 1-25>
 *
 * ─── Segmentos (64 valores de x de -π a π, paso 0.1) ────────────────────────
 *  Segmento 0 → x[0 ..15]  (-π        a ≈ -1.642)   Máquina 1
 *  Segmento 1 → x[16..31]  (≈ -1.542  a ≈ -0.042)   Máquina 2
 *  Segmento 2 → x[32..47]  (≈  0.058  a ≈  1.558)   Máquina 3
 *  Segmento 3 → x[48..63]  (≈  1.658  a    π    )   Máquina 4
 *
 * ─── Ejemplo: 4 máquinas calculando la función 0 con 10 armónicos ────────────
 *  Máquina 1:  ./Cliente_F 192.168.1.10 0 0 10
 *  Máquina 2:  ./Cliente_F 192.168.1.10 0 1 10
 *  Máquina 3:  ./Cliente_F 192.168.1.10 0 2 10
 *  Máquina 4:  ./Cliente_F 192.168.1.10 0 3 10
 *
 * ─── Funciones de Fourier disponibles ────────────────────────────────────────
 *  0: f(x) = -x² - x
 *  1: f(x) = -2x² - x + 1
 *  2: f(x) = -x/2 - 1/4
 *  3: f(x) = x⁴ - x
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>

#define PUERTO_SERVIDOR  55556
#define LONGITUD_BUFER   4096
#define TIMEOUT_SEG      8        /* Tiempo de espera por respuesta del servidor */
#define REINTENTOS       3        /* Intentos antes de abortar */

#define PI               3.14159265358979323846
#define PASO             0.1
#define TOTAL_VALORES    64       /* ceil(2π / 0.1) + 1 */
#define VALORES_POR_SEG  16      /* 64 / 4 segmentos  */

/* ──────────────────────────────────────────────────────────────────────────────
   n-ésimo armónico de la serie de Fourier
   (Idéntica lógica al main.c original, trasladada al cliente)
   ────────────────────────────────────────────────────────────────────────────── */
static double fourier_n(double x, int n, int func_opt)
{
    switch (func_opt)
    {
    case 0: /* f(x) = -x² - x */
        return -(4.0 * pow(-1.0, n) / ((double)n * n)) * cos(n * x)
               - (2.0 * pow(-1.0, n) / n) * sin(n * x);

    case 1: /* f(x) = -2x² - x + 1 */
        return -((8.0 / ((double)n * n)) * cos(n * PI) * cos(n * x))
               + ((2.0 / n) * cos(n * PI) * sin(n * x));

    case 2: /* f(x) = -x/2 - 1/4 */
        return (pow(-1.0, n + 1) / n) * sin(n * x);

    case 3: /* f(x) = x⁴ - x */
        return ((8.0 * pow(-1.0, n) * ((double)(n * n) * PI * PI - 6.0))
                / pow((double)n, 4.0)) * cos(n * x)
               + ((2.0 * pow(-1.0, n)) / n) * sin(n * x);

    default: return 0.0;
    }
}

/* Término independiente a₀/2 de cada función */
static double get_a0(int func_opt)
{
    switch (func_opt)
    {
    case 0: return -(PI * PI / 3.0);
    case 1: return (6.0 - 4.0 * PI * PI) / 6.0;
    case 2: return -1.0 / 4.0;           /* Corregido: la original hace div. entera */
    case 3: return (PI * PI * PI * PI) / 5.0;
    default: return 0.0;
    }
}

/* Nombre legible de cada función */
static const char *desc_funcion(int func_opt)
{
    switch (func_opt)
    {
    case 0: return "f(x) = -x^2 - x";
    case 1: return "f(x) = -2x^2 - x + 1";
    case 2: return "f(x) = -x/2 - 1/4";
    case 3: return "f(x) = x^4 - x";
    default: return "desconocida";
    }
}

/* ──────────────────────────────────────────────────────────────────────────────
   enviar_esperar():
   Envía msg al servidor, espera respuesta con timeout + reintentos.
   Devuelve la longitud de la respuesta, o -1 en error.
   ────────────────────────────────────────────────────────────────────────────── */
static int enviar_esperar(int fd,
                          const char *msg,
                          char       *resp,
                          int         max_resp,
                          struct sockaddr_in *srv)
{
    socklen_t srv_len = sizeof(struct sockaddr);

    for (int intento = 1; intento <= REINTENTOS; intento++)
    {
        /* Enviar datagrama */
        if (sendto(fd, msg, strlen(msg), 0,
                   (struct sockaddr *)srv, srv_len) == -1) {
            perror("sendto"); return -1;
        }

        /* Esperar respuesta */
        int n = recvfrom(fd, resp, max_resp - 1, 0,
                         (struct sockaddr *)srv, &srv_len);
        if (n > 0) {
            resp[n] = '\0';
            return n;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK)
            printf("  [!] Sin respuesta (intento %d/%d), reintentando...\n",
                   intento, REINTENTOS);
        else {
            perror("recvfrom"); return -1;
        }
    }

    fprintf(stderr, "  [✗] El servidor no respondió después de %d intentos.\n",
            REINTENTOS);
    return -1;
}

/* ══════════════════════════════════════════════════════════════════════════════
   MAIN
   ══════════════════════════════════════════════════════════════════════════════ */
int main(int argc, char *argv[])
{
    /* ── Validar argumentos ── */
    if (argc != 5) {
        fprintf(stderr,
                "Uso: %s <IP_servidor> <func_opt 0-3> <segmento 0-3> <n_armonicos 1-25>\n"
                "  Ej (4 maquinas, misma funcion):\n"
                "    %s 192.168.1.10 0 0 10\n"
                "    %s 192.168.1.10 0 1 10\n"
                "    %s 192.168.1.10 0 2 10\n"
                "    %s 192.168.1.10 0 3 10\n",
                argv[0], argv[0], argv[0], argv[0], argv[0]);
        exit(1);
    }

    int func_opt    = atoi(argv[2]);
    int segmento    = atoi(argv[3]);
    int n_armonicos = atoi(argv[4]);

    if (func_opt    < 0 || func_opt    > 3)  { fprintf(stderr, "func_opt debe ser 0-3\n");       exit(1); }
    if (segmento    < 0 || segmento    > 3)  { fprintf(stderr, "segmento debe ser 0-3\n");        exit(1); }
    if (n_armonicos < 1 || n_armonicos > 25) { fprintf(stderr, "n_armonicos debe ser 1-25\n");   exit(1); }

    /* ── Resolver hostname del servidor ── */
    struct hostent *he;
    if ((he = gethostbyname(argv[1])) == NULL) { herror("gethostbyname"); exit(1); }

    /* ── Crear socket UDP ── */
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) { perror("socket"); exit(1); }

    /* ── Configurar timeout de recepción ── */
    struct timeval tv = { .tv_sec = TIMEOUT_SEG, .tv_usec = 0 };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    /* ── Dirección del servidor ── */
    struct sockaddr_in srv;
    srv.sin_family = AF_INET;
    srv.sin_port   = htons(PUERTO_SERVIDOR);
    srv.sin_addr   = *((struct in_addr *)he->h_addr);
    bzero(&srv.sin_zero, 8);

    /* ── Calcular rango de índices para este segmento ── */
    int inicio = segmento * VALORES_POR_SEG;
    int fin    = (segmento == 3) ? TOTAL_VALORES : inicio + VALORES_POR_SEG;

    /* ── Pantalla de inicio ── */
    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║  CLIENTE FOURIER — Segmento %d                      ║\n", segmento);
    printf("╚════════════════════════════════════════════════════╝\n\n");
    printf("Servidor    : %s:%d\n", inet_ntoa(srv.sin_addr), PUERTO_SERVIDOR);
    printf("Función     : %d — %s\n", func_opt, desc_funcion(func_opt));
    printf("Segmento    : %d  (x[%d]..x[%d], %d valores)\n",
           segmento, inicio, fin - 1, fin - inicio);
    printf("Armónicos   : %d\n", n_armonicos);
    printf("────────────────────────────────────────────────────\n\n");

    char bufer[LONGITUD_BUFER];
    char respuesta[256];

    /* ════════════════════════════════════════════════════
       PASO 1 — INIT: pedir al servidor que cree el CSV
       ════════════════════════════════════════════════════ */
    snprintf(bufer, sizeof(bufer), "INIT:%d,%d,%d", func_opt, segmento, n_armonicos);
    printf("[ → ] INIT enviado al servidor...\n");

    if (enviar_esperar(fd, bufer, respuesta, sizeof(respuesta), &srv) <= 0) {
        fprintf(stderr, "[✗] Sin respuesta al INIT.\n"); close(fd); exit(1);
    }

    if (strcmp(respuesta, "CREATED") == 0)
        printf("[ ← ] CSV creado por el servidor.\n\n");
    else if (strcmp(respuesta, "EXISTS") == 0)
        printf("[ ← ] CSV ya existente — el servidor añadirá las filas.\n\n");
    else {
        fprintf(stderr, "[✗] Respuesta inesperada del servidor: '%s'\n", respuesta);
        close(fd); exit(1);
    }

    /* ════════════════════════════════════════════════════
       PASO 2 — Calcular armónicos y enviar fila por fila
       ════════════════════════════════════════════════════ */
    double a0 = get_a0(func_opt);   /* Término constante de la serie */
    double armonicos[26];           /* armonicos[1..n_armonicos]      */

    printf("[ × ] Calculando y enviando %d valores (segmento %d)...\n\n",
           fin - inicio, segmento);

    for (int i = inicio; i < fin; i++)
    {
        /* ── Valor de x para este índice ── */
        double x = -PI + i * PASO;
        if (x > PI) x = PI;       /* El último punto del último segmento es exactamente π */

        /* ── Calcular armónicos y suma total F ── */
        double f = a0;
        for (int j = 1; j <= n_armonicos; j++) {
            armonicos[j] = fourier_n(x, j, func_opt);
            f += armonicos[j];
        }

        /* ── Construir mensaje ROW:<func>:<x>,<h1>,...,<hn>,<F> ── */
        int pos = snprintf(bufer, sizeof(bufer), "ROW:%d:%.10f", func_opt, x);
        for (int j = 1; j <= n_armonicos; j++)
            pos += snprintf(bufer + pos, sizeof(bufer) - pos,
                            ",%.10f", armonicos[j]);
        snprintf(bufer + pos, sizeof(bufer) - pos, ",%.10f", f);

        /* ── Enviar fila y esperar ACK ── */
        if (enviar_esperar(fd, bufer, respuesta, sizeof(respuesta), &srv) <= 0) {
            fprintf(stderr, "[✗] Error enviando fila %d (x = %.5f)\n", i, x);
            close(fd); exit(1);
        }
        if (strcmp(respuesta, "ACK") != 0) {
            fprintf(stderr, "[✗] Servidor respondió '%s' (esperaba ACK)\n", respuesta);
            close(fd); exit(1);
        }

        printf("  [%2d/%d]  x = %9.5f   F = %12.6f  ✓\n",
               i - inicio + 1, fin - inicio, x, f);
    }

    /* ════════════════════════════════════════════════════
       PASO 3 — DONE: notificar al servidor que terminamos
       ════════════════════════════════════════════════════ */
    snprintf(bufer, sizeof(bufer), "DONE:%d,%d", func_opt, segmento);
    printf("\n[ → ] DONE enviado...\n");

    if (enviar_esperar(fd, bufer, respuesta, sizeof(respuesta), &srv) > 0) {
        if (strcmp(respuesta, "DONE_ACK") == 0)
            printf("[ ← ] Segmento %d confirmado por el servidor ✓\n", segmento);
    }

    printf("\n════════════════════════════════════════════════════\n");
    printf("  Segmento %d completado. Revisar el servidor para\n", segmento);
    printf("  ver el archivo: resultados_funcion%d.csv\n", func_opt + 1);
    printf("════════════════════════════════════════════════════\n");

    close(fd);
    return 0;
}
