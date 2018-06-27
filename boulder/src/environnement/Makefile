CC=gcc
LD=gcc
CFLAGS=-c -Wall -g
LIBS= -lSDL
LINKOBJ= niveau.o joueur.o liste_joueur.o temps.o touche.o liste.o diamant.o liste_diamant.o pierre.o liste_pierre.o explosion.o liste_explosion.o ennemi.o liste_ennemi.o slim.o mur_qui_bouge.o environnement.o environnement_joueur.o environnement_ennemi.o environnement_diamant.o environnement_explosion.o environnement_pierre.o environnement_slim.o test_environnement.o fichier.o

.PHONY: all

all: test_environnement

test_environnement: $(LINKOBJ)
	gcc $(LINKOBJ) -o test_environnement.out $(LIBS)  


test_environnement.o:	test_environnement.c
	gcc $(CFLAGS) test_environnement.c -o test_environnement.o

environnement.o:	environnement.c environnement.h
	gcc $(CFLAGS) environnement.c -o environnement.o

environnement_joueur.o:	environnement_joueur.c environnement.h
	gcc $(CFLAGS) environnement_joueur.c -o environnement_joueur.o

environnement_diamant.o:	environnement_diamant.c environnement.h
	gcc $(CFLAGS) environnement_diamant.c -o environnement_diamant.o

environnement_pierre.o:	environnement_pierre.c environnement.h
	gcc $(CFLAGS) environnement_pierre.c -o environnement_pierre.o

environnement_explosion.o: environnement_explosion.c environnement.h
	gcc $(CFLAGS) environnement_explosion.c -o environnement_explosion.o

environnement_ennemi.o:	environnement_ennemi.c environnement.h
	gcc $(CFLAGS) environnement_ennemi.c -o environnement_ennemi.o

environnement_slim.o:	environnement_slim.c environnement.h
	gcc $(CFLAGS) environnement_slim.c -o environnement_slim.o


niveau.o:	../niveau/niveau.c ../niveau/niveau.h  slim.o liste_pierre.o liste_ennemi.o liste_joueur.o  liste_diamant.o
	gcc $(CFLAGS) ../niveau/niveau.c -o niveau.o

liste_explosion.o: ../explosion/liste_explosion/liste_explosion.c ../explosion/liste_explosion/liste_explosion.h
	gcc $(CFLAGS) ../explosion/liste_explosion/liste_explosion.c -o liste_explosion.o

fichier.o:	../fichier/fichier.c ../fichier/fichier.h
	gcc $(CFLAGS) ../fichier/fichier.c -o ./fichier.o

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

