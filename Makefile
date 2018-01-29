CC = gcc
CFLAGS = -g -std=gnu11 -Og -Wall -Wno-unused-function

main: main.o implicit.o
	$(CC) $(CFLAGS) -o $@ main.o implicit.o -lm

clean:
	-/bin/rm -rf main main.o implicit.o

main.o: main.c implicit.h	
implicit.o: implicit.c implicit.h	
