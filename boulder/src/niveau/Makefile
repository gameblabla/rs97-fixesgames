CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all: test_niveau

test_niveau: ./output/test_niveau.o ./output/niveau.o ./output/joueur.o ./output/liste_joueur.o ./output/temps.o ./output/touche.o ./output/liste.o ./output/diamant.o ./output/liste_diamant.o ./output/pierre.o ./output/liste_pierre.o ./output/explosion.o ./output/liste_explosion.o ./output/ennemi.o ./output/liste_ennemi.o ./output/slim.o ./output/mur_qui_bouge.o ./output/fichier.o -lSDL

	gcc ./output/test_niveau.o ./output/niveau.o ./output/joueur.o ./output/liste_joueur.o ./output/temps.o ./output/touche.o ./output/liste.o ./output/diamant.o ./output/liste_diamant.o ./output/pierre.o ./output/liste_pierre.o ./output/explosion.o ./output/liste_explosion.o ./output/ennemi.o ./output/liste_ennemi.o ./output/slim.o ./output/mur_qui_bouge.o -o test_niveau.out ./output/fichier.o -lSDL


./output/test_niveau.o:	test_niveau.c
	gcc $(CFLAGS) test_niveau.c -o ./output/test_niveau.o

./output/niveau.o:	niveau.c niveau.h
	gcc $(CFLAGS) niveau.c -o ./output/niveau.o

./output/liste_explosion.o: ../explosion/liste_explosion/liste_explosion.c ../explosion/liste_explosion/liste_explosion.h
	gcc $(CFLAGS) ../explosion/liste_explosion/liste_explosion.c -o ./output/liste_explosion.o


./output/liste.o: ../liste/liste.c ../liste/liste.h
	gcc $(CFLAGS) ../liste/liste.c -o ./output/liste.o

./output/explosion.o: ../explosion/type_explosion/explosion.c ../explosion/type_explosion/explosion.h
	gcc $(CFLAGS) ../explosion/type_explosion/explosion.c -o ./output/explosion.o

./output/liste_joueur.o:	../joueur/liste_joueur/liste_joueur.c ../joueur/liste_joueur/liste_joueur.h	
	gcc $(CFLAGS) ../joueur/liste_joueur/liste_joueur.c -o ./output/liste_joueur.o

./output/joueur.o:	../joueur/type_joueur/joueur.c ../joueur/type_joueur/joueur.h	
	gcc $(CFLAGS) ../joueur/type_joueur/joueur.c -o ./output/joueur.o

./output/temps.o:	../evenement/temps/temps.h ../evenement/temps/temps.c
	gcc $(CFLAGS) ../evenement/temps/temps.c -o ./output/temps.o

./output/touche.o:	../evenement/touche/touche.h ../evenement/touche/touche.c
	gcc $(CFLAGS) ../evenement/touche/touche.c -o ./output/touche.o


./output/slim.o: ../slim/slim.c ../slim/slim.h
	gcc $(CFLAGS) ../slim/slim.c -o ./output/slim.o

./output/diamant.o:	../diamant/type_diamant/diamant.c ../diamant/type_diamant/diamant.h
	gcc $(CFLAGS) ../diamant/type_diamant/diamant.c -o ./output/diamant.o

./output/liste_diamant.o:	../diamant/liste_diamant/liste_diamant.c ../diamant/liste_diamant/liste_diamant.h
	gcc $(CFLAGS) ../diamant/liste_diamant/liste_diamant.c -o ./output/liste_diamant.o

./output/pierre.o:	../pierre/type_pierre/pierre.c ../pierre/type_pierre/pierre.h
	gcc $(CFLAGS) ../pierre/type_pierre/pierre.c -o ./output/pierre.o

./output/liste_pierre.o:	../pierre/liste_pierre/liste_pierre.c ../pierre/liste_pierre/liste_pierre.h
	gcc $(CFLAGS) ../pierre/liste_pierre/liste_pierre.c -o ./output/liste_pierre.o

./output/ennemi.o:	../ennemi/type_ennemi/ennemi.c ../ennemi/type_ennemi/ennemi.h
	gcc $(CFLAGS) ../ennemi/type_ennemi/ennemi.c -o ./output/ennemi.o

./output/liste_ennemi.o:	../ennemi/liste_ennemi/liste_ennemi.c ../ennemi/liste_ennemi/liste_ennemi.h
	gcc $(CFLAGS) ../ennemi/liste_ennemi/liste_ennemi.c -o ./output/liste_ennemi.o

./output/fichier.o:	../fichier/fichier.c ../fichier/fichier.h
	gcc $(CFLAGS) ../fichier/fichier.c -o ./output/fichier.o

./output/mur_qui_bouge.o: ../mur_qui_bouge/mur_qui_bouge.c ../mur_qui_bouge/mur_qui_bouge.h
	gcc $(CFLAGS) ../mur_qui_bouge/mur_qui_bouge.c -o ./output/mur_qui_bouge.o



clean:
	rm ./output/*.o; rm *.out; rm *~

