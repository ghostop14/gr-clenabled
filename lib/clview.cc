
// Use CPP exception handling.
// Note: If you include cl.hpp, the compiler just won't find cl::Error class.
// You have to use cl2.hpp to get it to go away
#define __CL_ENABLE_EXCEPTIONS
// Disable the deprecated functions warning.  If you want to keep support for 1.2 devices
// You need to use the deprecated functions.  This #define makes the warning go away.
// #define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_VERSION_1_2

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/opencl.hpp>
#endif

#include <iostream>
#include <boost/algorithm/string.hpp>

int
main (int argc, char **argv)
{
    // opencl variables
    cl::Context *context=NULL;
    std::vector<cl::Device> devices;

    cl_device_type contextType;

    std::string platformName="";
    std::string platformVendor="";
    std::string deviceName;
    std::string deviceType;


    std::vector<cl::Platform> platformList;

    // Pick platform
    try {
        cl::Platform::get(&platformList);
    }
    catch(...) {
    	std::cout << "OpenCL Error: Unable to get platform list." << std::endl;
    	return 1;
    }

    context = NULL;

	cl_context_properties cprops[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[0])(), 0};

    std::string kernelCode="";
    kernelCode += "kernel void test(__constant int * a, const int multiplier, __global int * restrict c) {\n";
    kernelCode += "return;\n";
    kernelCode += "}\n";

	std::vector<cl::Device> curDev;

	// Pick first platform
    for (int i=0;i<platformList.size();i++) {
    	try {
    		// Find the first platform that has devices of the type we want.
            cl_context_properties cprops[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[i])(), 0};
            platformName=platformList[i].getInfo<CL_PLATFORM_NAME>();
            platformVendor=platformList[i].getInfo<CL_PLATFORM_VENDOR>();
            context= new cl::Context(CL_DEVICE_TYPE_ALL, cprops);

            devices = context->getInfo<CL_CONTEXT_DEVICES>();

            for (int j=0;j<devices.size();j++) {
            	deviceName = devices[j].getInfo<CL_DEVICE_NAME>();

            	boost::trim(deviceName);

                switch (devices[j].getInfo<CL_DEVICE_TYPE>()) {
                case CL_DEVICE_TYPE_GPU:
                deviceType = "GPU";
        		break;
                case CL_DEVICE_TYPE_CPU:
                deviceType = "CPU";
        		break;
                case CL_DEVICE_TYPE_ACCELERATOR:
                deviceType = "Accelerator";
        		break;
                case CL_DEVICE_TYPE_ALL:
                deviceType = "ALL";
        		break;
                case CL_DEVICE_TYPE_CUSTOM:
                deviceType = "CUSTOM";
        		break;
                }

            	cl_int err;
            	cl_device_svm_capabilities caps;

            	err = clGetDeviceInfo(devices[j](),CL_DEVICE_SVM_CAPABILITIES,
            							sizeof(cl_device_svm_capabilities),&caps,0);
            	bool hasSharedVirtualMemory = (err == CL_SUCCESS);
            	bool hasSVMFineGrained = (err == CL_SUCCESS && (caps & CL_DEVICE_SVM_FINE_GRAIN_BUFFER));
        		cl_int maxConstMemSize = devices[j].getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>();
        		cl_int localMemSize = devices[j].getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();
        		cl_int memInK=(int)((float)maxConstMemSize / 1024.0);
        		cl_int localMemInK=(int)((float)localMemSize / 1024.0);
        		cl_uint computeUnits = devices[j].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
        		size_t wgSize = devices[j].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();

            	std::string Ocl2Caps="[OpenCL 2.0 Capabilities]";

            	std::cout << "Platform Id: " << i << std::endl;
            	std::cout << "Device Id: " << j << std::endl;
            	std::cout << "Platform Name: " << platformName << std::endl;
            	std::cout << "Device Name: " << deviceName << std::endl;
            	std::cout << "Device Type: " << deviceType << std::endl;
            	try {
            		// Only available from 2.0 on.
            		cl_uint freq = devices[j].getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>();
                	std::cout << "Clock Frequency: " << freq << " MHz" << std::endl;
            	}
            	catch(...) {

            	}
            	std::cout << "Compute Units: " << computeUnits << " (A workgroup executes on a compute unit.  This represents parallel workgroups.)" << std::endl;
            	std::cout << "Max Workgroup Size: " << wgSize << std::endl;
            	std::cout << "Constant Memory: " << memInK << "K (" << (maxConstMemSize/4) << " floats)" << std::endl;
            	std::cout << "Local Memory: " << localMemInK << "K (" << (localMemSize/4) << " floats)" << std::endl;


            	// ---- now check if we support double precision
            	size_t retSize;
            	bool hasDoublePrecisionSupport = false;
            	bool hasDoubleFMASupport = false;
            	err = clGetDeviceInfo(devices[j](),CL_DEVICE_EXTENSIONS, 0, NULL, &retSize);

            	if (err == CL_SUCCESS) {
            		char extensions[retSize];
            		// returns a char[] string of extension names
            		// https://www.khronos.org/registry/OpenCL/sdk/1.0/docs/man/xhtml/clGetDeviceInfo.html
            		err = clGetDeviceInfo(devices[j](),CL_DEVICE_EXTENSIONS, retSize, extensions, &retSize);

            		std::string dev_extensions(extensions);
            		std::string search_string = "cl_khr_fp64";

            		if (dev_extensions.find(search_string) != std::string::npos) {
            			hasDoublePrecisionSupport = true;

            			// Query if we support FMA in double
            			err = clGetDeviceInfo(devices[j](),CL_DEVICE_EXTENSIONS, 0, NULL, &retSize);

            			if (err == CL_SUCCESS) {
            				cl_device_fp_config config_properties;

            				err = clGetDeviceInfo(devices[j](),CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(cl_device_fp_config),&config_properties,NULL);

            				if (config_properties & CL_FP_FMA)
            					hasDoubleFMASupport = true;
            			}

            		}
            	}

            	if (hasDoublePrecisionSupport) {
            		std::cout << "Double Precision Math Support: Yes" << std::endl;
                	if (hasDoubleFMASupport) {
                		std::cout << "Double Precision Fused Multiply/Add [FMA] Support: Yes" << std::endl;
                	}
                	else {
                		std::cout << "Double Precision Fused Multiply/Add [FMA] Support: No" << std::endl;
                	}
            	}
            	else {
            		std::cout << "Double Precision Math Support: No  [WARNING THIS WILL NEGATIVELY IMPACT TRIG FUNCTIONS]" << std::endl;
            		std::cout << "Double Precision Fused Multiply/Add [FMA] Support: No" << std::endl;
            	}

            	// ----- now check if we support fused multiply / add
            	bool hasSingleFMASupport = false;

            	err = clGetDeviceInfo(devices[j](),CL_DEVICE_EXTENSIONS, 0, NULL, &retSize);

            	if (err == CL_SUCCESS) {
            		cl_device_fp_config config_properties;

            		err = clGetDeviceInfo(devices[j](),CL_DEVICE_SINGLE_FP_CONFIG, sizeof(cl_device_fp_config),&config_properties,NULL);

            		if (config_properties & CL_FP_FMA)
            			hasSingleFMASupport = true;
            	}

            	if (hasSingleFMASupport) {
            		std::cout << "Single Precision Fused Multiply/Add [FMA] Support: Yes" << std::endl;
            	}
            	else {
            		std::cout << "Single Precision Fused Multiply/Add [FMA] Support: No" << std::endl;
            	}

            	/*
            	// OpenCL 2.0 Capabilities
            	std::cout << "OpenCL 2.0 Capabilities:" << std::endl;
            	if (hasSharedVirtualMemory) {
            		std::cout << "Shared Virtual Memory (SVM): Yes" << std::endl;
            	}
            	else {
            		std::cout << "Shared Virtual Memory (SVM): No" << std::endl;
            	}

            	if (hasSVMFineGrained) {
            		std::cout << "Fine-grained SVM: Yes" << std::endl;
            	}
            	else {
            		std::cout << "Fine-grained SVM: No" << std::endl;
            	}
				*/

            	std::cout << std::endl;
            }

    	}
    	catch (...) {
        	std::cout << "OpenCL Error: Unable to get platform list." << std::endl;
        	return 2;
    	}
    }

    return 0;
}
