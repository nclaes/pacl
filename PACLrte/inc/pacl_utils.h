#ifndef PACL_UTILS_H
#define PACL_UTILS_H

#include "CL/opencl.h"
#include "gopt.h"


double getCurrentTime();
void print_kernel_status(cl_int status);
void error_check( int err, unsigned char* err_code);
unsigned char* kernel_name(cl_kernel kernel);
void save2File(unsigned int samples, const char* fName, float* readings, int start);
const char* constructFileName(const char* name, char* type);
void parseCommandLine(int g_argc, char* g_argv[]);
float generateConstrain(float a, float b);
int RandomInt(int min, int max);

class cmdParser{

	public:

    static int rebuild_flag;


	int argc;
	char* argv[];
	struct option options[5];

    cmdParser();

    void parseOptions(int iargc, char* iargv[]);

	~cmdParser();

};




#endif