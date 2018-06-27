CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all: test_liste_joueur




test_liste_joueur:    test_liste_joueur.o liste_joueur.o liste.o joueur.o touche.o
	gcc test_liste_joueur.o liste_joueur.o liste.o joueur.o -o test_liste_joueur.out -lSDL


touche.o:	../../evenement/touche/touche.c ../../evenement/touche/touche.h
	gcc -c ../../evenement/touche/touche.c -o touche.o

test_liste_joueur.o:	test_liste_joueur.c
	gcc -c test_liste_joueur.c -o test_liste_joueur.o

liste_joueur.o: liste_joueur.c liste_joueur.h
	gcc $(CFLAGS) liste_joueur.c -o liste_joueur.o


liste.o: ../../liste/liste.c ../../liste/liste.h
	gcc -c ../../liste/liste.c -o liste.o

joueur.o: ../type_joueur/joueur.c ../type_joueur/joueur.h
	gcc -c ../type_joueur/joueur.c -o joueur.o

clean:
	rm *.o; rm *.out; rm *~

