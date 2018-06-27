CC=gcc
LD=gcc
CFLAGS=-c -Wall -g
LIBS= -lSDL
LINKOBJ= niveau.o joueur.o liste_joueur.o temps.o touche.o liste.o diamant.o liste_diamant.o pierre.o liste_pierre.o explosion.o liste_explosion.o ennemi.o liste_ennemi.o slim.o mur_qui_bouge.o environnement.o environnement_joueur.o environnement_ennemi.o environnement_diamant.o environnement_pierre.o environnement_slim.o environnement_explosion.o

.PHONY: all

all: test

test: test_liste test_pierre test_liste_pierre test_explosion teste_liste_explosion test_diamant test_liste_diamant test_joueur test_liste_joueur test_ennemi test_liste_ennemi test_niveau  test_environnement 

test_liste: test_liste.o $(LINKOBJ)
	gcc test_liste.o $(LINKOBJ) -o test_liste.out $(LIBS)


test_pierre: test_pierre.o $(LINKOBJ)
	gcc test_pierre.o $(LINKOBJ) -o test_pierre.out $(LIBS)

test_liste_pierre: test_liste_pierre.o $(LINKOBJ)
	gcc test_liste_pierre.o $(LINKOBJ) -o test_liste_pierre.out $(LIBS)

test_explosion:	test_explosion.o $(LINKOBJ)
	gcc test_explosion.o $(LINKOBJ) -o test_explosion.out $(LIBS)

teste_liste_explosion: test_liste_explosion.o $(LINKOBJ)
	gcc test_liste_explosion.o $(LINKOBJ) -o test_liste_explosion.out $(LIBS)

test_diamant: test_diamant.o $(LINKOBJ)
	gcc test_diamant.o $(LINKOBJ) -o test_diamant.out $(LIBS)

test_liste_diamant: test_liste_diamant.o $(LINKOBJ)
	gcc test_liste_diamant.o $(LINKOBJ) -o test_liste_diamant.out $(LIBS)

test_joueur: test_joueur.o $(LINKOBJ)
	gcc test_joueur.o $(LINKOBJ) -o test_joueur.out $(LIBS)

test_liste_joueur: test_liste_joueur.o $(LINKOBJ)
	gcc test_liste_joueur.o $(LINKOBJ) -o test_liste_joueur.out $(LIBS)

test_ennemi: test_ennemi.o $(LINKOBJ)
	gcc test_ennemi.o $(LINKOBJ) -o test_ennemi.out $(LIBS)

test_liste_ennemi: test_liste_ennemi.o $(LINKOBJ)
	gcc test_liste_ennemi.o $(LINKOBJ) -o test_liste_ennemi.out $(LIBS)

test_niveau: test_niveau.o $(LINKOBJ)
	gcc test_niveau.o $(LINKOBJ) -o test_niveau.out $(LIBS)

test_environnement: test_environnement.o $(LINKOBJ)
	gcc test_environnement.o $(LINKOBJ) -o test_environnement.out $(LIBS)

test_liste.o: ../liste/test_liste.c liste.o 
	gcc $(CFLAGS) ../liste/test_liste.c -o test_liste.o
	

test_pierre.o: ../pierre/type_pierre/test_pierre.c pierre.o
	gcc $(CFLAGS) ../pierre/type_pierre/test_pierre.c -o test_pierre.o

test_liste_pierre.o: ../pierre/liste_pierre/test_liste_pierre.c pierre.o liste.o
	gcc $(CFLAGS) ../pierre/liste_pierre/test_liste_pierre.c -o test_liste_pierre.o

test_explosion.o: ../explosion/type_explosion/test_explosion.c explosion.o
	gcc $(CFLAGS) ../explosion/type_explosion/test_explosion.c -o test_explosion.o

test_liste_explosion.o: ../explosion/liste_explosion/test_liste_explosion.c explosion.o liste.o
	gcc $(CFLAGS) ../explosion/liste_explosion/test_liste_explosion.c -o test_liste_explosion.o

test_diamant.o: ../diamant/type_diamant/test_diamant.c diamant.o
	gcc $(CFLAGS) ../diamant/type_diamant/test_diamant.c -o test_diamant.o

test_liste_diamant.o: ../diamant/liste_diamant/test_liste_diamant.c diamant.o liste.o
	gcc $(CFLAGS) ../diamant/liste_diamant/test_liste_diamant.c -o test_liste_diamant.o
test_joueur.o: ../joueur/type_joueur/test_joueur.c joueur.o touche.o
	gcc $(CFLAGS) ../joueur/type_joueur/test_joueur.c -o test_joueur.o

test_liste_joueur.o: ../joueur/liste_joueur/test_liste_joueur.c joueur.o liste.o touche.o
	gcc $(CFLAGS) ../joueur/liste_joueur/test_liste_joueur.c -o test_liste_joueur.o

test_ennemi.o: ../ennemi/type_ennemi/test_ennemi.c ennemi.o
	gcc $(CFLAGS) ../ennemi/type_ennemi/test_ennemi.c -o test_ennemi.o

test_liste_ennemi.o: ../ennemi/liste_ennemi/test_liste_ennemi.c ennemi.o liste.o
	gcc $(CFLAGS) ../ennemi/liste_ennemi/test_liste_ennemi.c -o test_liste_ennemi.o

test_niveau.o: ../niveau/test_niveau.c $(LINKOBJ)
	gcc $(CFLAGS) ../niveau/test_niveau.c -o test_niveau.o

test_environnement.o: ../environnement/test_environnement.c $(LINKOBJ)
	gcc $(CFLAGS) ../environnement/test_environnement.c -o test_environnement.o





 



environnement.o:	../environnement/environnement.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement.c -o environnement.o

environnement_joueur.o:	../environnement/environnement_joueur.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement_joueur.c -o environnement_joueur.o

environnement_diamant.o:	../environnement/environnement_diamant.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement_diamant.c -o environnement_diamant.o

environnement_pierre.o:	../environnement/environnement_pierre.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement_pierre.c -o environnement_pierre.o


environnement_ennemi.o:	../environnement/environnement_ennemi.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement_ennemi.c -o environnement_ennemi.o

environnement_slim.o:	../environnement/environnement_slim.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement_slim.c -o environnement_slim.o

environnement_explosion.o:	../environnement/environnement_explosion.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement_explosion.c -o environnement_explosion.o

niveau.o:	../niveau/niveau.c ../niveau/niveau.h  slim.o liste_pierre.o liste_ennemi.o liste_joueur.o  liste_diamant.o
	gcc $(CFLAGS) ../niveau/niveau.c -o niveau.o

liste_explosion.o: ../explosion/liste_explosion/liste_explosion.c ../explosion/liste_explosion/liste_explosion.h
	gcc $(CFLAGS) ../explosion/liste_explosion/liste_explosion.c -o liste_explosion.o


liste.o: ../liste/liste.c ../liste/liste.h
	gcc $(CFLAGS) ../liste/liste.c -o liste.o

explosion.o: ../explosion/type_explosion/explosion.c ../explosion/type_explosion/explosion.h
	gcc $(CFLAGS) ../explosion/type_explosion/explosion.c -o explosion.o

liste_joueur.o:	../joueur/liste_joueur/liste_joueur.c ../joueur/liste_joueur/liste_joueur.h	joueur.o liste.o
	gcc $(CFLAGS) ../joueur/liste_joueur/liste_joueur.c -o liste_joueur.o

joueur.o:	../joueur/type_joueur/joueur.c ../joueur/type_joueur/joueur.h	temps.o
	gcc $(CFLAGS) ../joueur/type_joueur/joueur.c -o joueur.o

temps.o:	../evenement/temps/temps.h ../evenement/temps/temps.c
	gcc $(CFLAGS) ../evenement/temps/temps.c -o temps.o

touche.o:	../evenement/touche/touche.h ../evenement/touche/touche.c
	gcc $(CFLAGS) ../evenement/touche/touche.c -o touche.o


slim.o: ../slim/slim.c ../slim/slim.h
	gcc $(CFLAGS) ../slim/slim.c -o slim.o

diamant.o:	../diamant/type_diamant/diamant.c ../diamant/type_diamant/diamant.h
	gcc $(CFLAGS) ../diamant/type_diamant/diamant.c -o diamant.o

liste_diamant.o:	../diamant/liste_diamant/liste_diamant.c ../diamant/liste_diamant/liste_diamant.h diamant.o liste.o
	gcc $(CFLAGS) ../diamant/liste_diamant/liste_diamant.c -o liste_diamant.o

pierre.o:	../pierre/type_pierre/pierre.c ../pierre/type_pierre/pierre.h
	gcc $(CFLAGS) ../pierre/type_pierre/pierre.c -o pierre.o

liste_pierre.o:	../pierre/liste_pierre/liste_pierre.c ../pierre/liste_pierre/liste_pierre.h pierre.o liste.o
	gcc $(CFLAGS) ../pierre/liste_pierre/liste_pierre.c -o liste_pierre.o

ennemi.o:	../ennemi/type_ennemi/ennemi.c ../ennemi/type_ennemi/ennemi.h
	gcc $(CFLAGS) ../ennemi/type_ennemi/ennemi.c -o ennemi.o

liste_ennemi.o:	../ennemi/liste_ennemi/liste_ennemi.c ../ennemi/liste_ennemi/liste_ennemi.h ennemi.o liste.o
	gcc $(CFLAGS) ../ennemi/liste_ennemi/liste_ennemi.c -o liste_ennemi.o


mur_qui_bouge.o: ../mur_qui_bouge/mur_qui_bouge.c ../mur_qui_bouge/mur_qui_bouge.h
	gcc $(CFLAGS) ../mur_qui_bouge/mur_qui_bouge.c -o mur_qui_bouge.o



clean:
	rm *.o; rm *.out; rm *~

