
#ifndef DATA_H
#define DATA_H



/*
         Step 9[3]: implementation of clGetKernelInfo
         here 'kernel_sum' is kernal for summation of matrices
              '2nd argument' is information to query
              '3rd argument' is the sizeof memory pointed to by '4th argument'
              '4th argument' is A pointer to memory where the appropriate result being queried is returned
              '5th argument' is the actual size of data copied to param_value
         */
 


void collect_kernel_power(cl_event* event, cl_kernel* kernel, const int samplingPeriod, bool usage, int kSize);
//struct kernelInfo;
void printAverageKernelPE(cl_event event, unsigned int samples, double sum);
float idlePower();


#endif