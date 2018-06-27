CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all: test_liste_diamant




test_liste_diamant:    test_liste_diamant.o liste_diamant.o liste.o diamant.o
	gcc test_liste_diamant.o liste_diamant.o liste.o diamant.o -o test_liste_diamant.out


test_liste_diamant.o:	test_liste_diamant.c
	gcc -c test_liste_diamant.c -o test_liste_diamant.o

liste_diamant.o: liste_diamant.c liste_diamant.h
	gcc $(CFLAGS) liste_diamant.c -o liste_diamant.o


liste.o: ../../liste/liste.c ../../liste/liste.h
	gcc -c ../../liste/liste.c -o liste.o

diamant.o: ../type_diamant/diamant.c ../type_diamant/diamant.h
	gcc -c ../type_diamant/diamant.c -o diamant.o

clean:
	rm *.o; rm *.out; rm *~
