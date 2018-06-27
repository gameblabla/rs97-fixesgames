CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all:  test_slim

slim.o: slim.c slim.h
	gcc $(CFLAGS) slim.c -o slim.o


test_slim:   slim.o test_slim.o
	gcc test_slim.o slim.o -o test_slim.out

test_slim.o: test_slim.c
	gcc $(CFLAGS) test_slim.c -o test_slim.o



clean:
	rm *.o; rm *~; rm *.out




