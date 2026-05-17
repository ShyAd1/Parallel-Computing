CC = mpicc
CFLAGS = -Wall -Wextra -pedantic
NPROCS = 7

# Compilar y ejecutar el programa
all: main

# Regla para compilar el programa main.c
main: main.c
	$(CC) $(CFLAGS) main.c -o main

# Regla para ejecutar el programa
run: main
	mpirun -np $(NPROCS) ./main

# Limpiar los archivos generados
clean:
	rm -f main

# Regla por defecto si no se especifica ninguna
.PHONY: all run clean