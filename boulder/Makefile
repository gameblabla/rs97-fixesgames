CHAINPREFIX=/opt/rs97-toolchain
CROSS_COMPILE=$(CHAINPREFIX)/usr/bin/mipsel-linux-
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip
SYSROOT     := $(CHAINPREFIX)/usr/mipsel-buildroot-linux-uclibc/sysroot
SDL_CFLAGS  := $(shell $(SYSROOT)/usr/bin/sdl-config --cflags)
SDL_LIBS    := $(shell $(SYSROOT)/usr/bin/sdl-config --libs)
LD = $(CROSS_COMPILE)gcc

CFLAGS = -c -Wall -O2 -mips32 -mno-mips16 $(SDL_CFLAGS)
LIBS = $(SDL_LIBS) -lSDL_ttf -lSDL_mixer -s
LINKOBJ = niveau.o joueur.o liste_joueur.o temps.o touche.o liste.o diamant.o liste_diamant.o pierre.o liste_pierre.o explosion.o liste_explosion.o ennemi.o liste_ennemi.o slim.o mur_qui_bouge.o environnement.o environnement_joueur.o environnement_ennemi.o environnement_diamant.o environnement_explosion.o environnement_pierre.o environnement_slim.o  fichier.o moteur_graphique.o boulder_graphique_acc.o boulder_graphique_affiche.o boulder_graphique_calcul.o boulder_graphique_cons.o boulder_graphique_couleurs.o boulder_graphique_sprite.o interface.o main.o jeu.o menu.o

.PHONY: all

all: main

main: $(LINKOBJ)
	$(CC) $(LINKOBJ) -o boulder $(LIBS)  

main.o:	./src/main.c
	$(CC) $(CFLAGS) ./src/main.c -o main.o
interface.o:	./src/interface/interface.c ./src/interface/interface.h
	$(CC) $(CFLAGS) ./src/interface/interface.c -o interface.o

jeu.o:	./src/jeu/jeu.c ./src/jeu/jeu.h
	$(CC) $(CFLAGS) ./src/jeu/jeu.c

menu.o:	./src/menu/menu.c	./src/menu/menu.h
	$(CC) $(CFLAGS) ./src/menu/menu.c

boulder_graphique_acc.o:	./src/boulder_graphique/boulder_graphique_acc.c ./src/boulder_graphique/boulder_graphique.h
	$(CC) $(CFLAGS) ./src/boulder_graphique/boulder_graphique_acc.c -o boulder_graphique_acc.o

boulder_graphique_affiche.o:	./src/boulder_graphique/boulder_graphique_affiche.c ./src/boulder_graphique/boulder_graphique.h
	$(CC) $(CFLAGS) ./src/boulder_graphique/boulder_graphique_affiche.c -o boulder_graphique_affiche.o


boulder_graphique_calcul.o:	./src/boulder_graphique/boulder_graphique_calcul.c ./src/boulder_graphique/boulder_graphique.h
	$(CC) $(CFLAGS) ./src/boulder_graphique/boulder_graphique_calcul.c -o boulder_graphique_calcul.o

boulder_graphique_cons.o:	./src/boulder_graphique/boulder_graphique_cons.c ./src/boulder_graphique/boulder_graphique.h
	$(CC) $(CFLAGS) ./src/boulder_graphique/boulder_graphique_cons.c -o boulder_graphique_cons.o

boulder_graphique_couleurs.o:	./src/boulder_graphique/boulder_graphique_couleurs.c ./src/boulder_graphique/boulder_graphique.h
	$(CC) $(CFLAGS) ./src/boulder_graphique/boulder_graphique_couleurs.c -o boulder_graphique_couleurs.o

boulder_graphique_sprite.o:	./src/boulder_graphique/boulder_graphique_sprite.c ./src/boulder_graphique/boulder_graphique.h
	$(CC) $(CFLAGS) ./src/boulder_graphique/boulder_graphique_sprite.c -o boulder_graphique_sprite.o


moteur_graphique.o:	./src/moteur_graphique/moteur_graphique.c ./src/moteur_graphique/moteur_graphique.h
	$(CC) $(CFLAGS) ./src/moteur_graphique/moteur_graphique.c -o moteur_graphique.o

environnement.o:	./src/environnement/environnement.c ./src/environnement/environnement.h
	$(CC) $(CFLAGS) ./src/environnement/environnement.c -o environnement.o

environnement_joueur.o:	./src/environnement/environnement_joueur.c ./src/environnement/environnement.h
	$(CC) $(CFLAGS) ./src/environnement/environnement_joueur.c -o environnement_joueur.o

environnement_diamant.o:	./src/environnement/environnement_diamant.c ./src/environnement/environnement.h
	$(CC) $(CFLAGS) ./src/environnement/environnement_diamant.c -o environnement_diamant.o

environnement_pierre.o:	./src/environnement/environnement_pierre.c ./src/environnement/environnement.h
	$(CC) $(CFLAGS) ./src/environnement/environnement_pierre.c -o environnement_pierre.o

environnement_explosion.o: ./src/environnement/environnement_explosion.c ./src/environnement/environnement.h
	$(CC) $(CFLAGS) ./src/environnement/environnement_explosion.c -o environnement_explosion.o

environnement_ennemi.o:	./src/environnement/environnement_ennemi.c ./src/environnement/environnement.h
	$(CC) $(CFLAGS) ./src/environnement/environnement_ennemi.c -o environnement_ennemi.o

environnement_slim.o:	./src/environnement/environnement_slim.c ./src/environnement/environnement.h
	$(CC) $(CFLAGS) ./src/environnement/environnement_slim.c -o environnement_slim.o


niveau.o:	./src/niveau/niveau.c ./src/niveau/niveau.h  slim.o liste_pierre.o liste_ennemi.o liste_joueur.o  liste_diamant.o
	$(CC) $(CFLAGS) ./src/niveau/niveau.c -o niveau.o

liste_explosion.o: ./src/explosion/liste_explosion/liste_explosion.c ./src/explosion/liste_explosion/liste_explosion.h
	$(CC) $(CFLAGS) ./src/explosion/liste_explosion/liste_explosion.c -o liste_explosion.o

fichier.o:	./src/fichier/fichier.c ./src/fichier/fichier.h
	$(CC) $(CFLAGS) ./src/fichier/fichier.c -o ./fichier.o

liste.o: ./src/liste/liste.c ./src/liste/liste.h
	$(CC) $(CFLAGS) ./src/liste/liste.c -o liste.o

explosion.o: ./src/explosion/type_explosion/explosion.c ./src/explosion/type_explosion/explosion.h
	$(CC) $(CFLAGS) ./src/explosion/type_explosion/explosion.c -o explosion.o

liste_joueur.o:	./src/joueur/liste_joueur/liste_joueur.c ./src/joueur/liste_joueur/liste_joueur.h	joueur.o liste.o
	$(CC) $(CFLAGS) ./src/joueur/liste_joueur/liste_joueur.c -o liste_joueur.o

joueur.o:	./src/joueur/type_joueur/joueur.c ./src/joueur/type_joueur/joueur.h	temps.o
	$(CC) $(CFLAGS) ./src/joueur/type_joueur/joueur.c -o joueur.o

temps.o:	./src/evenement/temps/temps.h ./src/evenement/temps/temps.c
	$(CC) $(CFLAGS) ./src/evenement/temps/temps.c -o temps.o

touche.o:	./src/evenement/touche/touche.h ./src/evenement/touche/touche.c
	$(CC) $(CFLAGS) ./src/evenement/touche/touche.c -o touche.o


slim.o: ./src/slim/slim.c ./src/slim/slim.h
	$(CC) $(CFLAGS) ./src/slim/slim.c -o slim.o

diamant.o:	./src/diamant/type_diamant/diamant.c ./src/diamant/type_diamant/diamant.h
	$(CC) $(CFLAGS) ./src/diamant/type_diamant/diamant.c -o diamant.o

liste_diamant.o:	./src/diamant/liste_diamant/liste_diamant.c ./src/diamant/liste_diamant/liste_diamant.h diamant.o liste.o
	$(CC) $(CFLAGS) ./src/diamant/liste_diamant/liste_diamant.c -o liste_diamant.o

pierre.o:	./src/pierre/type_pierre/pierre.c ./src/pierre/type_pierre/pierre.h
	$(CC) $(CFLAGS) ./src/pierre/type_pierre/pierre.c -o pierre.o

liste_pierre.o:	./src/pierre/liste_pierre/liste_pierre.c ./src/pierre/liste_pierre/liste_pierre.h pierre.o liste.o
	$(CC) $(CFLAGS) ./src/pierre/liste_pierre/liste_pierre.c -o liste_pierre.o

ennemi.o:	./src/ennemi/type_ennemi/ennemi.c ./src/ennemi/type_ennemi/ennemi.h
	$(CC) $(CFLAGS) ./src/ennemi/type_ennemi/ennemi.c -o ennemi.o

liste_ennemi.o:	./src/ennemi/liste_ennemi/liste_ennemi.c ./src/ennemi/liste_ennemi/liste_ennemi.h ennemi.o liste.o
	$(CC) $(CFLAGS) ./src/ennemi/liste_ennemi/liste_ennemi.c -o liste_ennemi.o


mur_qui_bouge.o: ./src/mur_qui_bouge/mur_qui_bouge.c ./src/mur_qui_bouge/mur_qui_bouge.h
	$(CC) $(CFLAGS) ./src/mur_qui_bouge/mur_qui_bouge.c -o mur_qui_bouge.o



clean:
	rm *.o; rm *.out; rm *~

