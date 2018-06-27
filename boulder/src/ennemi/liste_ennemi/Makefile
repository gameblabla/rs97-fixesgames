CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all: test_liste_ennemi




test_liste_ennemi:    test_liste_ennemi.o liste_ennemi.o liste.o ennemi.o
	gcc test_liste_ennemi.o liste_ennemi.o liste.o ennemi.o -o test_liste_ennemi.out


test_liste_ennemi.o:	test_liste_ennemi.c
	gcc -c test_liste_ennemi.c -o test_liste_ennemi.o

liste_ennemi.o: liste_ennemi.c liste_ennemi.h
	gcc $(CFLAGS) liste_ennemi.c -o liste_ennemi.o


liste.o: ../../liste/liste.c ../../liste/liste.h
	gcc -c ../../liste/liste.c -o liste.o

ennemi.o: ../type_ennemi/ennemi.c ../type_ennemi/ennemi.h
	gcc -c ../type_ennemi/ennemi.c -o ennemi.o

clean:
	rm *.o; rm *.out; rm *~

