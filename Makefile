CC=gcc
CFLAGS=-O3 -D_GNU_SOURCE -I . -l bcm2835 -lm -ljack -std=c99
DEPS = main.h lpd6803.h audio.h
OBJ = main.o lpd6803.o audio.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)