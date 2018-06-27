CC=gcc
LD=gcc
CFLAGS=-c -Wall -g

.PHONY: all

all: test_liste

test_liste:    test_liste.o liste.o
	gcc test_liste.o liste.o -o test_liste.out

test_liste.o:   liste.o  test_liste.c
	gcc $(CFLAGS) test_liste.c -o test_liste.o

liste.o:	liste.c liste.h
	gcc $(CFLAGS) liste.c -DTEST_AVEC_INT -o liste.o

clean:
	rm *.o; rm *.out; rm *~
