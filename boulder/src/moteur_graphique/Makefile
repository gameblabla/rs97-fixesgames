CC=gcc
LD=gcc
CFLAGS=-c -Wall -g
LIBS= -lSDL -lSDL_ttf
LINKOBJ= niveau.o joueur.o liste_joueur.o temps.o touche.o liste.o diamant.o liste_diamant.o pierre.o liste_pierre.o explosion.o liste_explosion.o ennemi.o liste_ennemi.o slim.o mur_qui_bouge.o environnement.o environnement_joueur.o environnement_ennemi.o environnement_diamant.o environnement_explosion.o environnement_pierre.o environnement_slim.o  fichier.o moteur_graphique.o test_moteur_graphique.o

.PHONY: all

all: test_moteur_graphique

test_moteur_graphique: $(LINKOBJ)
	gcc $(LINKOBJ) -o test_moteur_graphique.out $(LIBS)  


test_moteur_graphique.o:	test_moteur_graphique.c
	gcc $(CFLAGS) test_moteur_graphique.c -o test_moteur_graphique.o

moteur_graphique.o:	moteur_graphique.c moteur_graphique.h
	gcc $(CFLAGS) moteur_graphique.c -o moteur_graphique.o



environnement.o:	../environnement/environnement.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement.c -o environnement.o

environnement_joueur.o:	../environnement/environnement_joueur.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement_joueur.c -o environnement_joueur.o

environnement_diamant.o:	../environnement/environnement_diamant.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement_diamant.c -o environnement_diamant.o

environnement_pierre.o:	../environnement/environnement_pierre.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement_pierre.c -o environnement_pierre.o

environnement_explosion.o: ../environnement/environnement_explosion.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement_explosion.c -o environnement_explosion.o

environnement_ennemi.o:	../environnement/environnement_ennemi.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement_ennemi.c -o environnement_ennemi.o

environnement_slim.o:	../environnement/environnement_slim.c ../environnement/environnement.h
	gcc $(CFLAGS) ../environnement/environnement_slim.c -o environnement_slim.o


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

