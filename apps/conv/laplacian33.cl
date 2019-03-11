#include "imageDims.h"

//Kernel reads in unsigned int pixel data (4 bytes), and each cycle performs gets individual pixel channel (RBG) data through masking, then calculates image convolution. This is then limited to upper and lower values, and wrote to an output buffer. An additional program called saveImage writes a PGM (portable greymap) image to file by writing 1 byte of data each cycle for each greyscale byte value. 

//TRY AND PASS PARAMETERS FROM PARSEIMAGE AS HEIGHT AND WIDTH INSTEAD OF USING INCLUDE

__kernel
void laplacian33(global unsigned int * restrict frame_in, 
           global unsigned char * restrict frame_out,
           const int totalpixels)
{
    //Define convolution matrix
    int h_n[3][3] = {{0,-1,0},
                    {-1,4,-1},
                    {0,-1,0}};

    char set_white = 0xff;
    char set_black = 0x00;

    // Pixel buffer of 2 columns and 3 extra pixels
    int pix_buff[3 * COLS + 3];

    // Initial iterations initialize the pixel buffer
    int count = -(3 * COLS + 3);

    //Iterator of the output index for greyscale PGM image
    int outIndex = 0;

    while (count != totalpixels) {

    //Fill pixel buffer
    #pragma unroll
        for (int i = 3 * COLS + 2; i > 0; --i) {
            pix_buff[i] = pix_buff[i - 1];
        }
        pix_buff[0] = count >= 0 ? frame_in[count] : 0;

        //Initialise greyscale variable.
        char grey_scale = 0x00; 

        //Compute one convolution each cycle
        #pragma unroll
        for (int filterRow = 0; filterRow < 3; ++filterRow) {

            #pragma unroll
            for (int filterCol = 0; filterCol < 3; ++filterCol) {

		//Get the current pixel
                unsigned int pixel = pix_buff[filterRow * COLS + filterCol];

		//Retrieve the individual bytes of pixel by masking. This is actually 
		//0x00ffffff but the 4th padding byte is omitted
                unsigned char b = pixel & 0xff0000;
                unsigned char g = pixel & 0x00ff00;
                unsigned char r = pixel & 0x0000ff;

		//Convert three bytes of RGB --> one byte of greyscale data
	
		    //Approximate version, simply apply a ratio and shift
    		     unsigned char luma_apprx = ((2 * r) + b + (3 * g)) >> 3;

	            //Typecast luma as a char and perform convolution
		    grey_scale += (char)luma_apprx*h_n[filterRow][filterCol]; 

            }
        }

	//Can then easily convert back to colour scale once the processing has been done
	//on luma converted data - this is more efficient!

	//Limit greyscale, to white and black at the respective upper and lower limits 
	if(grey_scale > 0xff) grey_scale = set_white;
	else if(grey_scale < 0x00) grey_scale = set_black;
	  
       //Write the total number of greyscale bytes - start at zero 
       //to ensure over-writing pixel data does not occur
       if(outIndex != totalpixels) {
       frame_out[outIndex++] = grey_scale; //Write byte, iterate index
       }

       count++;  //Iterate overall count
    }

}

__kernel
void laplacian55(global unsigned int * restrict frame_in, 
           global unsigned char * restrict frame_out,
           const int totalpixels)
{
    //Define convolution matrix
    int h_n[5][5] = {{0,0,-1,0,0},
                     {0,-1,-2,-1,0},
                     {-1,-2,16,-2,-1},
		     {0,-1,-2,-1,0},
		     {0,0,-1,0,0}};

    char set_white = 0xff;
    char set_black = 0x00;

    // Pixel buffer. As the kernel size increases, the number of pixels in the buffer
    // must increase, leading to slower convolution but better performance. 
    int pix_buff[5 * COLS + 3];

    // Initial iterations initialize the pixel buffer
    int count = -(5 * COLS + 3);

    //Iterator of the output index for greyscale PGM image
    int outIndex = 0;

    while (count != totalpixels) {

    //Fill pixel buffer
    #pragma unroll
        for (int i = 5 * COLS + 2; i > 0; --i) {
            pix_buff[i] = pix_buff[i - 1];
        }
        pix_buff[0] = count >= 0 ? frame_in[count] : 0;

        //Initialise greyscale variable.
        char grey_scale = 0x00; 

        //Compute one convolution each cycle
        #pragma unroll
        for (int filterRow = 0; filterRow < 5; ++filterRow) {

            #pragma unroll
            for (int filterCol = 0; filterCol < 5; ++filterCol) {

		//Get the current pixel
                unsigned int pixel = pix_buff[filterRow * COLS + filterCol];

		//Retrieve the individual bytes of pixel by masking. This is actually 
		//0x00ffffff but the 4th padding byte is omitted
                unsigned char b = pixel & 0xff0000;
                unsigned char g = pixel & 0x00ff00;
                unsigned char r = pixel & 0x0000ff;

		//Convert three bytes of RGB --> one byte of greyscale data
		    //Approximate version, simply apply a ratio and shift
    		     unsigned char luma_apprx = ((2 * r) + b + (3 * g)) >> 3;

	            //Typecast luma as a char and perform convolution
		    grey_scale += (char)luma_apprx*h_n[filterRow][filterCol]; 


            }
        }

	//Can then easily convert back to colour scale once the processing has been done
	//on luma converted data - this is more efficient!

	//Limit greyscale, to white and black at the respective upper and lower limits 
	if(grey_scale > 0xff) grey_scale = set_white;
	else if(grey_scale < 0x00) grey_scale = set_black;
	  
       //Write the total number of greyscale bytes - start at zero 
       //to ensure over-writing pixel data does not occur
       if(outIndex != totalpixels) {
       frame_out[outIndex++] = grey_scale; //Write byte, iterate index
       }

       count++;  //Iterate overall count
    }

}
