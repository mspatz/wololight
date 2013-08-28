CC=gcc
CFLAGS=-O3 -I . -l bcm2835 -lm -std=c99 -g
DEPS = main.h lpd6803.h
OBJ = main.o lpd6803.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)