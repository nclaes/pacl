


#ifndef CLOCK_H
#define CLOCK_H

#include "common_defines.h"


class freqSettings{

public:
    static unsigned int denominatorList[FREQLIST];
    static unsigned int freqList[FREQLIST];
    static unsigned int numeratorList[FREQLIST];

};


void delay(unsigned int milliseconds);
unsigned int binary_conversion(unsigned int num);
unsigned int update_MAIN_PLL_VCO_NUMER(unsigned int VCO_ptr_val, unsigned int target);
unsigned int update_MAIN_PLL_VCO_DENOM(unsigned int VCO_ptr_val, unsigned int target);
unsigned int update_MAIN_PLL_MPUCLK_CNT(unsigned int MPUCLK_ptr_val, unsigned int target);
unsigned int check_frequency_setting(unsigned int VCO_ptr_val, unsigned int MPUCLK_ptr_val);

void check_MAIN_PLL_LOCK_STATUS(unsigned int LOCK_ptr_val);

unsigned int clear_MAIN_PLL_LOCK_STATUS(unsigned int LOCK_ptr_val);

bool check_frequency_change(unsigned int currentFreqency, unsigned int desiredFrequency);

void find_frequency(unsigned int desiredFrequency, unsigned int* requiredDenom, unsigned int* requiredNumer, int* retIndex);

void set_hps_freq(unsigned int desiredFrequency);




#endif