/*
 * Cliente_Datagrama.c  —  Cliente UDP con cálculo de producto de fila
 *
 * Compilar en makefile:  make Cliente_D
 * Ejecutar en makefile:  make run ROLE=cliente IP=<IP_DEL_SERVIDOR> FILA=<fila 0-9>
 *
 * Ejemplos (varias máquinas en paralelo, mismo binario):
 *   Máquina A:  ./Cliente_D 192.168.1.10 0    → calcula 11×12×13
 *   Máquina B:  ./Cliente_D 192.168.1.10 4    → calcula 51×52×53
 *   Máquina C:  ./Cliente_D 192.168.1.10 9    → calcula 101×102×103
 *
 * Protocolo:
 *   1) Envía  "REQ:<fila>"       al servidor
 *   2) Recibe "ROW:<a>,<b>,<c>" del servidor
 *   3) Calcula a×b×c y lo muestra
 *   4) Envía  "RES:<producto>"   al servidor
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h> /* struct timeval */

#define PUERTO_SERVIDOR 55555
#define LONGITUD_BUFER 1024
#define TIMEOUT_SEG 5 /* Segundos de espera máxima al servidor */

int main(int argc, char *argv[])
{
    int fd;
    struct sockaddr_in dir_servidor;
    struct hostent *he;
    char bufer[LONGITUD_BUFER];
    int n_bytes;
    socklen_t len_dir;

    /* ── Validar argumentos ── */
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <hostname/IP> <fila 0-9>\n", argv[0]);
        fprintf(stderr, "  Ej: %s 192.168.1.10 3\n", argv[0]);
        exit(1);
    }

    int fila = atoi(argv[2]);
    if (fila < 0 || fila > 9)
    {
        fprintf(stderr, "Error: La fila debe ser un valor entre 0 y 9.\n");
        exit(1);
    }

    /* ── Resolver hostname / IP del servidor ── */
    if ((he = gethostbyname(argv[1])) == NULL)
    {
        herror("gethostbyname");
        exit(1);
    }

    /* ── Crear socket UDP ── */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    /* ── Timeout de recepción (evita bloqueo indefinido) ── */
    struct timeval tv = {.tv_sec = TIMEOUT_SEG, .tv_usec = 0};
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1)
    {
        perror("setsockopt");
        /* No es fatal; continuar sin timeout */
    }

    /* ── Configurar dirección del servidor ── */
    dir_servidor.sin_family = AF_INET;
    dir_servidor.sin_port = htons(PUERTO_SERVIDOR);
    dir_servidor.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&dir_servidor.sin_zero, 8);

    printf("╔════════════════════════════════════════════╗\n");
    printf("║  CLIENTE DATAGRAMA  —  Fila solicitada: %d  ║\n", fila);
    printf("╚════════════════════════════════════════════╝\n\n");
    printf("Servidor: %s:%d\n\n", inet_ntoa(dir_servidor.sin_addr), PUERTO_SERVIDOR);

    /* ══════════════════════════════════════
       PASO 1 — Solicitar la fila al servidor
       ══════════════════════════════════════ */
    snprintf(bufer, LONGITUD_BUFER, "REQ:%d", fila);
    n_bytes = sendto(fd, bufer, strlen(bufer), 0,
                     (struct sockaddr *)&dir_servidor, sizeof(struct sockaddr));
    if (n_bytes == -1)
    {
        perror("sendto (REQ)");
        close(fd);
        exit(1);
    }
    printf("[ → ] Solicitud enviada: fila %d\n", fila);

    /* ══════════════════════════════════════
       PASO 2 — Recibir los valores de la fila
       ══════════════════════════════════════ */
    len_dir = sizeof(struct sockaddr);
    n_bytes = recvfrom(fd, bufer, LONGITUD_BUFER - 1, 0,
                       (struct sockaddr *)&dir_servidor, &len_dir);
    if (n_bytes == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            fprintf(stderr, "[ ✗ ] Tiempo de espera agotado. ¿El servidor está activo?\n");
        else
            perror("recvfrom (ROW)");
        close(fd);
        exit(1);
    }
    bufer[n_bytes] = '\0';

    /* Verificar error del servidor */
    if (strncmp(bufer, "ERR:", 4) == 0)
    {
        fprintf(stderr, "[ ✗ ] Error del servidor: %s\n", bufer + 4);
        close(fd);
        exit(1);
    }

    /* Verificar formato ROW: */
    if (strncmp(bufer, "ROW:", 4) != 0)
    {
        fprintf(stderr, "[ ✗ ] Respuesta inesperada: \"%s\"\n", bufer);
        close(fd);
        exit(1);
    }

    /* Parsear los tres valores "ROW:<a>,<b>,<c>" */
    int a, b, c;
    if (sscanf(bufer + 4, "%d,%d,%d", &a, &b, &c) != 3)
    {
        fprintf(stderr, "[ ✗ ] Error al parsear los valores de la fila.\n");
        close(fd);
        exit(1);
    }
    printf("[ ← ] Fila %d recibida: %d  %d  %d\n", fila, a, b, c);

    /* ══════════════════════════════════════
       PASO 3 — Calcular el producto
       ══════════════════════════════════════ */
    long long producto = (long long)a * (long long)b * (long long)c;

    printf("\n");
    printf("  Cálculo: %d × %d × %d\n", a, b, c);
    printf("  ┌────────────────────────────────┐\n");
    printf("  │  Resultado = %-17lld │\n", producto);
    printf("  └────────────────────────────────┘\n\n");

    /* ══════════════════════════════════════
       PASO 4 — Enviar el resultado al servidor
       ══════════════════════════════════════ */
    snprintf(bufer, LONGITUD_BUFER, "RES:%lld", producto);
    n_bytes = sendto(fd, bufer, strlen(bufer), 0,
                     (struct sockaddr *)&dir_servidor, sizeof(struct sockaddr));
    if (n_bytes == -1)
    {
        perror("sendto (RES)");
        close(fd);
        exit(1);
    }
    printf("[ → ] Resultado enviado al servidor: %lld\n", producto);

    close(fd);
    return 0;
}
