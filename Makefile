CC=gcc
LD=gcc
CFLAGS=-I. -ggdb -std=c99 -Wall
LDFLAGS= -lm

all: lodepng_code rotate

.PHONY: lodepng_code

rotate: rotate.o pixutils.h pixutils.o lodepng/lodepng.o
	$(LD) -o rotate rotate.o pixutils.o lodepng/lodepng.o $(LDFLAGS)

rotate.o : rotate.c
	$(CC) -o rotate.o -c $(CFLAGS) rotate.c

pixutils.o : pixutils.c
	$(CC) -o pixutils.o -c $(CFLAGS) pixutils.c

lodepng_code:
	$(MAKE) -C lodepng
	
clean:
	$(MAKE) -C lodepng clean
	rm *.o
	rm rotate
