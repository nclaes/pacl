#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include "saveImage.h"

void saveImage(const unsigned int imageCols, const unsigned int 
imageRows, unsigned char *data)
{
//Define file pointer
FILE *pFile = NULL;

//Open file in binary writing mode, as a PGM file (portable grey map)
pFile = fopen("output.pgm", "wb");

//Print the initial magic number byte, followed by dimensions
//P5 header for PGM, max value is 255 (white byte)
fprintf(pFile,"P5\n 1920 1080\n 255\n");

//Write one byte of greyscale char each iteration, for total number of pixels
fwrite(data,imageRows*imageCols,1,pFile);

//Close file
fclose(pFile);

}
