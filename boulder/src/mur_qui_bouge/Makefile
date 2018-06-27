CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all:  test_mur_qui_bouge

mur_qui_bouge.o: mur_qui_bouge.c mur_qui_bouge.h
	gcc $(CFLAGS) mur_qui_bouge.c -o mur_qui_bouge.o


test_mur_qui_bouge:   mur_qui_bouge.o test_mur_qui_bouge.o temps.o
	gcc test_mur_qui_bouge.o mur_qui_bouge.o temps.o -o test_mur_qui_bouge.out -lSDL

test_mur_qui_bouge.o: test_mur_qui_bouge.c
	gcc $(CFLAGS) test_mur_qui_bouge.c -o test_mur_qui_bouge.o

temps.o:	../evenement/temps/temps.c ../evenement/temps/temps.h
	gcc $(CFLAGS) ../evenement/temps/temps.c -o temps.o

clean:
	rm *.o; rm *~; rm *.out




