CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all:  test_touche

touche.o: touche.c touche.h
	gcc $(CFLAGS) touche.c -o touche.o


test_touche:   touche.o test_touche.o
	gcc test_touche.o touche.o -o test_touche.out -lSDL

test_touche.o: test_touche.c
	gcc $(CFLAGS) test_touche.c -o test_touche.o



clean:
	rm *.o; rm *~; rm *.out




