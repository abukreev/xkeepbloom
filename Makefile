CC=gcc
CFLAGS=-I. -g -Wall
LDFLAGS=-lX11
SOURCES=main.c args.c
OBJ=main.o args.o

%.o: %.c $(SOURCES)
	$(CC) -c -o $@ $< $(CFLAGS)

xkeepbloom: $(OBJ)
	gcc -o $@ $^ $(LDFLAGS)

