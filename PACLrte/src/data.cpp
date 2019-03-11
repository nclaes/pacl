#include "stdio.h"
#include <unistd.h>
#include "../inc/pacl.h"
#include <iostream>

using std::cout;
using std::endl;



typedef struct {

	const char* name;
	double sum;
	double avPower;
	double avEnergy;

}kernelInfo;




void collect_kernel_power(cl_event* event, cl_kernel* kernel, const int samplingPeriod, bool usage, int kSize){

	
	kernelInfo kStruct;
	


	cl_int stats;
	const int numKernels = kSize / 4 ;

	//cout << "kSize = " << kSize << endl;
	//cout << "Kernels = " << numKernels << endl;

	int finished = 0;
	int lastFinished = 0;
	//int * doneAlert = new int [numKernels];
	//doneAlert = {0};
	int doneAlert[numKernels];

	cl_int status = !CL_COMPLETE;
    unsigned int samples = 0;
	double sum = 0;
    float *saveP = new float[sampleArraySize];  //estimate size for array ex.time/samples

 	//printf("\nPACL::Collecting Kernel Power Data !\n"); 

	

	do {  // collect readings while kernel is executing
		
		saveP[samples] = readPower(FPGA_1_1, FPGA_MONITOR, usage);
		sum+= saveP[samples];

	    //usleep(samplingPeriod); // For emulation
        //print_kernel_status(status);

		for (int k = 0; k < numKernels; k++){
			clGetEventInfo(event[k], CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &stats, NULL);
			
		if (stats == CL_COMPLETE && doneAlert[k] != 1){
				
				finished++;
				doneAlert[k] = 1;
				cout << "PACL::Kernel " << k << " has finished execution! "<< endl;
					
			} 
		}
		   
			

		 if (finished == numKernels) status = CL_COMPLETE;

		
		
		

		samples++;
		
	} while(status != CL_COMPLETE && samples < sampleArraySize);


	//printf("\nPACL::numb kernels = %d\n", numKernels); 
	//printf("\nPACL::finished = %d\n", finished); 

   printf("\nPACL::samples = %d\n", samples);   

 


	//const char* kernelFunctionName = (const char*)kernel_name(kernel);


	//const char* kernelFile =  constructFileName(kernelFunctionName, "_collected_power");
   // save2File(samples, kernelFile, saveP);						

	//printAverageKernelPE(event[lastFinished], samples, sum);

	

	
	delete [] saveP;
	//delete kernelFunctionName;

}





void printAverageKernelPE(cl_event event, unsigned int samples, double sum ){


	
	double total_time1, total_time2;
	cl_ulong start_time1, end_time1;
  	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(start_time1), &start_time1, NULL);
  	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(end_time1), &end_time1, NULL);
 	total_time1 = end_time1 - start_time1;
     printf("SExecution Time is %.3fms\n", total_time1/1000000.0);
    double breakdown [2];

	breakdown[0] = sum/samples;  // estimate average kernel power
	breakdown[1] = breakdown[0]*((total_time1/1000000.0)/1000.0); //estime average energy
	//std::cout << "Sum = " << sum << std::endl;
	//std::cout << "Samples = " << samples << std::endl
	std::cout << "PACL::Average P = " << breakdown[0]<< " W" << std::endl; 
	std::cout << "PACL::Average Energy = " << breakdown[0]*((total_time1/1000000.0)/1000.0) << " J" <<std::endl; 

/*
 
       float integ = 0;

       float timeP =  ((total_time1/1000000.0)/1000.0)/samples;

	for(int n=0;n<samples-1;n++) {

	   integralP+= timeP*((saveP[n] + saveP[n+1])*0.5); 

	}
	
	
*/


	//std::cout << "Integral P = " << integralP << " J" <<std::end


}

float idlePower(){


	//cout << "VS MODE =" << Modes::VS_MODE << endl;


	float idleP; 
	float Vin = check_Vin(HPS_VIN, HPS_MONITOR, Modes::Run_select);
    

	if(Vin < 12){
		cout << "PACL::Vin below 13V ! " << endl;
		switch(Modes::VS_MODE){
			case 0:
				idleP = 0.2868*Vin + 6.8533; // NO VS
				break;
			case 1:
				idleP = 0.2697*Vin + 5.9041; // FULL VS
				break;
			case 2:
				idleP = 0.2667*Vin + 6.4671 ;// FPGA only
				break;
			case 3:
				idleP = 0.2763*Vin + 6.4292; // HPS only
				break;
		}
	}else{
	    cout << "PACL::Vin above 13V ! " << endl;
		switch(Modes::VS_MODE){
			case 0:
				idleP = IDLE_P_ABOVE_SENSE_RANGE;  							    // NO VS
				break;
			case 1:
				idleP = IDLE_P_ABOVE_SENSE_RANGE - IDLE_FULL_VS_SAVINGS;        // FULL VS
				break;
			case 2:
				idleP =  IDLE_P_ABOVE_SENSE_RANGE - IDLE_FPGA_ONLY_SAVINGS;  	// FPGA only
				break;
			case 3:
				idleP =  IDLE_P_ABOVE_SENSE_RANGE - IDLE_HPS_ONLY_SAVINGS;  	// HPS only
				break;
		}	

	}

	cout << "PACL::IDLE power = " << idleP << "w" << endl;

	return idleP;

}

