CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all: test_liste_pierre




test_liste_pierre:    test_liste_pierre.o liste_pierre.o liste.o pierre.o
	gcc test_liste_pierre.o liste_pierre.o liste.o pierre.o -o test_liste_pierre.out


test_liste_pierre.o:	test_liste_pierre.c
	gcc -c test_liste_pierre.c -o test_liste_pierre.o

liste_pierre.o: liste_pierre.c liste_pierre.h
	gcc $(CFLAGS) liste_pierre.c -o liste_pierre.o


liste.o: ../../liste/liste.c ../../liste/liste.h
	gcc -c ../../liste/liste.c -o liste.o

pierre.o: ../type_pierre/pierre.c ../type_pierre/pierre.h
	gcc -c ../type_pierre/pierre.c -o pierre.o

clean:
	rm *.o; rm *.out; rm *~

