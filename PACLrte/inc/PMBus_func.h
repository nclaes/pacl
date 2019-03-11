
#ifndef PMBUS_FUNC_H
#define PMBUS_FUNC_H


#include "common_defines.h"


float readPower(unsigned char channel, unsigned char device, bool usage);
float check_Vin(unsigned char channel, unsigned device, bool usage);
void  writeVout(unsigned char channel, unsigned char device,  float voltage);
class voltageScaling{

    public:

    int pmbus;
    
    voltageScaling();

    void apply_HPS(float level);
    void apply_NO_VS();
    void apply_FULL_VS();
    void apply_FPGA_ONLY();
    void apply_HPS_ONLY();
    void apply_HPS_MIN_Peripheral();
    void apply_HPS_MAX_Peripheral();
    ~voltageScaling();

};

class voltSettings{

public:

    static float voltList[VOLTLIST];

};


#endif
