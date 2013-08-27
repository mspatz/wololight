CC=gcc
CFLAGS=-O0 -I . -l bcm2835 -std=c99 -g
DEPS = main.h
OBJ = main.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)