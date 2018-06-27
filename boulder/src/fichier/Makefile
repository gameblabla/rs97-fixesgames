CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all:  test_fichier

fichier.o: fichier.c fichier.h
	gcc $(CFLAGS) fichier.c -o fichier.o


test_fichier:   fichier.o test_fichier.o
	gcc test_fichier.o fichier.o -o test_fichier.out

test_fichier.o: test_fichier.c
	gcc $(CFLAGS) test_fichier.c -o test_fichier.o



clean:
	rm *.o; rm *~; rm *.out




