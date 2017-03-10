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

void GRCLBase::InitOpenCL(int idataType, size_t dsize,int openCLPlatformType, int devSelector,int platformId, int devId, bool setDebug) {

	debugMode=setDebug;

	dataType=idataType;
	platformMode=openCLPlatformType;

    int devIndex = 0;

    std::vector<cl::Platform> platformList;

    // Get the list of all platforms in the system
    try {
        cl::Platform::get(&platformList);
    }
    catch(...) {
    	std::string errMsg = "OpenCL Error: Unable to get platform list.";
    	throw cl::Error(CL_DEVICE_NOT_FOUND,(const char *)errMsg.c_str());
    }

	// Now we set up our OpenCL space
    try {
    	// Set default to GPU
        std::string openclString="GPU";
        cl_device_type clType=CL_DEVICE_TYPE_GPU;

        // See if we have to adjust anything.
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
				// Any means any so we're really taking first available.

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

        // So here we have a list of platforms and if we've selected ANY we know if we have GPU or CPU in platform[0]

        // Given the type we want now we have to make some choices.
        // If we asked for a specific platform and device Id we set that up
        // else:
        // We first need to find a platform that has devices of the type we asked for
        // If we asked for the first device, we need to take just 1 device from its list of type-specific devices
        // If we asked for ALL then we leave the list as is.

        if (devSelector==OCLDEVICESELECTOR_SPECIFIC) {
        	// Let's make sure that device number is present.

        	if ((platformId + 1) > platformList.size()) {
            	std::string errMsg = "The requested platform Id " + std::to_string(platformId) + " does not exist.";
            	throw cl::Error(CL_DEVICE_NOT_FOUND,(const char *)errMsg.c_str());
        	}

            cl_context_properties cprops[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[platformId])(), 0};
            try {
                context= new cl::Context(clType, cprops);
            }
            catch(...){
            	std::string errMsg = "The requested platform Id " + std::to_string(platformId) + " does not have a device of the requested type (" + openclString + ")";
            	throw cl::Error(CL_DEVICE_NOT_FOUND,(const char *)errMsg.c_str());
            }

            platformName=platformList[platformId].getInfo<CL_PLATFORM_NAME>();
            platformVendor=platformList[platformId].getInfo<CL_PLATFORM_VENDOR>();
        }
        else {
        	// Find the first platform that has the requested device type.
        	// Note this is the first platform, not the first device.  So if the system has 2 NVIDIA cards,
        	// The platform would be NVIDIA.  The specific cards would be the devices under that platform.
            for (int i=0;i<platformList.size();i++) {
            	try {
        			try {
        				cl_context_properties cprops[] = {
        					CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[i])(), 0};
                        context= new cl::Context(clType, cprops);
        				// If we didn't throw an exception, we're good here
                        platformName=platformList[i].getInfo<CL_PLATFORM_NAME>();
                        platformVendor=platformList[i].getInfo<CL_PLATFORM_VENDOR>();
                        break;
        			}
        			catch (cl::Error& err) {
        				// doesn't have this type.  Continue
        			}
            	}
            	catch (...) {
            		context = NULL;
            	}
            }

        }

        if (context == NULL) {
        	std::string errMsg = "No OpenCL devices of type " + openclString;
        	throw cl::Error(CL_DEVICE_NOT_FOUND,(const char *)errMsg.c_str());
        }

        // Use this link as a reference to get other platform info:
        // https://gist.github.com/dogukancagatay/8419284

        // Query the set of devices attached to the context
        try {
            devices = context->getInfo<CL_CONTEXT_DEVICES>();
        }
        catch(...) {
        	std::string errMsg = "OpenCL Error: unable to enumerate " + platformName + " devices.";
        	throw cl::Error(CL_DEVICE_NOT_FOUND,(const char *)errMsg.c_str());
        }

        // Okay so we have a platform selected with devices of the type we requested, or a specific context
        // Now we have to decide if we need a specific device, the first device, or ALL devices.

        if (devSelector==OCLDEVICESELECTOR_SPECIFIC) {
        	if ((devId + 1) > devices.size()) {
            	std::string errMsg = "The requested device Id " + std::to_string(devId) + " does not exist.";
            	throw cl::Error(CL_DEVICE_NOT_FOUND,(const char *)errMsg.c_str());
        	}

        	devIndex = devId;
        }

        try {
        	deviceNames.push_back(devices[devIndex].getInfo<CL_DEVICE_NAME>());
        }
        catch (...) {
        	std::string errMsg = "Error getting device info for " + openclString;
        	throw cl::Error(CL_DEVICE_NOT_FOUND,(const char *)errMsg.c_str());
        }

        try {
            switch (devices[devIndex].getInfo<CL_DEVICE_TYPE>()) {
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
		maxConstMemSize = devices[devIndex].getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>();
	}
	catch(cl::Error& e) {
		std::cout << "Error getting device constant memory size." << std::endl;
	}

	// This can be overridden in derived classes.
	// This calculation assumes only a single input stream.
	maxConstItems = maxConstMemSize / dataSize;

	// see https://software.intel.com/sites/default/files/managed/9d/6d/TutorialSVMBasic.pdf on SVM

	cl_device_svm_capabilities caps;
	cl_int err;

	err = clGetDeviceInfo(devices[devIndex](),CL_DEVICE_SVM_CAPABILITIES,
							sizeof(cl_device_svm_capabilities),&caps,0);
	hasSharedVirtualMemory = (err == CL_SUCCESS);
	hasSVMFineGrained = (err == CL_SUCCESS && (caps & CL_DEVICE_SVM_FINE_GRAIN_BUFFER));

	/*
	if (debugMode) {
		if (hasSharedVirtualMemory) {
			if (hasSVMFineGrained) {
				std::cout <<"OpenCL Info: " << platformName << " supports fine-grained shared virtual memory (SVM)" << std::endl;
			}
			else {
				std::cout <<"OpenCL Info: " << platformName << " supports shared virtual memory (SVM) but only coarse-grained." << std::endl;
			}
		}
		else {
			std::cout <<"OpenCL Info: " << platformName << " does not support shared virtual memory (SVM)" << std::endl;
		}
	}
	*/
    // Create command queue
	try {
		// devIndex will either be 0 or if a specific platform and device id were specified, that one.
	    queue = new cl::CommandQueue(*context, devices[devIndex], 0);
	}
	catch(...) {
    	std::string errMsg = "OpenCL Error: Unable to create OpenCL command queue on " + platformName;
    	throw cl::Error(CL_DEVICE_NOT_FOUND,(const char *)errMsg.c_str());
	}
}

GRCLBase::GRCLBase(int idataType, size_t dsize,int openCLPlatformType, int devSelector,int platformId, int devId, bool setDebug) {
	InitOpenCL(idataType,dsize,openCLPlatformType,devSelector,platformId,devId,setDebug);
}

GRCLBase::GRCLBase(int idataType, size_t dsize,int openCLPlatformType, bool setDebug) {
	InitOpenCL(idataType,dsize,openCLPlatformType,OCLDEVICESELECTOR_FIRST,0,0,setDebug);
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
