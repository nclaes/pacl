

#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H


#define kernelNameLenght 50
#define sampleArraySize 4096 // kernel sample array size
#define kernelMeasurePeriod 10000     // power measure period

#define NO_VS 0
#define FULL_VS 1
#define FPGA_VS 2
#define HPS_VS 3

#define IDLE_P_ABOVE_SENSE_RANGE 10.86  // Average Idle Power 13-19V range 
#define IDLE_FULL_VS_SAVINGS 1.23
#define IDLE_HPS_ONLY_SAVINGS 0.54
#define IDLE_FPGA_ONLY_SAVINGS 0.64

#define emulator_version 0
#define fpga_version 1

#define PERF_RUNNING 0



// For perf.cpp and model.cpp
#define FAILED 2       // Model failed
#define SUCCESS 1      // Model was built



#define FREQLIST 24  // Available frequencies
#define VOLTLIST 12


/*
------------------------------------ HPS frequency LIST---------------------------------------------------

Index        0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23
Frequency    925,900,850,800,750,700,650,600,550,500,450,400,350,300,275,250,225,200,175,150,137,125,112,100

*/


class Modes {
public:
    static bool Run_select;
    static unsigned int VS_MODE;
};





#endif