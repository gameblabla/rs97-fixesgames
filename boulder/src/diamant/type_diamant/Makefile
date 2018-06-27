CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all:  test_diamant

diamant.o: diamant.c diamant.h
	gcc $(CFLAGS) diamant.c -o diamant.o


test_diamant:   diamant.o test_diamant.o
	gcc test_diamant.o diamant.o -o test_diamant.out

test_diamant.o: test_diamant.c
	gcc $(CFLAGS) test_diamant.c -o test_diamant.o



clean:
	rm *.o; rm *~; rm *.out




