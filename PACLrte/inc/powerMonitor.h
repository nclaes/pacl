#ifndef POWERMONITOR_H
#define POWERMONITOR_H



#include <math.h>       /* pow */
#include<iostream>
#include<iomanip>

#include <stdio.h>
#include "common_defines.h"
#include "pthread.h"
#include "PMBus_def.h"
#include "PMBus_func.h"
#include <string.h>
#include "pacl_utils.h"

using std::cout;
using std::endl;


#define SAMPLING_DELAY 30
#define VMAX 1.1
#define VMIN 1.0

#define VMAX_1_5 1.5
#define VMIN_1_5 1.36

#define VMAX_2_5 2.5
#define VMIN_2_5 2.28

#define VMAX_3_3 3.3
#define VMIN_3_3 3.0

#define FMAX 925
#define FMIN 100
#define FREF 925

#define POW_THR_MAX 0.05
#define POW_THR_MIN 0.01
#define POWER_THRESHOLD0 0.025
#define POWER_THRESHOLD1 0.03
#define MEASURE_ITERATIONS 4


void* start_Monitor(void * arg);



class powerMonitorFeatures{


public:

    typedef struct powerSamples{

        //HPS 1.1 rail
        float SUM_HPS_1_1V; // Sum of samples
        float SUM_HPS_1_5V; // Sum of samples
        float SUM_HPS_2_5V; // Sum of samples
        float SUM_HPS_3_3V; // Sum of samples
        
        //for HPS rails
        float HPS_1_1V;     // Power sample
        float HPS_1_5V;
        float HPS_2_5V;
        float HPS_3_3V;

        float AVR_HPS_1_1V;     // Average power
        float AVR_HPS_1_5V;     // Average power 1.5V
        float AVR_HPS_2_5V;     // Average power 2.5V
        float AVR_HPS_3_3V;     // Average power 3.3V

        // 0 - PV_MAX, 1 - PV_MIN, 2 - PF_MIN,  PV_MAX = PF_MAX, 3 - PMIN (Lowest F and V)
        float DATA_HPS_1_1[MEASURE_ITERATIONS];

        float HPS_PMIN, HPS_PMAX;

        float HPS_1_5V_MAX;
        float HPS_1_5V_MIN;
        float HPS_2_5V_MAX;
        float HPS_2_5V_MIN;
        float HPS_3_3V_MAX;
        float HPS_3_3V_MIN;

        // 0 - alfa0, 1 - alfa1, 2 - beta1
        float MODEL_HPS_1_1[20];
        float MODEL_HPS_PERIPHERAL_1_5[3];
        float MODEL_HPS_PERIPHERAL_2_5[3];
        float MODEL_HPS_PERIPHERAL_3_3[3];
        
        float PREDICTED_HPS_1_1;
        float PREDICTED_HPS_1_5;
        float PREDICTED_HPS_2_5;
        float PREDICTED_HPS_3_3;
        float PREDICTED_HPS_1_1_NEW;
        float PREDICTED_HPS_1_1_NEW_1;

        float HPS_1_1_LUT[FREQLIST];

        float HPS_1_1_ENERGY[FREQLIST];


        

    }powerSample;

    powerSample pwrSample;

    static FILE* predictedPowerFile;
    static FILE* predictedNewPowerFile;
    static FILE* predictedNew1PowerFile;

    static FILE* measuredPowerFile;
    static FILE* powerBudgetFile;
    static FILE* averagePowerFile;
    static FILE* powerModelFile;

    static FILE* HPS_1_5_File;
    static FILE* HPS_2_5_File;   
    static FILE* HPS_3_3_File;

    static int collectingSamples;
    static int collectStatus;
    
   



    int buildStatusHPS_1_1;
    int applied;
    int collectedSamples;

    voltageScaling setVoltage;

    float lastPowerBudget;
    static float initialVoltageLevel;
    float newVoltageLevel;
    int   newFreqIndex;

    powerMonitorFeatures();
    void setPowerBudget();
    void estimateAveragePower();
    void collectDataHPS_1_1(int iterNum);
    void realTimeMonitoring();
    void buildModelHPS_1_1();
    void buildModelHPS_1_1_Peripheral();
    int findMinErrorIndex(float powerBudget, float LUT[]);
    void meetPowerConstrain(float powerConstrain);
    void estimateEnergy_HPS_1_1(float* fpsData);
    int findMinEnergy_HPS_1_1();


    float predictPowerHPS_1_1(int fVal, float vLevel);
    float predictPowerHPS_1_5(float vLevel);
    float predictPowerHPS_2_5(float vLevel);
    float predictPowerHPS_3_3(float vLevel);
    float predictPowerHPS_1_1_NEW(int fVal, float vLevel);
    float predictPowerHPS_1_1_NEW_1(int fVal, float vLevel);
    float predictSlopeHPS(int fVal);

    void recordAveragePower();
    void applyVFScaling(float randomVoltage, int randomFrequencyIndex);
    void findSettings(float powerConstrain);
    ~powerMonitorFeatures();



};



class powerMonitor
{

public:

//int updateCounter;
int powerBudgetUpdateInterval;
int threadCheck;
static int threadActive;
powerMonitorFeatures monitorTask; // Create Oject

static int powerModelStatus;

pthread_t monitorThread;
pthread_attr_t attr;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;


powerMonitor();
float HPS_PMIN();
float HPS_PMAX();
float HPS_AVERAGE_1_1();
void printPowerBudgetRangeHPS();
void startMonitorThread();
void updatePowerBudget(float pwrBudget, int userThreshold);

~powerMonitor();

};







#endif