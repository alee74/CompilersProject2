# Makefile for Project One reference implemention.

CFLAGS=-Wall -O2

alloc:		alloc.o lexing.o parsing.o 
		gcc $(CFLAGS) -o alloc alloc.o lexing.o parsing.o

alloc.o:	alloc.c lexing.h parsing.h alloc.h
		gcc $(CFLAGS) -c alloc.c

parsing.o:	lexing.h parsing.c parsing.h
		gcc $(CFLAGS) -c parsing.c

lexing.o:	lexing.c lexing.h 
		gcc $(CFLAGS) -c lexing.c

clean:
		rm *.o
		rm alloc

wc:		
		wc -l lexing.h lexing.c parsing.h parsing.c alloc.h alloc.c

export:		lexing.c parsing.c alloc.c lexing.h parsing.h alloc.h Makefile README
		tar cvf export.tar Makefile README *.c *.h 
