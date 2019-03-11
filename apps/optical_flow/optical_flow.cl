// Copyright (C) 2013-2017 Altera Corporation, San Jose, California, USA. All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to
// whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// This agreement shall be governed in all respects by the laws of the State of California and
// by the laws of the United States of America.

// Optical Flow implementation as described in this paper:
// 
// Pyramidal Implementation of the Lucas Kanade Feature Tracker
// Description of the algorithm by Jean-Yves Bouguet
//   http://robots.stanford.edu/cs223b04/algo_tracking.pdf
// 
// written by Dmitry Denisenko in January, 2013
//
// D17 is dense, 56x56, non-iterative, non-pyramidal version.
// Uses running column sums to compute running window sums
// with *fixed* number of arithmetic ops (fixed with respect
// to window size).


// Algorithm parameters
// WINDOW_SIZE can be defined on command-line
#ifndef WINDOW_SIZE
#define WINDOW_SIZE 56
#endif
#define WIDTH 1280
#define HEIGHT 800

// Flow vector scaling factor
#define FLOW_SCALING_FACTOR (1.0f/4.0f)

// Image access without bounds wrapping.
#define PO(image,x,y,W) ((image)[(y)*(W)+(x)])
// Image access with bounds wrapping.
#define P(image,x,y,W,H) ((image)[( (y)>=(H) ? (H)-1 : ((y)<0 ? 0:(y)) )*(W)+( (x)>=W ? W-1 : ((x)<0 ? 0 : (x)) )])

// hard-coded colorwheel constants
constant int l_ncols = 55;
constant int l_colorwheel[] = {
  255, 0, 0,
  255, 17, 0,
  255, 34, 0,
  255, 51, 0,
  255, 68, 0,
  255, 85, 0,
  255, 102, 0,
  255, 119, 0,
  255, 136, 0,
  255, 153, 0,
  255, 170, 0,
  255, 187, 0,
  255, 204, 0,
  255, 221, 0,
  255, 238, 0,
  255, 255, 0,
  213, 255, 0,
  170, 255, 0,
  128, 255, 0,
  85, 255, 0,
  43, 255, 0,
  0, 255, 0,
  0, 255, 63,
  0, 255, 127,
  0, 255, 191,
  0, 255, 255,
  0, 232, 255,
  0, 209, 255,
  0, 186, 255,
  0, 163, 255,
  0, 140, 255,
  0, 116, 255,
  0, 93, 255,
  0, 70, 255,
  0, 47, 255,
  0, 24, 255,
  0, 0, 255,
  19, 0, 255,
  39, 0, 255,
  58, 0, 255,
  78, 0, 255,
  98, 0, 255,
  117, 0, 255,
  137, 0, 255,
  156, 0, 255,
  176, 0, 255,
  196, 0, 255,
  215, 0, 255,
  235, 0, 255,
  255, 0, 255,
  255, 0, 213,
  255, 0, 170,
  255, 0, 128,
  255, 0, 85,
  255, 0, 43
};

int get_matrix_inv (int* G, float* G_inv);
int get_gradient_inv (constant uchar * restrict im1, int i, int j, int W, int H, float *G_inv);
void computeColor (float fx, float fy, uchar4 *pix);


kernel
void optical_flow_for_images (global uchar * restrict im1, global uchar * restrict im2,
                              global uchar4 * restrict out) {

  // some shift registers will actually be much smaller
  // (and the compiler will detect that)
  #define SHIFT_REG_SIZE ( (WINDOW_SIZE+2) * WIDTH + (WINDOW_SIZE+1) )
  #define RES_REG_SIZE   ( SHIFT_REG_SIZE - WIDTH )
  uchar im1_reg[SHIFT_REG_SIZE];
  uchar im2_reg[SHIFT_REG_SIZE];
  
  // column sums (height = WINDOW_SIZE)
  int col_ixix_reg[RES_REG_SIZE];
  int col_ixiy_reg[RES_REG_SIZE];
  int col_iyiy_reg[RES_REG_SIZE];
  int col_deltaK_ix_reg[RES_REG_SIZE];
  int col_deltaK_iy_reg[RES_REG_SIZE];
  
  // square sums (WINDOW_SIZE x WINDOW_SIZE)
  int sq_ixix_reg[RES_REG_SIZE];
  int sq_ixiy_reg[RES_REG_SIZE];
  int sq_iyiy_reg[RES_REG_SIZE];
  int sq_deltaK_ix_reg[RES_REG_SIZE];
  int sq_deltaK_iy_reg[RES_REG_SIZE];
  
  #pragma unroll
  for (uint i = 0; i < SHIFT_REG_SIZE; i++) {
    im1_reg[i] = 0;
    im2_reg[i] = 0;
  }
  #pragma unroll
  for (uint i = 0; i < RES_REG_SIZE; i++) {
    
    col_ixix_reg[i] = 0;
    col_ixiy_reg[i] = 0;
    col_iyiy_reg[i] = 0;
    col_deltaK_ix_reg[i] = 0;
    col_deltaK_iy_reg[i] = 0;

    sq_ixix_reg[i] = 0;
    sq_ixiy_reg[i] = 0;
    sq_iyiy_reg[i] = 0;
    sq_deltaK_ix_reg[i] = 0;
    sq_deltaK_iy_reg[i] = 0;
  }
  
  int i = 0;
  int j = 0;
  while (j < HEIGHT) {
    // shift the image value into shift reg
    #pragma unroll
    for (uint i_shift = 1; i_shift < SHIFT_REG_SIZE; i_shift++) {
      im1_reg[i_shift-1] = im1_reg[i_shift];
      im2_reg[i_shift-1] = im2_reg[i_shift];
    }
    im1_reg[SHIFT_REG_SIZE-1] = PO(im1, i, j, WIDTH);
    im2_reg[SHIFT_REG_SIZE-1] = PO(im2, i, j, WIDTH);
    
    #pragma unroll
    for (uint i_shift = 1; i_shift < RES_REG_SIZE; i_shift++) {
      
      col_ixix_reg[i_shift-1] = col_ixix_reg[i_shift];
      col_ixiy_reg[i_shift-1] = col_ixiy_reg[i_shift];
      col_iyiy_reg[i_shift-1] = col_iyiy_reg[i_shift];
      col_deltaK_ix_reg[i_shift-1] = col_deltaK_ix_reg[i_shift];
      col_deltaK_iy_reg[i_shift-1] = col_deltaK_iy_reg[i_shift];
      
      sq_ixix_reg[i_shift-1] = sq_ixix_reg[i_shift];
      sq_ixiy_reg[i_shift-1] = sq_ixiy_reg[i_shift];
      sq_iyiy_reg[i_shift-1] = sq_iyiy_reg[i_shift];
      sq_deltaK_ix_reg[i_shift-1] = sq_deltaK_ix_reg[i_shift];
      sq_deltaK_iy_reg[i_shift-1] = sq_deltaK_iy_reg[i_shift];
    }
    
    #define IX(P)         ((im1_reg[(P)+1] - im1_reg[(P)-1]) >> 1)
    #define IY(P)         ((im1_reg[(P)+WIDTH] - im1_reg[(P)-WIDTH]) >> 1)
    #define DELTAK(P)     (im1_reg[(P)] - im2_reg[(P)])
    #define IXIX(P)       (IX(P)*IX(P))
    #define IXIY(P)       (IX(P)*IY(P))
    #define IYIY(P)       (IY(P)*IY(P))
    #define DELTAK_IX(P)  (DELTAK(P)*IX(P))
    #define DELTAK_IY(P)  (DELTAK(P)*IY(P))
    
    
    // Image difference. Formula #30
    // Computing deltaK, Ix, Iy for tap point RES_REG_SIZE-1
    int deltaK = DELTAK(RES_REG_SIZE-1);
    // one pixel delayed
    int cIx = IX(RES_REG_SIZE-1);
    // one row delayed
    int cIy = IY(RES_REG_SIZE-1);
    
    // value from previous *row*, subtract top-most value, add new one
    col_ixix_reg[RES_REG_SIZE-1] = col_ixix_reg[RES_REG_SIZE-1-WIDTH] - IXIX(RES_REG_SIZE-1-WINDOW_SIZE*WIDTH) + cIx * cIx;
    col_ixiy_reg[RES_REG_SIZE-1] = col_ixiy_reg[RES_REG_SIZE-1-WIDTH] - IXIY(RES_REG_SIZE-1-WINDOW_SIZE*WIDTH) + cIx * cIy;
    col_iyiy_reg[RES_REG_SIZE-1] = col_iyiy_reg[RES_REG_SIZE-1-WIDTH] - IYIY(RES_REG_SIZE-1-WINDOW_SIZE*WIDTH) + cIy * cIy;
    col_deltaK_ix_reg[RES_REG_SIZE-1] = col_deltaK_ix_reg[RES_REG_SIZE-1-WIDTH] - DELTAK_IX(RES_REG_SIZE-1-WINDOW_SIZE*WIDTH) + deltaK * cIx;
    col_deltaK_iy_reg[RES_REG_SIZE-1] = col_deltaK_iy_reg[RES_REG_SIZE-1-WIDTH] - DELTAK_IY(RES_REG_SIZE-1-WINDOW_SIZE*WIDTH) + deltaK * cIy;
    
    // previous value, subtract last column, add new column
    sq_ixix_reg[RES_REG_SIZE-1] = sq_ixix_reg[RES_REG_SIZE-2] - col_ixix_reg[RES_REG_SIZE-1-WINDOW_SIZE] + col_ixix_reg[RES_REG_SIZE-1];
    sq_ixiy_reg[RES_REG_SIZE-1] = sq_ixiy_reg[RES_REG_SIZE-2] - col_ixiy_reg[RES_REG_SIZE-1-WINDOW_SIZE] + col_ixiy_reg[RES_REG_SIZE-1];
    sq_iyiy_reg[RES_REG_SIZE-1] = sq_iyiy_reg[RES_REG_SIZE-2] - col_iyiy_reg[RES_REG_SIZE-1-WINDOW_SIZE] + col_iyiy_reg[RES_REG_SIZE-1];
    sq_deltaK_ix_reg[RES_REG_SIZE-1] = sq_deltaK_ix_reg[RES_REG_SIZE-2] - col_deltaK_ix_reg[RES_REG_SIZE-1-WINDOW_SIZE] + col_deltaK_ix_reg[RES_REG_SIZE-1];
    sq_deltaK_iy_reg[RES_REG_SIZE-1] = sq_deltaK_iy_reg[RES_REG_SIZE-2] - col_deltaK_iy_reg[RES_REG_SIZE-1-WINDOW_SIZE] + col_deltaK_iy_reg[RES_REG_SIZE-1];
    
    float G_inv[4] = {0.0f, 0.0f, 0.0f, 0.0f};      
    // Guess for optical flow for each iteration
    float mu[2] = {0, 0};
    int G[4] = {0, 0, 0, 0};
    // Image mismatch vector. Formula #29
    int b_k[2] = {0, 0};
    
    G[0] = sq_ixix_reg[RES_REG_SIZE-1];
    G[1] = sq_ixiy_reg[RES_REG_SIZE-1];
    G[2] = G[1];
    G[3] = sq_iyiy_reg[RES_REG_SIZE-1];
    b_k[0] = sq_deltaK_ix_reg[RES_REG_SIZE-1];
    b_k[1] = sq_deltaK_iy_reg[RES_REG_SIZE-1];
    
    get_matrix_inv (G, G_inv);
    // If not invertible, G_inv will stay as 0s
    
    float fx=0.0f, fy=0.0f;
    // Avoiding borders
    if (j >= (WINDOW_SIZE/2 + 1) && i >= (WINDOW_SIZE/2 + 1) ) {
      // Optical flow. Formula #28
      fx = G_inv[0] * b_k[0] + G_inv[1] * b_k[1];
      fy = G_inv[2] * b_k[0] + G_inv[3] * b_k[1];
    }
    
    int zj = j - WINDOW_SIZE;
    int zi = i - WINDOW_SIZE;

    if (zi >= 0 && zj >= 0) {
      uchar4 result;
      computeColor (fx * FLOW_SCALING_FACTOR, fy * FLOW_SCALING_FACTOR, &result);
      PO(out, zi, zj, WIDTH) = result;
   }
    // increment (j, i) loop counters
    j = (i+1)>=(WIDTH) ? (j+1) : j;
    i = (i+1)>=(WIDTH) ? 0 : (i+1);
  }
}

// Find inverse of 2x2 int matrix
// returns 1 on success, 0 on failure
int get_matrix_inv (int* G, float* G_inv) {

  // need to cast elements of G to float before multiplication,
  // to avoid integer overflow.
  float detG = (float)G[0] * G[3]  - (float)G[1] * G[2];
  if (detG <= 1.0f) {
    return 0;
  }

  float detG_inv = 1.0f / detG;
  G_inv[0] =  G[3] * detG_inv;
  G_inv[1] = -G[1] * detG_inv;
  G_inv[2] = -G[2] * detG_inv;
  G_inv[3] =  G[0] * detG_inv;
  
  return 1;
}


// colorcode.cpp
//
// Color encoding of flow vectors
// adapted from the color circle idea described at
//   http://members.shaw.ca/quadibloc/other/colint.htm
//
// Daniel Scharstein, 4/2007
// added tick marks and out-of-range coding 6/05/07
//
// THIS IS SIMPLIFIED version of that code
void computeColor (float fx, float fy, uchar4 *pix)
{
  float rad = sqrt(fx * fx + fy * fy);
  float a = atan2(-fy, -fx) / (float)M_PI;
  float fk = (a + 1.0f) / 2.0f * (l_ncols-1);
  int k0 = (int)fk;
  
  int4 col;
  col.s0 = *(l_colorwheel + k0*3 + 0);
  col.s1 = *(l_colorwheel + k0*3 + 1);
  col.s2 = *(l_colorwheel + k0*3 + 2);
  if (rad > 1) rad = 1; // max at 1
  col.s0 = 255 - rad * (255 - col.s0); // increase saturation with radius
  col.s1 = 255 - rad * (255 - col.s1); // increase saturation with radius
  col.s2 = 255 - rad * (255 - col.s2); // increase saturation with radius
  col.s3 = 0;
  *pix = convert_uchar4(col);
}
