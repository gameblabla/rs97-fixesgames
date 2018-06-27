CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all:  test_ennemi

ennemi.o: ennemi.c ennemi.h
	gcc $(CFLAGS) ennemi.c -o ennemi.o


test_ennemi:   ennemi.o test_ennemi.o
	gcc test_ennemi.o ennemi.o -o test_ennemi.out

test_ennemi.o: test_ennemi.c
	gcc $(CFLAGS) test_ennemi.c -o test_ennemi.o



clean:
	rm *.o; rm *.out; rm *~




