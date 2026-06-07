/*
 * Servidor_Datagrama.c  —  Servidor UDP con Matriz 10×3
 *
 * Compilar en makefile:  make Servidor_D
 * Ejecutar en makefile:  make run ROLE=servidor
 *
 * Protocolo de mensajes:
 *   REQ:<índice>   — Cliente solicita una fila (0–9)
 *   ROW:<a>,<b>,<c>— Servidor envía los 3 valores de la fila
 *   RES:<valor>    — Cliente devuelve el producto de los 3 valores
 *
 * Múltiples clientes pueden conectarse simultáneamente; el servidor
 * distingue cada mensaje por su prefijo y responde a la IP:puerto
 * correspondiente.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define PUERTO_SERVIDOR 55555
#define LONGITUD_BUFER 1024
#define FILAS 10
#define COLUMNAS 3

/* ──────────────────────────────────────────
   Matriz 10×3
   Fila i, columna j → (i+1)*10 + (j+1)
   ────────────────────────────────────────── */
int matriz[FILAS][COLUMNAS] = {
    {11, 12, 13},   /* fila 0 */
    {21, 22, 23},   /* fila 1 */
    {31, 32, 33},   /* fila 2 */
    {41, 42, 43},   /* fila 3 */
    {51, 52, 53},   /* fila 4 */
    {61, 62, 63},   /* fila 5 */
    {71, 72, 73},   /* fila 6 */
    {81, 82, 83},   /* fila 7 */
    {91, 92, 93},   /* fila 8 */
    {101, 102, 103} /* fila 9 */
};

/* ──────────────────────────────────────────
   Función auxiliar: imprime la matriz
   ────────────────────────────────────────── */
void imprimir_matriz(void)
{
    printf("  +-------+---------+---------+---------+\n");
    printf("  | Fila  |  Col 0  |  Col 1  |  Col 2  |\n");
    printf("  +-------+---------+---------+---------+\n");
    for (int i = 0; i < FILAS; i++)
    {
        printf("  |   %d   |  %5d  |  %5d  |  %5d  |\n",
               i, matriz[i][0], matriz[i][1], matriz[i][2]);
    }
    printf("  +-------+---------+---------+---------+\n\n");
}

/* ══════════════════════════════════════════
   MAIN
   ══════════════════════════════════════════ */
int main(void)
{
    int fd;
    struct sockaddr_in mi_dir, dir_cliente;
    socklen_t len_dir;
    char bufer[LONGITUD_BUFER];
    int n_bytes;

    /* ── Crear socket UDP ── */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    /* ── Configurar dirección del servidor ── */
    mi_dir.sin_family = AF_INET;
    mi_dir.sin_port = htons(PUERTO_SERVIDOR);
    mi_dir.sin_addr.s_addr = INADDR_ANY;
    bzero(&mi_dir.sin_zero, 8);

    if (bind(fd, (struct sockaddr *)&mi_dir, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    /* ── Pantalla de bienvenida ── */
    printf("╔══════════════════════════════════════╗\n");
    printf("║  SERVIDOR DATAGRAMA  —  Matriz 10×3  ║\n");
    printf("╚══════════════════════════════════════╝\n\n");
    imprimir_matriz();
    printf("Escuchando en puerto %d...\n", PUERTO_SERVIDOR);
    printf("(Ctrl+C para detener)\n\n");

    /* ══════════════════════════════════════
       Bucle principal — atiende múltiples
       clientes sin estado entre mensajes.
       El prefijo del mensaje indica la acción.
       ══════════════════════════════════════ */
    while (1)
    {
        len_dir = sizeof(struct sockaddr);

        n_bytes = recvfrom(fd, bufer, LONGITUD_BUFER - 1, 0,
                           (struct sockaddr *)&dir_cliente, &len_dir);
        if (n_bytes <= 0)
        {
            perror("recvfrom");
            continue;
        }
        bufer[n_bytes] = '\0';

        char *ip = inet_ntoa(dir_cliente.sin_addr);

        /* ────────────────────────────────────
           CASO 1: Solicitud de fila  "REQ:<n>"
           ──────────────────────────────────── */
        if (strncmp(bufer, "REQ:", 4) == 0)
        {

            int fila = atoi(bufer + 4);
            printf("[REQ]  %s  →  solicita fila %d\n", ip, fila);

            if (fila < 0 || fila >= FILAS)
            {
                /* Índice fuera de rango */
                const char err[] = "ERR:Indice invalido. Usa 0-9.";
                sendto(fd, err, strlen(err), 0,
                       (struct sockaddr *)&dir_cliente, len_dir);
                printf("[ERR]  Índice fuera de rango: %d\n\n", fila);
            }
            else
            {
                /* Enviar los 3 valores de la fila */
                snprintf(bufer, LONGITUD_BUFER, "ROW:%d,%d,%d",
                         matriz[fila][0], matriz[fila][1], matriz[fila][2]);
                sendto(fd, bufer, strlen(bufer), 0,
                       (struct sockaddr *)&dir_cliente, len_dir);
                printf("[ROW]  Enviado a %s  ←  %d  %d  %d\n",
                       ip,
                       matriz[fila][0], matriz[fila][1], matriz[fila][2]);
            }

            /* ────────────────────────────────────
               CASO 2: Resultado del cliente  "RES:<producto>"
               ──────────────────────────────────── */
        }
        else if (strncmp(bufer, "RES:", 4) == 0)
        {

            long long resultado = atoll(bufer + 4);
            printf("[RES]  %s  →  Producto recibido = %lld\n\n", ip, resultado);

            /* ────────────────────────────────────
               CASO 3: Mensaje desconocido
               ──────────────────────────────────── */
        }
        else
        {
            printf("[???]  Mensaje desconocido de %s: \"%s\"\n\n", ip, bufer);
        }
    }

    close(fd);
    return 0;
}
