CC=gcc
CFLAGS=-I. -g -Wall
LDFLAGS=-lX11
OBJ = main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

xkeepbloom: $(OBJ)
	gcc -o $@ $^ $(LDFLAGS)

