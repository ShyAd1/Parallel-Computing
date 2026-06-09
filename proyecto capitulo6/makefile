CC = gcc
CFLAGS = -Wall -Wextra -pedantic
LDLIBS = -lm

# Compilar ambos programas
all: Cliente_F Servidor_F

# Regla para compilar el cliente
Cliente_F: Cliente_Fourier.c
	$(CC) $(CFLAGS) Cliente_Fourier.c -o Cliente_F $(LDLIBS)

# Regla para compilar el servidor
Servidor_F: Servidor_Fourier.c
	$(CC) $(CFLAGS) Servidor_Fourier.c -o Servidor_F

# Regla para ejecutar uno de los dos programas
# Uso:
#   make run ROLE=cliente IP=<IP_DEL_SERVIDOR> FUNC_OBJ=<FUNCION_OBJETIVO 0-3> SEGMENTO=<SEGMENTO 0-3> NUM_ARMONICOS=<NUMERO_DE_ARMONICOS 1-25>
#   make run ROLE=servidor
run:
	@if [ "$(ROLE)" = "cliente" ]; then \
		./Cliente_F $(IP) $(FUNC_OBJ) $(SEGMENTO) $(NUM_ARMONICOS); \
	elif [ "$(ROLE)" = "servidor" ]; then \
		./Servidor_F; \
	else \
		echo "Uso: make run ROLE=cliente|servidor"; \
		exit 1; \
	fi

# Limpiar los archivos generados
clean:
	rm -f Cliente_F Servidor_F \
	rm -f *.csv

# Regla por defecto si no se especifica ninguna
.PHONY: all run clean