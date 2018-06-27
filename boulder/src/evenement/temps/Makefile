CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all:  test_temps

temps.o: temps.c temps.h
	gcc $(CFLAGS) temps.c -o temps.o


test_temps:   temps.o test_temps.o
	gcc test_temps.o temps.o -o test_temps.out -lSDL

test_temps.o: test_temps.c
	gcc $(CFLAGS) test_temps.c -o test_temps.o



clean:
	rm *.o; rm *~; rm *.out




