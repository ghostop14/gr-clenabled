/*
 * GRCLBase.h
 *
 *  Created on: Feb 9, 2017
 *      Author: root
 */

#ifndef LIB_GRCLBASE_H_
#define LIB_GRCLBASE_H_

// Use CPP exception handling.
// Note: If you include cl.hpp, the compiler just won't find cl::Error class.
// You have to use cl2.hpp to get it to go away
#define __CL_ENABLE_EXCEPTIONS
// Disable the deprecated functions warning.  If you want to keep support for 1.2 devices
// You need to use the deprecated functions.  This #define makes the warning go away.
// #define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_VERSION_1_2

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#include "clSComplex.h"
#include "../include/clenabled/api.h"

#define DTYPE_COMPLEX 1
#define DTYPE_FLOAT 2
#define DTYPE_INT 3

#define OCLTYPE_GPU 1
#define OCLTYPE_ACCELERATOR 2
#define OCLTYPE_CPU 3
#define OCLTYPE_ANY 4


namespace gr {
namespace clenabled {

class CLENABLED_API GRCLBase {
protected:
     int dataType;
	 size_t dataSize;
	 int preferredWorkGroupSizeMultiple=1;
	 int maxWorkGroupSize=1;
	 int maxConstMemSize=0;
	 int optimalBufferType = CL_MEM_USE_HOST_PTR;

    int platformMode;

    virtual void cleanup();

    // opencl variables
    cl::Program *program=NULL;
    cl::Context *context=NULL;
    std::vector<cl::Device> devices;
    cl::Program::Sources *sources=NULL;
    cl::CommandQueue *queue=NULL;
    cl::Kernel *kernel=NULL;

    cl_device_type contextType;

    std::string platformName="";
    std::string platformVendor="";
    std::vector<std::string> deviceNames;
    std::vector<std::string> deviceTypes;

    void CompileKernel(const char* kernelCode, const char* kernelFunctionName);

public:
	GRCLBase(int idataType, size_t dsize,int openCLPlatformType);
	virtual ~GRCLBase();


    cl_device_type GetContextType();

};

} /* namespace clenabled */
} /* namespace gr */

#endif /* LIB_GRCLBASE_H_ */
