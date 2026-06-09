/*
 * Servidor_Fourier.c  —  Servidor UDP Distribuido de Series de Fourier
 *
 * Compilar:  gcc Servidor_Fourier.c -o Servidor_F
 * Ejecutar:  ./Servidor_F
 *
 * ─── Protocolo de mensajes ───────────────────────────────────────────────────
 *
 *  Cliente → Servidor           Servidor → Cliente
 *  ────────────────────         ──────────────────
 *  INIT:<func>,<seg>,<n_arm>  → CREATED  (CSV creado con encabezado)
 *                             → EXISTS   (CSV ya existe, se añadirán filas)
 *
 *  ROW:<func>:<x>,<h1>,...,   → ACK
 *       <hn>,<F>
 *
 *  DONE:<func>,<seg>          → DONE_ACK
 *
 * ─── Funciones disponibles ───────────────────────────────────────────────────
 *  Func 0 → resultados_funcion1.csv   f(x) = -x² - x
 *  Func 1 → resultados_funcion2.csv   f(x) = -2x² - x + 1
 *  Func 2 → resultados_funcion3.csv   f(x) = -x/2 - 1/4
 *  Func 3 → resultados_funcion4.csv   f(x) = x⁴ - x
 *
 * ─── Múltiples clientes ──────────────────────────────────────────────────────
 *  Varios clientes pueden enviar datos para la misma función con segmentos
 *  distintos. El servidor verifica la existencia del CSV antes de crearlo,
 *  evitando sobreescribir el encabezado si ya fue generado por otro cliente.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define PUERTO_SERVIDOR 55556
#define LONGITUD_BUFER  4096

/* ──────────────────────────────────────────────────────────────────────────────
   Utilidades para gestión de CSV
   ────────────────────────────────────────────────────────────────────────────── */

/* Devuelve el nombre del CSV según la opción de función (0-3 → _funcion1-4.csv) */
static void nombre_csv(int func_opt, char *out, int out_sz)
{
    snprintf(out, out_sz, "resultados_funcion%d.csv", func_opt + 1);
}

/* Devuelve 1 si el CSV ya existe en disco, 0 si no */
static int csv_existe(int func_opt)
{
    char nombre[64];
    nombre_csv(func_opt, nombre, sizeof(nombre));
    return access(nombre, F_OK) == 0;
}

/*
 * Crea el CSV con el encabezado:
 *   x,1,2,3,...,n_arm,F
 */
static void crear_csv(int func_opt, int n_arm)
{
    char nombre[64];
    nombre_csv(func_opt, nombre, sizeof(nombre));

    FILE *f = fopen(nombre, "w");
    if (!f) { perror("fopen (crear CSV)"); return; }

    fprintf(f, "x");
    for (int i = 1; i <= n_arm; i++)
        fprintf(f, ",%d", i);
    fprintf(f, ",F\n");

    fclose(f);
    printf("[CSV]  Archivo creado: %s  (n = %d armónicos)\n", nombre, n_arm);
}

/*
 * Añade una fila al CSV en modo append.
 * 'fila' contiene la cadena: "x,h1,h2,...,hn,F"
 */
static void añadir_fila(int func_opt, const char *fila)
{
    char nombre[64];
    nombre_csv(func_opt, nombre, sizeof(nombre));

    FILE *f = fopen(nombre, "a");
    if (!f) { perror("fopen (append CSV)"); return; }

    fprintf(f, "%s\n", fila);
    fclose(f);
}

/* ──────────────────────────────────────────────────────────────────────────────
   MAIN
   ────────────────────────────────────────────────────────────────────────────── */
int main(void)
{
    int fd;
    struct sockaddr_in mi_dir, dir_cli;
    socklen_t          len_dir;
    char               bufer[LONGITUD_BUFER];
    int                n_bytes;

    /* ── Crear socket UDP ── */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket"); exit(1);
    }

    /* ── Configurar dirección ── */
    mi_dir.sin_family      = AF_INET;
    mi_dir.sin_port        = htons(PUERTO_SERVIDOR);
    mi_dir.sin_addr.s_addr = INADDR_ANY;
    bzero(&mi_dir.sin_zero, 8);

    if (bind(fd, (struct sockaddr *)&mi_dir, sizeof(mi_dir)) == -1) {
        perror("bind"); exit(1);
    }

    /* ── Bienvenida ── */
    printf("╔════════════════════════════════════════════════════╗\n");
    printf("║  SERVIDOR FOURIER — Generador de CSV Distribuido   ║\n");
    printf("╚════════════════════════════════════════════════════╝\n\n");
    printf("Puerto    : %d\n", PUERTO_SERVIDOR);
    printf("Protocolo :\n");
    printf("  INIT:<func>,<seg>,<n_arm>  → CREATED | EXISTS\n");
    printf("  ROW:<func>:<x,h1,...,hn,F> → ACK\n");
    printf("  DONE:<func>,<seg>          → DONE_ACK\n\n");
    printf("Archivos generados: resultados_funcion[1-4].csv\n");
    printf("────────────────────────────────────────────────────\n");
    printf("Esperando clientes...\n\n");

    /* ══════════════════════════════════════════════════════════════
       Bucle principal — atiende mensajes de cualquier cliente en
       cualquier orden. El prefijo del mensaje identifica la acción.
       ══════════════════════════════════════════════════════════════ */
    while (1)
    {
        len_dir = sizeof(mi_dir);
        n_bytes = recvfrom(fd, bufer, LONGITUD_BUFER - 1, 0,
                           (struct sockaddr *)&dir_cli, &len_dir);
        if (n_bytes <= 0) { perror("recvfrom"); continue; }
        bufer[n_bytes] = '\0';

        char *ip = inet_ntoa(dir_cli.sin_addr);

        /* ────────────────────────────────────────────────────────────
           INIT:<func>,<seg>,<n_arm>
           El cliente solicita inicio; el servidor crea o verifica CSV.
           ──────────────────────────────────────────────────────────── */
        if (strncmp(bufer, "INIT:", 5) == 0) {
            int func_opt, seg, n_arm;
            if (sscanf(bufer + 5, "%d,%d,%d", &func_opt, &seg, &n_arm) != 3) {
                sendto(fd, "ERR:INIT_FMT", 12, 0,
                       (struct sockaddr *)&dir_cli, len_dir);
                continue;
            }

            printf("[INIT]  %-15s │ Función %d │ Segmento %d │ %d armónicos\n",
                   ip, func_opt, seg, n_arm);

            const char *resp;
            if (csv_existe(func_opt)) {
                char nombre[64];
                nombre_csv(func_opt, nombre, sizeof(nombre));
                printf("[CSV]   Ya existe: %s — se añadirán filas\n", nombre);
                resp = "EXISTS";
            } else {
                crear_csv(func_opt, n_arm);
                resp = "CREATED";
            }
            sendto(fd, resp, strlen(resp), 0,
                   (struct sockaddr *)&dir_cli, len_dir);

        /* ────────────────────────────────────────────────────────────
           ROW:<func>:<x,h1,...,hn,F>
           El cliente envía una fila calculada; el servidor la guarda.
           ──────────────────────────────────────────────────────────── */
        } else if (strncmp(bufer, "ROW:", 4) == 0) {
            char *p   = bufer + 4;
            char *sep = strchr(p, ':');
            if (!sep) {
                sendto(fd, "ERR:ROW_FMT", 11, 0,
                       (struct sockaddr *)&dir_cli, len_dir);
                continue;
            }
            *sep      = '\0';
            int   func_opt = atoi(p);
            char *datos    = sep + 1;    /* "x,h1,...,hn,F" */

            añadir_fila(func_opt, datos);
            sendto(fd, "ACK", 3, 0, (struct sockaddr *)&dir_cli, len_dir);

            /* Mostrar preview del valor de x */
            char x_prev[14] = {0};
            strncpy(x_prev, datos, 13);
            printf("[ROW]   %-15s │ F%d │ x ≈ %s…\n", ip, func_opt, x_prev);

        /* ────────────────────────────────────────────────────────────
           DONE:<func>,<seg>
           El cliente indica que terminó de enviar su segmento.
           ──────────────────────────────────────────────────────────── */
        } else if (strncmp(bufer, "DONE:", 5) == 0) {
            int func_opt, seg;
            sscanf(bufer + 5, "%d,%d", &func_opt, &seg);

            char nombre[64];
            nombre_csv(func_opt, nombre, sizeof(nombre));
            printf("[DONE]  %-15s │ Función %d │ Segmento %d ✓ → %s\n\n",
                   ip, func_opt, seg, nombre);

            sendto(fd, "DONE_ACK", 8, 0, (struct sockaddr *)&dir_cli, len_dir);

        } else {
            printf("[???]   %-15s │ Mensaje desconocido: \"%.40s\"\n\n", ip, bufer);
        }
    }

    close(fd);
    return 0;
}
