#ifndef PERF_H
#define PERF_H

#include <time.h>

#include "common_defines.h"
#include <vector>
#include <iostream>
#include <math.h>
#include <fstream>
#define PERF_THRESHOLD 0


class perfHost{

private:

double start_timer, stop_timer;
double execTime;

public:


 std::fstream dataFile;

 static FILE* perfConstrainFile;
 static FILE* perfDataFile;
 static FILE*  freqOverheadsFile;

 static int perfStatus;
 static int sampleCount;
 static int profBegin;
 static float sum;
 static int freqCount;
 static int checkBegin;

 float averageFPS[FREQLIST];
 
float lastFPSConstrain;

static float checkSum;
static int sampleCount1;
static float currentFPS;

static float freqIndx;
float perfLUT[FREQLIST];

  perfHost();
  void begin(int freqNum, int setSamples, int modelStatus);
  int checkStatus();
  void checkPerf(int setSamples);
  void collect(int freqNum, int setSamples);
  double captureTime();
  void start();
  void stop();

  int readPerfFile();
  void setPerformanceConstrain(float perfConstrain);
  float* output();
  float MAX_FPS();
  float MIN_FPS();

  ~perfHost();


};



#endif

	