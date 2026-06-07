CC = gcc
CFLAGS = -Wall -Wextra -pedantic

# Compilar ambos programas
all: Cliente_D Servidor_D

# Regla para compilar el cliente
Cliente_D: Cliente_Datagrama.c
	$(CC) $(CFLAGS) Cliente_Datagrama.c -o Cliente_D

# Regla para compilar el servidor
Servidor_D: Servidor_Datagrama.c
	$(CC) $(CFLAGS) Servidor_Datagrama.c -o Servidor_D

# Regla para ejecutar uno de los dos programas
# Uso:
#   make run ROLE=cliente IP=<IP_DEL_SERVIDOR> FILA=<fila 0-9>
#   make run ROLE=servidor
run:
	@if [ "$(ROLE)" = "cliente" ]; then \
		./Cliente_D $(IP) $(FILA); \
	elif [ "$(ROLE)" = "servidor" ]; then \
		./Servidor_D; \
	else \
		echo "Uso: make run ROLE=cliente|servidor"; \
		exit 1; \
	fi

# Limpiar los archivos generados
clean:
	rm -f Cliente_D Servidor_D

# Regla por defecto si no se especifica ninguna
.PHONY: all run clean