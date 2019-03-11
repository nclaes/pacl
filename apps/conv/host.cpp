
#include <CL/opencl.h>
#include <algorithm>
#include <iostream>
#include "parseImage.h"
#include "imageDims.h"
#include "saveImage.h"
#include "AOCLUtils/aocl_utils.h"
#include "pthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <mqueue.h>
#include <signal.h>
#include <errno.h>
#include <string>

// PACL header
#include "pacl.h"
// Update interval for performance constrain
#define udpatePerfIntr 20
// Update interval for power budget
#define udpatePowIntr 5

using namespace aocl_utils;
using namespace std;
static unsigned int counter;


pthread_t meas_p;
pthread_attr_t attr;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
int kernel_status = 0;


cl_uint *input = NULL;
cl_uint *output = NULL;

//NEW BUFFERS
cl_uint *out_33 = NULL;
cl_uint *out_55 = NULL;

//OpenCL Functions.
cl_uint num_platforms;
cl_platform_id platform;
cl_uint num_devices;
cl_device_id device;
cl_context context;
cl_command_queue queue[2];
cl_program program;
cl_event events[2];
cl_event lol;

//NEW KERNELS. Launch TWO
cl_kernel kernel_list[2];


//Memory objects of type cl_mem.
cl_mem in_buffer, out_buffer;
cl_mem outbuff_33, outbuff_55;
cl_mem in_buff33, in_buff55;
 

//------------------------------Dynamics-------------------------------------//
//There are 3 bytes per pixel, so totalElements/3

#define LINE_BUFSIZE 128

int total_pixels = ROWS*COLS;
int kernel1_pixels = total_pixels;
int kernel2_pixels = total_pixels;

//Dynamic input buffers (coloured)
int bufferSize = ROWS*COLS*sizeof(unsigned int);
int bufferSizek1 = bufferSize;
int bufferSizek2 = bufferSize;

//Dynamic output buffers (greyscale)
int imageElements = ROWS*COLS;
int imageElementsk1 = ROWS*COLS;
int imageElementsk2 = ROWS*COLS;
float alloc_3x3 = 1;
float alloc_5x5 = 0;
//-----------------------------------------------------------------------------//

//Define total times, holding profiling info
double total_time1, total_time2;
size_t filterSize = 1;


void printTime(int kernel, cl_event* event)
{
	
  	cl_ulong start_time1, end_time1;
 	clGetEventProfilingInfo(*event, CL_PROFILING_COMMAND_START, sizeof(start_time1), &start_time1, NULL);
  	clGetEventProfilingInfo(*event, CL_PROFILING_COMMAND_END, sizeof(end_time1), &end_time1, NULL);
	total_time1 = end_time1 - start_time1;
	printf("Kernel %d Execution Time is %.3fms\n", kernel,(total_time1/1000000.0)/1000.0);
	//clReleaseEvent(event);
}

   /*required for run_PACL function*/

	float hostPowerBudget;
	float hostPerfConstrain;
	int udpatePerfCntr = 0;
	int udpatePowCntr = 0;


	// Function to run PACL optimisation routines
	void run_PACL(pacl::Runtime& pacl_RT);


int main(int argc, char **argv)
{
 
	// Initialize PACL runtime
	pacl::Runtime pacl_RT(argc, argv);
	// Define operation mode of Runtime Adaptation Unit (RAU)
	pacl_RT.RAU_mode("power");

	double startTime;
	double endTime;

 	double overall_time;
	std::string imageFile;
	//imageFile = "west_1.ppm";
	imageFile = "salvadore.ppm";
 
	cl_int status[2];	
  	float* abc;
  	float reading; 
  	const int period = 10000; // 10ms



  if (argc > 1) { 

	
  	alloc_3x3 = atoi(argv[1])/100.0f;
 	alloc_5x5 = 1.0f-alloc_3x3;

 	printf("3x3 allocation: %.f %\n", alloc_3x3*100);
 	printf("5x5 allocation: %.f %\n", alloc_5x5*100);

  	kernel1_pixels = total_pixels*alloc_3x3;
  	kernel2_pixels = total_pixels*alloc_5x5;

	bufferSizek1 = bufferSize*alloc_3x3;
	bufferSizek2 = bufferSize*alloc_5x5;

	imageElementsk1 = ROWS*COLS*alloc_3x3;
	imageElementsk2 = ROWS*COLS*alloc_5x5;

    input = (cl_uint*)alignedMalloc(bufferSize);
    output = (cl_uint*)alignedMalloc(imageElements);

	out_33 = (cl_uint*)alignedMalloc(imageElementsk1);
    out_55 = (cl_uint*)alignedMalloc(imageElementsk2); 

    cl_int err;

	platform = findPlatform("Intel(R) FPGA");

	//Query devices.
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
	num_devices = 1; // One device, Intel FPGA.

	//Create OpenCL Context.
	context = clCreateContext(0, num_devices, &device, &oclContextCallback, NULL, &err);

	//Create command queue.
	queue[0] = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
	queue[1] = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
	//Build program from binary file.
	std::string binary_file = getBoardBinaryFile("bin/laplacian", device);

	program = createProgramFromBinary(context, binary_file.c_str(), &device, 1);

	//Build Program.
	clBuildProgram(program, num_devices, &device, "", NULL, NULL);

	//Create the kernel.
	
	
	//Need to allow for writing of PPM headers and magic numbers
	//Create input buffer.m
	in_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, bufferSize, NULL, &err);

	//Create output buffer.
	out_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, imageElements, NULL, &err);
  


  if(alloc_3x3>0){
	 printf("Buffers for 3x3 allocated !\n");  
	 kernel_list[0] = clCreateKernel(program, "laplacian33", &err);
  	 in_buff33 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bufferSizek1, NULL, &err);
	 outbuff_33 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, imageElementsk1, NULL, &err);
     clSetKernelArg(kernel_list[0], 0, sizeof(cl_mem), &in_buff33);
     clSetKernelArg(kernel_list[0], 1, sizeof(cl_mem), &outbuff_33);
 	 clSetKernelArg(kernel_list[0], 2, sizeof(int), &kernel1_pixels);
	  
	  
   }
  
  if(alloc_5x5>0){
    printf("Buffers for 5x5 allocated !\n");   
	kernel_list[1] = clCreateKernel(program, "laplacian55", &err);
	outbuff_55 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, imageElementsk2, NULL, &err);
  	in_buff55 = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bufferSizek2, NULL, &err);
	clSetKernelArg(kernel_list[1], 0, sizeof(cl_mem), &in_buff55);
	clSetKernelArg(kernel_list[1], 1, sizeof(cl_mem), &outbuff_55);
	clSetKernelArg(kernel_list[1], 2, sizeof(int), &kernel2_pixels);

  }


    parseImage(imageFile.c_str(), COLS, ROWS, (unsigned char *)input);
 	clEnqueueWriteBuffer(queue[0], in_buffer, CL_TRUE, 0, bufferSize,  input, 0, NULL, NULL);
 
   
  }
  else {printf("Missing arguments! Specify kernel allocation (0-100) \n");  exit(1); }

	cl_int stat1, stat2;	

  
while(1){
	
	
    // Call function to enable PACL features
	run_PACL(pacl_RT);

	

  if(alloc_3x3>0 && alloc_5x5>0){

	clEnqueueCopyBuffer(queue[0], in_buffer, in_buff33, 0, 0, bufferSizek1, 0, NULL, NULL);
    clEnqueueCopyBuffer(queue[1], in_buffer, in_buff55, bufferSizek1, 0, bufferSizek2, 0, NULL, NULL);
	//printf("\nBoth kernels will be executed! \n");
	//sleep(1); 
		
	clEnqueueNDRangeKernel(queue[0], kernel_list[0], 1, NULL, &filterSize, &filterSize, 0, NULL, &events[0]);
	clEnqueueNDRangeKernel(queue[1], kernel_list[1], 1, NULL, &filterSize, &filterSize, 0, NULL, &events[1]);
	 
	//PACL_ANALYSE_KERNEL(events, kernel_list, sizeof(kernel_list));

 	/*do {  // collect readings while kernel is executing
	
		clGetEventInfo(events[0], CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &stat1, NULL);
		clGetEventInfo(events[1], CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &stat2, NULL);


	}while(stat2 != CL_COMPLETE || stat1 != CL_COMPLETE);
	*/
    
	clFinish(queue[0]);
	clFinish(queue[1]);

  
	double total_time1, total_time2;
	cl_ulong start_time1, end_time1;
  	clGetEventProfilingInfo(events[0], CL_PROFILING_COMMAND_START, sizeof(start_time1), &start_time1, NULL);
  	clGetEventProfilingInfo(events[0], CL_PROFILING_COMMAND_END, sizeof(end_time1), &end_time1, NULL);
 	total_time1 = end_time1 - start_time1;

   	//Print total time in milliseconds
  	//printf("\nKernel 3x3 Execution Time is %.3fms",total_time1/1000000.0);

	clGetEventProfilingInfo(events[1], CL_PROFILING_COMMAND_START, sizeof(start_time1), &start_time1, NULL);
  	clGetEventProfilingInfo(events[1], CL_PROFILING_COMMAND_END, sizeof(end_time1), &end_time1, NULL);
 	total_time1 = end_time1 - start_time1;

	//printf("\nKernel 5x5 Execution Time is %.3fms",total_time1/1000000.0);


 	clEnqueueCopyBuffer(queue[0], outbuff_33, out_buffer, 0, 0, imageElementsk1, 0, NULL, NULL);
 	clEnqueueCopyBuffer(queue[1], outbuff_55, out_buffer, 0, imageElementsk1, imageElementsk2, 0, NULL, NULL);	
 	clEnqueueReadBuffer(queue[0], out_buffer, CL_TRUE, 0, imageElements, output, 0, NULL, NULL);
        clEnqueueReadBuffer(queue[1], out_buffer, CL_TRUE, 0, imageElements, output, 0, NULL, NULL);
 
	
  }else{

   	if(alloc_3x3>0){

		clEnqueueCopyBuffer(queue[0], in_buffer, in_buff33, 0, 0, bufferSizek1, 0, NULL, NULL);
		//printf("\nOnly 3x3 will be executed !\n");  
		//sleep(1); 

		clEnqueueNDRangeKernel(queue[0], kernel_list[0], 1, NULL, &filterSize, &filterSize, 0, NULL, &events[0]);

	   // PACL_ANALYSE_KERNEL(&events[0], &kernel_list[0], sizeof(kernel_list[0]));

		//printTime(1, events[1]);
		
		clEnqueueCopyBuffer(queue[0], outbuff_33, out_buffer, 0, 0, imageElementsk1, 0, NULL, NULL);
		clEnqueueReadBuffer(queue[0], out_buffer, CL_TRUE, 0, imageElements, output, 0, NULL, NULL);

	} 
    else {   

		clEnqueueCopyBuffer(queue[1], in_buffer, in_buff55, bufferSizek1, 0, bufferSizek2, 0, NULL, NULL);
		//printf("\nOnly 5x5 will be executed!\n");        
		//sleep(1); 

		clEnqueueNDRangeKernel(queue[1], kernel_list[1], 1, NULL, &filterSize, &filterSize, 0, NULL, &events[1]);

		//PACL_ANALYSE_KERNEL(&events[1], &kernel_list[1], sizeof(kernel_list[1]));
		
		//printTime(2, events[2]);
		
		clEnqueueCopyBuffer(queue[1], outbuff_55, out_buffer, 0, imageElementsk1, imageElementsk2, 0, NULL, NULL);
		clEnqueueReadBuffer(queue[1], out_buffer, CL_TRUE, 0, imageElements, output, 0, NULL, NULL);
		
	}
  }
 
      
	clFinish(queue[0]);
	clFinish(queue[1]);
	
	clReleaseEvent(events[0]);
	clReleaseEvent(events[1]);

}

  if (input) alignedFree(input);
  if (output) alignedFree(output);
  if (out_33) alignedFree(out_33);
  if (out_55) alignedFree(out_55);
  if (in_buffer) clReleaseMemObject(in_buffer);
  if (in_buff33) clReleaseMemObject(in_buff33);
  if (in_buff55) clReleaseMemObject(in_buff55);
  if (outbuff_33) clReleaseMemObject(out_buffer);
  if (outbuff_55) clReleaseMemObject(out_buffer);
  if (out_buffer) clReleaseMemObject(out_buffer);
  if (kernel_list[0]) clReleaseKernel(kernel_list[0]);
  if (kernel_list[1]) clReleaseKernel(kernel_list[1]);
  if (program) clReleaseProgram(program);
  if (queue[1]) clReleaseCommandQueue(queue[1]);
  if (queue[0]) clReleaseCommandQueue(queue[0]);
  if (context) clReleaseContext(context);
		
  	
  return 0;
}


void run_PACL(pacl::Runtime& pacl_RT){
/*Check if profiling has finished and provide runtime application requirements*/
		if(pacl_RT.profiler()){
			
			/*-------------------Runtime Power Budget Update-----------------*/	

			// Generate value for power constrain every updatePowIntr iterations
			if(udpatePowCntr == udpatePowIntr){
				hostPowerBudget =  generateConstrain(pacl_RT.pwrMonitor.HPS_PMIN(), pacl_RT.pwrMonitor.HPS_PMAX());
				cout << "\n New Power Budget = " << hostPowerBudget;
				udpatePowCntr = 0;
			}else udpatePowCntr++;
			// Update power budget with the generated power constrain
			pacl_RT.setPowerBudget(hostPowerBudget, 50);
		
			/*-------------------Runtime Performance Constrain Update-----------------*/

			// Generate a new performance constrain every updatePerfIntr iterations
			if(udpatePerfCntr == udpatePerfIntr){
				hostPerfConstrain = generateConstrain(pacl_RT.perfMonitor.MIN_FPS(), pacl_RT.perfMonitor.MAX_FPS());
				cout << "\n\n New Perf(FPS) Constrain = " << hostPerfConstrain << endl;
				udpatePerfCntr = 0;
			}else udpatePerfCntr++;
			
			// Set a new porformance constrain
			pacl_RT.setPerfConstrain(hostPerfConstrain);

		}	
		
}


