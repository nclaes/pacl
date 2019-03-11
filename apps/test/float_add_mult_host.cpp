#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include <string>
#include "time.h"
#include "assert.h"
#include <iostream>
#include <time.h>
#include <unistd.h>
#include "pthread.h"

#include "CL/opencl.h"
#include "AOCLUtils/aocl_utils.h"

// PACL header
#include "pacl.h"


#undef NDEBUG
#define N 10000

// Update interval for performance constrain
#define udpatePerfIntr 20
// Update interval for power budget
#define udpatePowIntr 5

using namespace std;
using namespace aocl_utils;


cl_device_id device;
cl_platform_id platform;
cl_int err;
cl_uint num_platforms;
cl_uint num_devices;
cl_context context;
cl_command_queue queue_add;
cl_program program;
cl_event event_add;
cl_int status;
cl_int stat;


 /*required for run_PACL function*/
float hostPowerBudget;
float hostPerfConstrain;
int udpatePerfCntr = 0;
int udpatePowCntr = 0;


// Function to run PACL optimisation routines
void run_PACL(pacl::Runtime& pacl_RT);



int main(int argc, char* argv[]) {


	// Initialize PACL runtime
	pacl::Runtime pacl_RT(argc, argv);
	// Define operation mode of Runtime Adaptation Unit (RAU)
	pacl_RT.RAU_mode("power");


	int ret, i;

	platform = findPlatform("Intel(R) FPGA");

	//Query devices.
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
	num_devices = 1; // One device, Intel FPGA.

	//Create OpenCL Context.
	context = clCreateContext(0, num_devices, &device, &oclContextCallback, NULL, &err);

	//Create command queue.
	cl_command_queue queue_add =  clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
	//Build program from binary file.
	std::string binary_file = getBoardBinaryFile("bin/float_add_mult", device);
	program = createProgramFromBinary(context, binary_file.c_str(), &device, 1);

	//Build Program.
	clBuildProgram(program, num_devices, &device, "", NULL, NULL);
			
	// Create kernels
	cl_kernel kernel_add = clCreateKernel(program, "float_add", &err);
	// Create memory objects
	float *x_add = (float *)malloc(N*sizeof(float));
	cl_mem x_buffer_add = clCreateBuffer(context, CL_MEM_READ_ONLY, N*sizeof(float), NULL, &err);
	float *y_add = (float *)malloc(N*sizeof(float));
	cl_mem y_buffer_add = clCreateBuffer(context, CL_MEM_READ_ONLY, N*sizeof(float), NULL, &err);
	float *z_add = (float *)malloc(N*sizeof(float));
	cl_mem z_buffer_add = clCreateBuffer(context, CL_MEM_WRITE_ONLY, N*sizeof(float), NULL, &err);
	
	// Set kernel arguments
	clSetKernelArg(kernel_add, 0, sizeof(cl_mem), (void *)&x_buffer_add);
	clSetKernelArg(kernel_add, 1, sizeof(cl_mem), (void *)&y_buffer_add);
	clSetKernelArg(kernel_add, 2, sizeof(cl_mem), (void *)&z_buffer_add);
	
	// Seed random number generator
	srand48(time(NULL));
	

	while(1){

	// Function to enable PACL features in the main application loop
	run_PACL(pacl_RT);


	// Create input data
	for(i = 0; i < N; i++) {
		x_add[i] = (float)drand48();
		y_add[i] = (float)drand48();
	}
	
	// Write to memory objects
	clEnqueueWriteBuffer(queue_add, x_buffer_add, CL_TRUE, 0, N*sizeof(float), x_add, 0, NULL, NULL);
	clEnqueueWriteBuffer(queue_add, y_buffer_add, CL_TRUE, 0, N*sizeof(float), y_add, 0, NULL, NULL);

	const size_t global_work_size = N;
	const size_t local_work_size = 1;


	clEnqueueNDRangeKernel(queue_add, kernel_add, (cl_uint)1, 0, &global_work_size, &local_work_size, 	0, NULL, &event_add);
	//clGetEventInfo(event_add, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &status, NULL);
	//printf("clEnqueueNDRangeKernel returned1\n");
	
	clFinish(queue_add);

	double total_time1, total_time2;
	cl_ulong start_time1, end_time1;
  	clGetEventProfilingInfo(event_add, CL_PROFILING_COMMAND_START, sizeof(start_time1), &start_time1, NULL);
  	clGetEventProfilingInfo(event_add, CL_PROFILING_COMMAND_END, sizeof(end_time1), &end_time1, NULL);
 	total_time1 = end_time1 - start_time1;


	clEnqueueReadBuffer(queue_add, z_buffer_add, CL_TRUE, 0, N*sizeof(float), z_add, 0, NULL, NULL);	
	clReleaseEvent(event_add);

	}


	// Clean up
	ret = clReleaseKernel(kernel_add);
	assert(ret == CL_SUCCESS);
	ret = clReleaseCommandQueue(queue_add);
	assert(ret == CL_SUCCESS);
	ret = clReleaseMemObject(x_buffer_add);
	assert(ret == CL_SUCCESS);
	ret = clReleaseMemObject(y_buffer_add);
	assert(ret == CL_SUCCESS);
	ret = clReleaseMemObject(z_buffer_add);
	assert(ret == CL_SUCCESS);
	ret = clReleaseProgram(program);
	assert(ret == CL_SUCCESS);
	ret = clReleaseContext(context);
	assert(ret == CL_SUCCESS);

	free(x_add);
	free(y_add);
	free(z_add);


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