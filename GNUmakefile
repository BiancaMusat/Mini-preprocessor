CC = gcc
CFLAGS = -Wall -g -std=c89

build: so-cpp

so-cpp: so-cpp.o hashtable.o
 
so-cpp.o: so-cpp.c

hashtable.o: hashtable.c
 
.PHONY: clean
 
clean:
	rm -fr *~ *.o so-cpp
