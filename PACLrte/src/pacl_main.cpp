#include "stdio.h"
#include <unistd.h>
#include "../inc/pacl.h"
#include <iostream>

#include <fstream>
#include <sstream>
#include <string>


using std::cout;
using std::endl;




namespace pacl{


int Runtime::energy_flag = 0;

 
Runtime::Runtime(int argc, char* argv[]){

    // call to parse command line
    parser.parseOptions(argc, argv);

    if(parser.rebuild_flag == 0) {   // If user asks to rebuild models

        // Read profiled perf data and if success, set perfStatus to finished
        perfMonitor.perfStatus = perfMonitor.readPerfFile();  
        
        // Add for power data
        // pwrMonitor.powerModelStatus = 1; 


        // Add to estimate energy, set energy_flag = 1


    }
}

void Runtime::RAU_mode(char* appMode){ // get RAU Mode from user's application

    RAU_task = appMode;


}


void Runtime::searchPerfData(){

    ////
}


void Runtime::setPowerBudget(float hostPowerBudget, int TPerror){

    if(RAU_task == "power")
        pwrMonitor.updatePowerBudget(hostPowerBudget, TPerror);

}


void Runtime::setPerfConstrain(float fpsConstrain){

    if(RAU_task == "perf"){
        perfMonitor.checkPerf(1);
        perfMonitor.setPerformanceConstrain(fpsConstrain);
        
    }
}


int Runtime::profiler(){
	
   

	if(pwrMonitor.powerModelStatus == 1 && perfMonitor.perfStatus == 1){

        if(RAU_task == "energy" && energy_flag == 0){
            
            // Estimate energy
            pwrMonitor.monitorTask.estimateEnergy_HPS_1_1(perfMonitor.averageFPS);
            
            // Apply frequency for energy minimisation
            //set_hps_freq(pwrMonitor.monitorTask.findMinEnergy_HPS_1_1());
            //int indx = pwrMonitor.monitorTask.findMinEnergy_HPS_1_1();

            energy_flag = 1;
        }

    /*    if(RAU_task == "power")
            cout << "\n Measured HPS 1.1V Power = " << readPower(HPS_1_1, HPS_MONITOR, 1);
    */

          pwrMonitor.startMonitorThread();

		return 1;
    }
	else {
        pwrMonitor.startMonitorThread();
	    perfMonitor.begin(FREQLIST, 3, pwrMonitor.powerModelStatus);
        return 0;
    }

  



}

        

Runtime::~Runtime(){}



}



void PACL_ANALYSE_KERNEL(cl_event* event, cl_kernel* kernel, int kSize){

    collect_kernel_power(event, kernel, kernelMeasurePeriod,  Modes::Run_select, kSize);


}