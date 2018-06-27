CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all:  test_joueur

joueur.o: joueur.c joueur.h touche.o
	gcc $(CFLAGS) joueur.c -o joueur.o


test_joueur:   joueur.o test_joueur.o
	gcc test_joueur.o joueur.o touche.o -o test_joueur.out -lSDL

test_joueur.o: test_joueur.c
	gcc $(CFLAGS) test_joueur.c -o test_joueur.o

touche.o: ../../evenement/touche/touche.c ../../evenement/touche/touche.h
	gcc -c ../../evenement/touche/touche.c -o touche.o


clean:
	rm *.o; rm *.out; rm *~




