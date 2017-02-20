/*
 * GRCLBase.cpp
 *
 *  Created on: Feb 9, 2017
 *      Author: root
 */

#include <iostream>

#include "GRCLBase.h"

namespace gr {
namespace clenabled {

GRCLBase::GRCLBase(int idataType, size_t dsize,int openCLPlatformType) {
	// TODO Auto-generated constructor stub

	dataType=idataType;
	platformMode=openCLPlatformType;

	// Now we set up our OpenCL space
    try {
        std::vector<cl::Platform> platformList;

        // Pick platform
        try {
            cl::Platform::get(&platformList);
        }
        catch(...) {
        	std::string errMsg = "OpenCL Error: Unable to get platform list.";
        	throw cl::Error(CL_DEVICE_NOT_FOUND,(const char *)errMsg.c_str());
        }

        std::string openclString="GPU";

        cl_device_type clType=CL_DEVICE_TYPE_GPU;
        switch(platformMode) {
        case OCLTYPE_CPU:
        	clType=CL_DEVICE_TYPE_CPU;
        	openclString = "CPU";
        break;
        case OCLTYPE_ACCELERATOR:
        	clType=CL_DEVICE_TYPE_ACCELERATOR;
        	openclString = "Accelerator";
        break;
        case OCLTYPE_ANY:
			try {
				// Test for GPU first...
				cl_context_properties cprops[] = {
					CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[0])(), 0};
				cl::Context testContext(clType, cprops);

				clType=CL_DEVICE_TYPE_GPU;
				openclString="Any/GPU Preferred";
			}
			catch(cl::Error& err) {
				try {
					// Will take any time.  So whatever's left.
					try {
						cl_context_properties cprops[] = {
							CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[0])(), 0};
						cl::Context testContext(CL_DEVICE_TYPE_CPU, cprops);

						clType=CL_DEVICE_TYPE_CPU;
						openclString="Any/CPU";
					}
					catch (cl::Error& err) {
						cl_context_properties cprops[] = {
							CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[0])(), 0};
						cl::Context testContext(CL_DEVICE_TYPE_ALL, cprops);

						clType=CL_DEVICE_TYPE_ALL;
						openclString="Any/Any";

					}
				}
				catch(cl::Error& err) {
					clType=CL_DEVICE_TYPE_ALL;
					openclString="Any/ALL";
				}
			}
        break;
        }

        contextType = clType;

        // set buffer type for a zero-copy mode
        if (contextType==CL_DEVICE_TYPE_CPU) {
        	optimalBufferType=CL_MEM_USE_HOST_PTR;
        }
        else {
        	optimalBufferType=CL_MEM_ALLOC_HOST_PTR;
        }

        context = NULL;

        // Pick first platform
        for (int i=0;i<platformList.size();i++) {
        	try {
        		// Find the first platform that has devices of the type we want.
                cl_context_properties cprops[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[i])(), 0};
                context= new cl::Context(clType, cprops);
                platformName=platformList[i].getInfo<CL_PLATFORM_NAME>();
                platformVendor=platformList[i].getInfo<CL_PLATFORM_VENDOR>();
                break;
        	}
        	catch (...) {
        		context = NULL;
        	}
        }

        // Use this link as a reference to get other platform info:
        // https://gist.github.com/dogukancagatay/8419284

        if (context == NULL) {
        	std::string errMsg = "No OpenCL devices of type " + openclString + " found.";
        	throw cl::Error(CL_DEVICE_NOT_FOUND,(const char *)errMsg.c_str());
        }
        // Query the set of devices attached to the context
        try {
            devices = context->getInfo<CL_CONTEXT_DEVICES>();
        }
        catch(...) {
        	std::string errMsg = "OpenCL Error: unable to enumerate " + platformName + " devices.";
        	throw cl::Error(CL_DEVICE_NOT_FOUND,(const char *)errMsg.c_str());
        }

        try {
        	deviceNames.push_back(devices[0].getInfo<CL_DEVICE_NAME>());
        }
        catch (...) {
        	std::cout << "GRCLBase: Error getting CL_DEVICE_NAME" << std::endl;
        	exit(0);
        }

        try {
            switch (devices[0].getInfo<CL_DEVICE_TYPE>()) {
            case CL_DEVICE_TYPE_GPU:
            deviceTypes.push_back("GPU");
    		break;
            case CL_DEVICE_TYPE_CPU:
            deviceTypes.push_back("CPU");
    		break;
            case CL_DEVICE_TYPE_ACCELERATOR:
            deviceTypes.push_back("Accelerator");
    		break;
            case CL_DEVICE_TYPE_ALL:
            deviceTypes.push_back("ALL");
    		break;
            case CL_DEVICE_TYPE_CUSTOM:
            deviceTypes.push_back("CUSTOM");
    		break;
            }
        }
        catch (...){
        	std::cout << "GRCLBase: Error getting CL_DEVICE_TYPE" << std::endl;
        	exit(0);
        }

        dataSize=0;

        switch(dataType) {
        case DTYPE_FLOAT:
        	dataSize=sizeof(float);
        break;
        case DTYPE_INT:
        	dataSize=sizeof(int);
        break;
        case DTYPE_COMPLEX:
        	dataSize=sizeof(SComplex);
        break;
        }
    }
    catch(cl::Error& e) {
    	std::cout << "OpenCL Error " + std::to_string(e.err()) + ": " << e.what() << std::endl;
        switch(platformMode) {
        case OCLTYPE_CPU:
        	std::cout << "Attempted Context Type: CPU" << std::endl;
        break;
        case OCLTYPE_ACCELERATOR:
        	std::cout << "Attempted Context Type: Accelerator" << std::endl;
        break;
        case OCLTYPE_GPU:
        	std::cout << "Attempted Context Type: GPU" << std::endl;
        break;
        case OCLTYPE_ANY:
        	std::cout << "Attempted Context Type: ANY" << std::endl;
        break;
        }

    	exit(0);
    }

	try {
		maxConstMemSize = devices[0].getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>();
	}
	catch(cl::Error& e) {
		std::cout << "Error getting device constant memory size." << std::endl;
	}

    // Create command queue
	try {
	    queue = new cl::CommandQueue(*context, devices[0], 0);
	}
	catch(...) {
    	std::string errMsg = "OpenCL Error: Unable to create OpenCL command queue on " + platformName;
    	throw cl::Error(CL_DEVICE_NOT_FOUND,(const char *)errMsg.c_str());
	}
}

cl_device_type GRCLBase::GetContextType() {
	return contextType;
}

void GRCLBase::CompileKernel(const char* kernelCode, const char* kernelFunctionName) {
	try {
		// Create and program from source
		if (program) {
			delete program;
			program = NULL;
		}
		if (sources) {
			delete sources;
			sources = NULL;
		}
		sources=new cl::Program::Sources(1, std::make_pair(kernelCode, 0));
		program = new cl::Program(*context, *sources);

		// Build program
		program->build(devices);

		kernel=new cl::Kernel(*program, (const char *)kernelFunctionName);
	}
	catch(cl::Error& e) {
		std::cout << "OpenCL Error compiling kernel for " << kernelFunctionName << std::endl;
		std::cout << kernelCode << std::endl;
		exit(0);
	}

	try {
		preferredWorkGroupSizeMultiple = kernel->getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(devices[0]);
	}
	catch(cl::Error& e) {
		std::cout << "Error getting kernel preferred work group size multiple" << std::endl;
	}

	try {
		maxWorkGroupSize = kernel->getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0]);
	}
	catch(cl::Error& e) {
		std::cout << "Error getting kernel preferred work group size multiple" << std::endl;
	}
}
void GRCLBase::cleanup() {
	// Cleanup order:
	// Memory
	// Command queue
	// kernel
	// program
	// context

	try {
		if (queue != NULL) {
			delete queue;
			queue = NULL;
		}
	}
	catch(...) {
		queue=NULL;
		std::cout<<"queue delete error." << std::endl;
	}

	try {
		if (kernel != NULL) {
			delete kernel;
			kernel=NULL;
		}
	}
	catch (...) {
		kernel=NULL;
		std::cout<<"Kernel delete error." << std::endl;
	}

	try {
		if (program != NULL) {
			delete program;
			program = NULL;
		}
	}
	catch(...) {
		program = NULL;
		std::cout<<"program delete error." << std::endl;
	}

	try {
		if (context != NULL) {
			delete context;
			context=NULL;
		}
	}
	catch(...) {
		context=NULL;
		std::cout<<"program delete error." << std::endl;
	}
}

GRCLBase::~GRCLBase() {
	// TODO Auto-generated destructor stub
	cleanup();
}

} /* namespace clenabled */
} /* namespace gr */
