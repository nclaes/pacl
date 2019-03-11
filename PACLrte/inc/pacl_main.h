#ifndef PACL_MAIN_H
#define PACL_MAIN_H

#include "gopt.h"
#include "powerMonitor.h"
#include "perf.h"

namespace pacl{

class Runtime{


    public:

    cmdParser parser;

    powerMonitor pwrMonitor;
	perfHost perfMonitor;
    char* RAU_task;
    int RAU_status;

    Runtime(int argc, char* argv[]);

    int profiler();
    void setPowerBudget(float hostPowerBudget, int TPerror);
    void setPerfConstrain(float fpsConstrain);
    void RAU_mode(char* appMode);
    void searchPerfData();

    static int energy_flag;
	 


    ~Runtime();
};


}


void PACL_ANALYSE_KERNEL(cl_event* event, cl_kernel* kernel, int kSize);


#endif