#ifndef __pixutils_h
#define __pixutils_h
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "lodepng/lodepng.h"


#ifndef M_PI
 #define M_PI 3.14159265
#endif

#define degreesToRadians(degrees) (degrees * M_PI / 180.0)
#define MAXWIDTH 256

typedef struct{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
}rgba;

typedef struct{
	unsigned char *image;
	unsigned int imageWidth;
	unsigned int imageHeight;
	unsigned char arrayType;
	rgba  (*pixArray_arrays)[MAXWIDTH];
	rgba **pixArray_blocks;
	rgba **pixArray_overlay;
}pixMap;

//public methods
pixMap* pixMap_read(char *inputFile, unsigned char arrayType); //return zero pointer if failure
int pixMap_write(pixMap *p, char *outputFile); //return 1 if failure
int pixMap_rotate(pixMap *p,float theta);
void pixMap_destroy (pixMap **p); //free memory and set p to zero
#endif
