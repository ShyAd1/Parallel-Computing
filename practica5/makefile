CC = mpicc
CFLAGS = -Wall -Wextra -O2
TARGET = main
SRC = main.c
NPROCS = 7

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	mpirun -np $(NPROCS) ./$(TARGET)

clean:
	rm -f $(TARGET) matriz.csv resultados.csv
