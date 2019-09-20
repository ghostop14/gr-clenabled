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

#include <boost/thread/mutex.hpp>

#define DTYPE_COMPLEX 1
#define DTYPE_FLOAT 2
#define DTYPE_INT 3

#define OCLTYPE_GPU 1
#define OCLTYPE_ACCELERATOR 2
#define OCLTYPE_CPU 3
#define OCLTYPE_ANY 4

#define OCLDEVICESELECTOR_FIRST 1
#define OCLDEVICESELECTOR_SPECIFIC 2

namespace gr {
namespace clenabled {

extern bool CLPRINT_NITEMS;  // Enable this in GRCLBase.cpp to print the number of items passed to the work functions if debug is enabled for the module

class CLENABLED_API GRCLBase {
protected:
     int dataType;
	 size_t dataSize;
	 int preferredWorkGroupSizeMultiple=1;
	 int maxWorkGroupSize=1;
	 int maxConstMemSize=0;
	 bool hasSharedVirtualMemory;
	 bool hasSVMFineGrained;
	 int optimalBufferType = CL_MEM_USE_HOST_PTR;

	 bool hasSingleFMASupport=false;  // Fused Multiply/Add
	 bool hasDoubleFMASupport=false;  // Fused Multiply/Add
	 bool hasDoublePrecisionSupport= false;

	 int maxConstItems = 0;

	 bool debugMode = false;

    int platformMode;

    boost::mutex d_mutex;

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

    bool CompileKernel(const char* kernelCode, const char* kernelFunctionName, bool exitOnFail=true);

	virtual void InitOpenCL(int idataType, size_t dsize,int openCLPlatformType, int devSelector,int platformId, int devId, bool setDebug=false,bool outOfOrderQueue=false);

public:
    bool hasSVMAvailable() { return hasSharedVirtualMemory; };

    int MaxConstItems() { return maxConstItems; };

    std::string debugDisplayName = "";

    void SetDebugMode(bool newMode) { debugMode = newMode; };

    int ActiveContextType() {return contextType;};

    std::string getPlatformName() { return platformName; };
    std::string getVendorName() { return platformVendor; };

	GRCLBase(int idataType, size_t dsize,int openCLPlatformType, bool setDebug=false,bool outOfOrderQueue=false); // selects First of specified type
	GRCLBase(int idataType, size_t dsize,int openCLPlatformType, int devSelector,int platformId, int devId, bool setDebug=false,bool outOfOrderQueue=false);
	virtual ~GRCLBase();
    virtual bool stop();

    cl_device_type GetContextType();

};

} /* namespace clenabled */
} /* namespace gr */

#endif /* LIB_GRCLBASE_H_ */
