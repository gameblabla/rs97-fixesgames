CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all:  test_explosion

explosion.o: explosion.c explosion.h
	gcc $(CFLAGS) explosion.c -o explosion.o


test_explosion:   explosion.o test_explosion.o
	gcc test_explosion.o explosion.o -o test_explosion.out

test_explosion.o: test_explosion.c
	gcc $(CFLAGS) test_explosion.c -o test_explosion.o



clean:
	rm *.o; rm *.out; rm *~




