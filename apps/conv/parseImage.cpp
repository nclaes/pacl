//Could pass values of w and h into main to process image of any width and height. Would also somehow need to pass as kernel


//----------Typical PPM Format----------//
//                P6                    //
//          height  width               //
//          RGB Intensity               //
//  Followed by the actual Pixel Data   //
//--------------------------------------//

#include <stdio.h>
#include <stdlib.h>

//Declare a struct of input image properties
struct image
{
char p;
int format;
unsigned int maxRGB;
unsigned int fWidth;
unsigned int fHeight;
};

bool parseImage(const char *filename, const unsigned int width, const unsigned int height, unsigned char *data)
{
  FILE *pImage = NULL;

  unsigned char* buffer;
  size_t valid;
  struct image in;
  pImage = fopen(filename, "rb");
  
  const size_t headerSize = 0x40;
  char header[headerSize];

  int readSuccess = 0;

  //sscanf function returns the number of elements written correctly, therefore if "P" and "6" are read, they are stored in the struct, then readSuccess is incremented by 2. And so on. 

  while (readSuccess < 5) {
    if (fgets(header, headerSize, pImage) != NULL) { 
  
    //Collate the data, reading line by line, ignoring comments and whitespaces
    if (header[0] == '#') continue;

	switch(readSuccess) {
	   case 0:
	   readSuccess += sscanf(header, "%c%d", &in.p, &in.format);
     	   break;

	   case 2:
	   readSuccess += sscanf(header,"%u %u", &in.fWidth, &in.fHeight);
	   break;
	
	   case 4:
	   readSuccess += sscanf(header, "%u", &in.maxRGB);
	   break;
	}
     }
   }

  //Verify if data collected is correct, for different images for example
  //printf("%c%d, %u, %u, %u", in.p, in.format, in.fWidth, in.fHeight, in.fmaxRGB);

  //Obtain size of ppm input file
  long totPixels = width*height;
  long totElements = width*height*3;

  //Manually allocate memory to buffer to be used for reading
  buffer = (unsigned char *)malloc(sizeof(unsigned char)*totElements);

  //Check if image data is of valid PPM format
  //Then read image data to the bufer, one byte at a time, for the total image
  //dimensions using the pImage pointer which specifies the input stream
  valid = fread(buffer, sizeof(unsigned char), totElements, pImage);
  if(valid != totElements) {
  fputs("Error while reading image.",stderr); 
  return false;
  }

  // Create pointers to increment through channels 1 byte at a time
  unsigned char *buffer_ptr = buffer;
  unsigned char *data_ptr = data;

  //Transfer image pixel data stored in buffer to the data pointer, to be returned to 
  //host program - incrementing char pointer 4 times allows us to write an unsigned int
  for (int i = 0; i != totPixels; ++i) {
    // Read rgb and pad
    *data_ptr++ = *buffer_ptr++; //red
    *data_ptr++ = *buffer_ptr++; //green
    *data_ptr++ = *buffer_ptr++; //blue
    *data_ptr++ = 0;
  }

  //Free the malloced data
  free(buffer);
  
  //Close file pointer
  fclose(pImage);
  return true;
}

