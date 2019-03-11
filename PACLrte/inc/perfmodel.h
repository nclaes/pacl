#ifndef PERFMODEL_H
#define PERFMODEL_H



#include <math.h>       /* pow */
#include<iostream>
#include<iomanip>

#include "common_defines.h"

using std::cout;
using std::endl;


#define PERF_ERROR 50  //Model Max error allowance





class perfModel
{

public:

// Settings

static const int n = FREQLIST;
static const int d = 3; // degree of polynomial

///

float predictedFPS;
float q, m;
int  i, j, k;
static int perfDone;
float A[d+1][d+2];
unsigned int *x;
static int status;
float modelData[FREQLIST];

perfModel();

float predict(int freqIndex);
void perf(int perfStatus, float* averageFPS);
static FILE* dataFile;
void recollect();



~perfModel();




};





#endif