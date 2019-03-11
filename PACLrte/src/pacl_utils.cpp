

#include <time.h>

#include "stdio.h"
#include <string.h>
#include "../inc/pacl.h"
#include <iostream>
#include <unistd.h>
#include "gopt.h"


using std::cout;
using std::endl;


bool Modes::Run_select = false; // Default Emulator Mode

int cmdParser::rebuild_flag = 0;

cmdParser::cmdParser(){

	options[0].long_name  = "help";
	options[0].short_name = 'h';
	options[0].flags      = GOPT_ARGUMENT_FORBIDDEN;

	options[1].long_name  = "version";
	options[1].short_name = 'V';
	options[1].flags      = GOPT_ARGUMENT_FORBIDDEN;

	options[2].long_name  = "rebuild";
	options[2].short_name = 'r';
	options[2].flags      = GOPT_ARGUMENT_FORBIDDEN;

	options[3].long_name  = "mode";
	options[3].short_name = 'm';
	options[3].flags      = GOPT_ARGUMENT_REQUIRED;

	options[4].flags      = GOPT_LAST;

}


void cmdParser::parseOptions(int iargc, char* iargv[]){
	

	iargc = gopt (iargv, options);
	gopt_errors (iargv[0], options);


	if (options[0].count)
	{
		fprintf (stdout, options[0].argument);

		exit (EXIT_SUCCESS);
	}

	if (options[1].count)
	{
		fprintf (stdout, "version 1.0\n");
		exit (EXIT_SUCCESS);
	}

	if (options[2].count)
	{
		fputs ("Profiler will run again and models will be re-built...\n", stderr);
		rebuild_flag = 1;
	}

	if (options[3].count)
	{
		fprintf (stdout, options[3].argument);
	}


}


	cmdParser::~cmdParser(){}





// High-resolution timer.
double getCurrentTime() {
 
  timespec a;
  clock_gettime(CLOCK_MONOTONIC, &a);
  return (double(a.tv_nsec) * 1.0e-9) + double(a.tv_sec);
 
}



void print_kernel_status(cl_int status){   
    
    switch(status){
    case 0:
        printf("\n PACL::CL_COMPLETE");   // 0
        break;
    case 1:
        printf("\n PACL::CL_RUNNING");    // 1
        break;    
    case 2:
        printf("\n PACL::CL_SUBMITTED");  // 2
        break;
    case 3:
        printf("\n PACL::CL_QUEUED");     // 3
        break; 
    }

}

// for error checking 
void error_check( int err, char* err_code ) {
	if ( err != CL_SUCCESS ) {
		printf("Error: %s  %d \n",err_code, err);
		exit(-1);
	}
}

unsigned char* kernel_name(cl_kernel kernel){   // returns kernel function name
  
    unsigned char* kname = new unsigned char[kernelNameLenght];	
    int err = clGetKernelInfo (kernel, CL_KERNEL_FUNCTION_NAME, sizeof(unsigned char)*kernelNameLenght,	kname, NULL);
    //printf("\n PACL::kname = %s", kname);
    
   
	return kname;

}

const char* constructFileName(const char* name, char* type){

    char kp[]=" powerReadings";
    char * kernelFile = new char[strlen(name) + strlen(type) +1];
    strcpy(kernelFile, name);
    strcat(kernelFile, type);
 
    return kernelFile;
} 

// save readings to a file
void save2File(unsigned int samples, const char* fName, float* readings, int start){

	FILE* checkFile;
	FILE* dataFile;

	//dataFile = fopen(fName, "w");
    /*
	if (checkFile = fopen(fName, "r")){   // check if exists, detele
        fclose(checkFile);
        remove(fName);
    }*/

	
	dataFile = fopen(fName, "a");

	for(int n=0;n<samples;n++) {

	   fprintf(dataFile, "%f\n", readings[n]);

	}

	fclose(dataFile);


}


void parseCommandLine(int g_argc, char* g_argv[]){
	int mode_flag, opt, iterations;
    mode_flag=0;
		
		if(g_argc==1){
			std::cout << "PACL::No args set, running default Emulator mode!" << std::endl;
		}else{
   		 	while ((opt = getopt(g_argc, g_argv, "fn:en:")) != -1) {

       	 		switch (opt) {
        		case 'f':
            		mode_flag++;
					std::cout << "PACL::FPGA mode selected!" << std::endl;
					//usage = true;
                    Modes::Run_select = fpga_version;
            		break;
	    		case 'e':
            		mode_flag++;
					std::cout << "PACL::Emulator mode selected!" << std:: endl;
					//usage = false;
                     Modes::Run_select = emulator_version;
            		break;
        		case 'n':
            		iterations = atoi(optarg);
					std::cout << "Iterations =" << iterations << std::endl;
            		break;
        		case '?': /* '?' */
            		fprintf(stderr, "Usage: %s [-e emulator mode] [-f fpga mode] [-n <iterations>] \n", g_argv[0]);
            		exit(EXIT_FAILURE);
      	  		}
		
			}
			
		}
    	
	if(mode_flag>1){
		std:cout << "Cannot select two modes at the same time!" << std::endl;
		exit(1);
	}

	//std::cout << "PACL::RunMode = " << Modes::Run_select << std::endl;

}

float generateConstrain(float a, float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

int RandomInt(int min, int max){
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}


