CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all: test_liste_explosion




test_liste_explosion:    test_liste_explosion.o liste_explosion.o liste.o explosion.o
	gcc test_liste_explosion.o liste_explosion.o liste.o explosion.o -o test_liste_explosion.out


test_liste_explosion.o:	test_liste_explosion.c
	gcc -c test_liste_explosion.c -o test_liste_explosion.o

liste_explosion.o: liste_explosion.c liste_explosion.h
	gcc $(CFLAGS) liste_explosion.c -o liste_explosion.o


liste.o: ../../liste/liste.c ../../liste/liste.h
	gcc -c ../../liste/liste.c -o liste.o

explosion.o: ../type_explosion/explosion.c ../type_explosion/explosion.h
	gcc -c ../type_explosion/explosion.c -o explosion.o

clean:
	rm *.o; rm *.out; rm *~

