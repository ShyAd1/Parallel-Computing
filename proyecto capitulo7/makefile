CC = nvcc
NVCCFLAGS = -arch=sm_50 -Xcompiler "-Wall -Wextra -pedantic"
LDLIBS = -lm

# Compilar y ejecutar el programa
all: main

# Regla para compilar el programa main.cu
main: main.cu
	$(CC) $(NVCCFLAGS) main.cu -o main $(LDLIBS)

# Regla para ejecutar el programa
run: main
	./main

# Limpiar los archivos generados
clean:
	rm -f main
	rm -f *.csv

# Regla por defecto si no se especifica ninguna
.PHONY: all run clean