CC = nvcc
NVCCFLAGS = -arch=sm_50 -Xcompiler "-Wall -Wextra -pedantic"

# Compilar y ejecutar el programa
all: main

# Regla para compilar el programa main.cu
main: main.cu
	$(CC) $(CFLAGS) main.cu -o main

# Regla para ejecutar el programa
run: main
	./main

# Limpiar los archivos generados
clean:
	rm -f main

# Regla por defecto si no se especifica ninguna
.PHONY: all run clean