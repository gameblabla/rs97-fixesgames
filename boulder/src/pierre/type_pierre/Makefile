CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all:  test_pierre

pierre.o: pierre.c pierre.h
	gcc $(CFLAGS) pierre.c -o pierre.o


test_pierre:   pierre.o test_pierre.o
	gcc test_pierre.o pierre.o -o test_pierre.out

test_pierre.o: test_pierre.c
	gcc $(CFLAGS) test_pierre.c -o test_pierre.o



clean:
	rm *.o; rm *.out; rm *~




