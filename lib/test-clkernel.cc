/*
 * test-clkernel.cc
 *
 *  Created on: Mar 19, 2017
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <string.h>
#include <chrono>
#include <ctime>
#include <boost/algorithm/string/replace.hpp>

#include "clKernel1To1_impl.h"
#include "clKernel2To1_impl.h"

bool verbose=false;
int largeBlockSize=8192;
int opencltype=OCLTYPE_ANY;
int selectorType=OCLDEVICESELECTOR_FIRST;
int platformId=0;
int devId=0;
int d_vlen = 1;
int iterations = 100;
int inputStreams=0;
std::string inputFile = "";
std::string fnName = "";
int idataType = -1;

bool testkernel2to1() {
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing custom kernel 2 input streams to 1 output stream with " << largeBlockSize << " data points" << std::endl;

	int dsize=sizeof(float);

    switch(idataType) {
    case DTYPE_FLOAT:
  	  dsize=sizeof(float);
    break;
    case DTYPE_INT:
  	  dsize=sizeof(int);
    break;
    case DTYPE_COMPLEX:
  	  dsize=sizeof(gr_complex);
    break;
    }

	gr::clenabled::clKernel2To1_impl *test=NULL;
	try {
		test = new gr::clenabled::clKernel2To1_impl(idataType, dsize,opencltype,selectorType,platformId,
				devId,fnName.c_str(), inputFile.c_str(),true);
	}
	catch (...) {
		std::cout << "ERROR: error setting up OpenCL environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		return false;
	}

	std::cout << "Kernel Function Name: " << test->fnName << std::endl;
	std::cout << std::endl;
	std::cout << "Kernel: " << std::endl;
	std::cout << test->srcStdStr << std::endl;
	std::cout << std::endl;

	test->setBufferLength(largeBlockSize);

	int i;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	if (verbose) {
		std::cout << "building test array..." << std::endl;
	}

	std::vector<float> finputItems;
	std::vector<float> finputItems2;
	std::vector<float> foutputItems;
	std::vector<const void *> finputPointers;
	std::vector<void *> foutputPointers;

	std::vector<int> iinputItems;
	std::vector<int> iinputItems2;
	std::vector<int> ioutputItems;
	std::vector<const void *> iinputPointers;
	std::vector<void *> ioutputPointers;

	std::vector<gr_complex> cinputItems;
	std::vector<gr_complex> cinputItems2;
	std::vector<gr_complex> coutputItems;
	std::vector<const void *> cinputPointers;
	std::vector<void *> coutputPointers;

	int noutputitems;

    switch(idataType) {
    case DTYPE_FLOAT:
    	for (i=0;i<largeBlockSize;i++) {
    		finputItems.push_back(1.0f);
    		finputItems2.push_back(2.0f);
    		foutputItems.push_back(0.0f);
    	}

    	finputPointers.push_back((const void *)&finputItems[0]);
    	finputPointers.push_back((const void *)&finputItems2[0]);
    	foutputPointers.push_back((void *)&foutputItems[0]);

    	// Get a test run out of the way.
    	noutputitems = test->processOpenCL(largeBlockSize,ninitems,finputPointers,foutputPointers);

    	start = std::chrono::steady_clock::now();
    	// make iterations calls to get average.
    	for (i=0;i<iterations;i++) {
    		noutputitems = test->processOpenCL(largeBlockSize,ninitems,finputPointers,foutputPointers);
    	}
    	end = std::chrono::steady_clock::now();
    break;
    case DTYPE_INT:
    	for (i=0;i<largeBlockSize;i++) {
    		iinputItems.push_back(1);
    		iinputItems2.push_back(1);
    		ioutputItems.push_back(0);
    	}

    	iinputPointers.push_back((const void *)&iinputItems[0]);
    	iinputPointers.push_back((const void *)&iinputItems2[0]);
    	ioutputPointers.push_back((void *)&ioutputItems[0]);
    	ioutputPointers.push_back((void *)&ioutputItems[0]);

    	// Get a test run out of the way.
    	noutputitems = test->processOpenCL(largeBlockSize,ninitems,iinputPointers,ioutputPointers);

    	start = std::chrono::steady_clock::now();
    	// make iterations calls to get average.
    	for (i=0;i<iterations;i++) {
    		noutputitems = test->processOpenCL(largeBlockSize,ninitems,iinputPointers,ioutputPointers);
    	}
    	end = std::chrono::steady_clock::now();
    break;
    case DTYPE_COMPLEX:
    	for (i=0;i<largeBlockSize;i++) {
    		cinputItems.push_back(gr_complex(1.0f,0.5f));
    		cinputItems2.push_back(gr_complex(1.0f,0.5f));
    		coutputItems.push_back(gr_complex(1.0f,0.5f));
    	}

    	cinputPointers.push_back((const void *)&cinputItems[0]);
    	cinputPointers.push_back((const void *)&cinputItems2[0]);
    	coutputPointers.push_back((void *)&coutputItems[0]);

    	// Get a test run out of the way.
    	noutputitems = test->processOpenCL(largeBlockSize,ninitems,cinputPointers,coutputPointers);

    	start = std::chrono::steady_clock::now();
    	// make iterations calls to get average.
    	for (i=0;i<iterations;i++) {
    		noutputitems = test->processOpenCL(largeBlockSize,ninitems,cinputPointers,coutputPointers);
    	}
    	end = std::chrono::steady_clock::now();
    break;
    }

	elapsed_seconds = end-start;

	switch(test->GetContextType()) {
	case CL_DEVICE_TYPE_GPU:
		std::cout << "OpenCL Context: GPU" << std::endl;
	break;
	case CL_DEVICE_TYPE_CPU:
		std::cout << "OpenCL Context: CPU" << std::endl;
	break;
	case CL_DEVICE_TYPE_ACCELERATOR:
		std::cout << "OpenCL Context: Accelerator" << std::endl;
	break;
	case CL_DEVICE_TYPE_ALL:
		std::cout << "OpenCL Context: ALL" << std::endl;
	break;
	}

	float elapsed_time,throughput;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	std::cout << std::endl;

	// ----------------------------------------------------------------------
	// Clean up
	if (test != NULL) {
		delete test;
	}

	return true;
}

bool testkernel1to1() {
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing custom kernel 1 input stream to 1 output stream with " << largeBlockSize << " data points" << std::endl;

	int dsize=sizeof(float);

    switch(idataType) {
    case DTYPE_FLOAT:
  	  dsize=sizeof(float);
    break;
    case DTYPE_INT:
  	  dsize=sizeof(int);
    break;
    case DTYPE_COMPLEX:
  	  dsize=sizeof(gr_complex);
    break;
    }

	gr::clenabled::clKernel1To1_impl *test=NULL;
	try {
		test = new gr::clenabled::clKernel1To1_impl(idataType, dsize,opencltype,selectorType,platformId,
				devId,fnName.c_str(), inputFile.c_str(),true);
	}
	catch (...) {
		std::cout << "ERROR: error setting up OpenCL environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		return false;
	}

	std::cout << "Kernel Function Name: " << test->fnName << std::endl;
	std::cout << std::endl;
	std::cout << "Kernel: " << std::endl;
	std::cout << test->srcStdStr << std::endl;
	std::cout << std::endl;

	test->setBufferLength(largeBlockSize);

	int i;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	if (verbose) {
		std::cout << "building test array..." << std::endl;
	}

	std::vector<float> finputItems;
	std::vector<float> finputItems2;
	std::vector<float> foutputItems;
	std::vector<const void *> finputPointers;
	std::vector<void *> foutputPointers;

	std::vector<int> iinputItems;
	std::vector<int> iinputItems2;
	std::vector<int> ioutputItems;
	std::vector<const void *> iinputPointers;
	std::vector<void *> ioutputPointers;

	std::vector<gr_complex> cinputItems;
	std::vector<gr_complex> cinputItems2;
	std::vector<gr_complex> coutputItems;
	std::vector<const void *> cinputPointers;
	std::vector<void *> coutputPointers;

	int noutputitems;


    switch(idataType) {
    case DTYPE_FLOAT:
    	for (i=0;i<largeBlockSize;i++) {
    		finputItems.push_back(1.0f);
    		foutputItems.push_back(0.0f);
    	}

    	finputPointers.push_back((const void *)&finputItems[0]);
    	foutputPointers.push_back((void *)&foutputItems[0]);

    	// Get a test run out of the way.
    	noutputitems = test->processOpenCL(largeBlockSize,ninitems,finputPointers,foutputPointers);

    	start = std::chrono::steady_clock::now();
    	// make iterations calls to get average.
    	for (i=0;i<iterations;i++) {
    		noutputitems = test->processOpenCL(largeBlockSize,ninitems,finputPointers,foutputPointers);
    	}
    	end = std::chrono::steady_clock::now();
    break;
    case DTYPE_INT:
    	for (i=0;i<largeBlockSize;i++) {
    		iinputItems.push_back(1);
    		ioutputItems.push_back(0);
    	}

    	iinputPointers.push_back((const void *)&iinputItems[0]);
    	ioutputPointers.push_back((void *)&ioutputItems[0]);

    	// Get a test run out of the way.
    	noutputitems = test->processOpenCL(largeBlockSize,ninitems,iinputPointers,ioutputPointers);

    	start = std::chrono::steady_clock::now();
    	// make iterations calls to get average.
    	for (i=0;i<iterations;i++) {
    		noutputitems = test->processOpenCL(largeBlockSize,ninitems,iinputPointers,ioutputPointers);
    	}
    	end = std::chrono::steady_clock::now();
    break;
    case DTYPE_COMPLEX:
    	for (i=0;i<largeBlockSize;i++) {
    		cinputItems.push_back(gr_complex(1.0f,0.5f));
    		coutputItems.push_back(gr_complex(1.0f,0.5f));
    	}

    	cinputPointers.push_back((const void *)&cinputItems[0]);
    	coutputPointers.push_back((void *)&coutputItems[0]);

    	// Get a test run out of the way.
    	noutputitems = test->processOpenCL(largeBlockSize,ninitems,cinputPointers,coutputPointers);

    	start = std::chrono::steady_clock::now();
    	// make iterations calls to get average.
    	for (i=0;i<iterations;i++) {
    		noutputitems = test->processOpenCL(largeBlockSize,ninitems,cinputPointers,coutputPointers);
    	}
    	end = std::chrono::steady_clock::now();
    break;
    }

	elapsed_seconds = end-start;

	switch(test->GetContextType()) {
	case CL_DEVICE_TYPE_GPU:
		std::cout << "OpenCL Context: GPU" << std::endl;
	break;
	case CL_DEVICE_TYPE_CPU:
		std::cout << "OpenCL Context: CPU" << std::endl;
	break;
	case CL_DEVICE_TYPE_ACCELERATOR:
		std::cout << "OpenCL Context: Accelerator" << std::endl;
	break;
	case CL_DEVICE_TYPE_ALL:
		std::cout << "OpenCL Context: ALL" << std::endl;
	break;
	}

	float elapsed_time,throughput;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	std::cout << std::endl;

	// ----------------------------------------------------------------------
	// Clean up
	if (test != NULL) {
		delete test;
	}

	return true;
}


void displayHelp() {
	std::cout << std::endl;
	std::cout << "Usage: <[--1to1] [--2to1]> <[--complex] [--float] [--int]> [--gpu] [--cpu] [--accel] [--any] [--device=<platformid>:<device id>] --kernelfile=<file> --fnname=<kernel function name> [number of samples (default is 8192)]" << std::endl;
	std::cout << "Where:" << std::endl;
	std::cout << "	--1to1 says use the 1 input stream to 1 output stream module" << std::endl;
	std::cout << "	--2to1 says use the 2 input streams to 1 output stream module" << std::endl;
	std::cout << "	--complex/float/int defines the data type of the streams (in matches out)" << std::endl;
	std::cout << "	--fnname is the kernel function name to call in the provided kernel file (e.g. what's on the __kernel line" << std::endl;
	std::cout << "	--kernelfile is the file containing a valid OpenCL kernel matching the stream format 2-in/1-out or 1-in/2-out" << std::endl;
	std::cout << "	--gpu, --cpu, --accel[erator], or any defines the type of OpenCL device opened." << std::endl;
	std::cout << "The optional --device argument allows for a specific OpenCL platform and device to be chosen.  Use the included clview utility to get the numbers." << std::endl;
	std::cout << std::endl;
	std::cout << "Example:" <<std::endl;
	std::cout << "test-clkernel --1to1 --complex --kernelfile=kernel1to1_sincos.cl --fnname=fn_sin_cos" << std::endl;
	std::cout << std::endl;
}

int
main (int argc, char **argv)
{
	if (argc > 1) {
		// 1 is the file name
		if (strcmp(argv[1],"--help")==0) {
			displayHelp();
			exit(0);
		}

		for (int i=1;i<argc;i++) {
			std::string param = argv[i];

			if (strcmp(argv[i],"--1to1")==0) {
				inputStreams=1;
			}
			else if (strcmp(argv[i],"--2to1")==0) {
				inputStreams=2;
			}
			else if (strcmp(argv[i],"--complex")==0) {
				idataType=DTYPE_COMPLEX;
			}
			else if (strcmp(argv[i],"--float")==0) {
				idataType=DTYPE_FLOAT;
			}
			else if (strcmp(argv[i],"--int")==0) {
				idataType=DTYPE_INT;
			}
			else if (param.find("--fnname") != std::string::npos) {
				boost::replace_all(param,"--fnname=","");
				fnName=param;
			}
			else if (param.find("--kernelfile") != std::string::npos) {
				boost::replace_all(param,"--kernelfile=","");
				inputFile=param;
			}
			else if (strcmp(argv[i],"--gpu")==0) {
				opencltype=OCLTYPE_GPU;
			}
			else if (strcmp(argv[i],"--cpu")==0) {
				opencltype=OCLTYPE_CPU;
			}
			else if (strcmp(argv[i],"--accel")==0) {
				opencltype=OCLTYPE_ACCELERATOR;
			}
			else if (param.find("--device") != std::string::npos) {
				if (param.find("--device") == std::string::npos) {
					std::cout<< "Error: device format should be <platform id>:<device id>" << std::endl;
					exit(2);
				}

				selectorType = OCLDEVICESELECTOR_SPECIFIC;
				boost::replace_all(param,"--device=","");
				int posColon = param.find(":");
				platformId=atoi(param.substr(0,1).c_str());
				devId=atoi(param.substr(posColon+1,1).c_str());

			}
			else if (strcmp(argv[i],"--any")==0) {
				opencltype=OCLTYPE_ANY;
			}else if (atoi(argv[i]) > 0) {
				int newVal=atoi(argv[i]);

				largeBlockSize=newVal;
				std::cout << "Running with user-defined test buffer size of " << largeBlockSize << std::endl;
			}
			else {
				std::cout << "ERROR: Unknown parameter." << std::endl;
				exit(1);

			}
		}
	}
	else {
		displayHelp();
		exit(1);
	}

	if (inputStreams == 0) {
		std::cout << "ERROR: please specify a stream format (--2to1 or --1to1)" << std::endl;
		exit(1);
	}
	if (idataType == 0) {
		std::cout << "ERROR: please specify a stream data type (--complex, --float, or --in)" << std::endl;
		exit(1);
	}

	if (inputFile.length()==0) {
		std::cout << "ERROR: please specify a kernel file." << std::endl;
		exit(1);
	}

	if (fnName.length()==0) {
		std::cout << "ERROR: please specify a kernel function name." << std::endl;
		exit(1);
	}

	std::cout << "Running function " << fnName << std::endl;
	std::cout <<"Kernel file: " << inputFile << std::endl;

	if (inputStreams == 1)
		testkernel1to1();
	else
		testkernel2to1();

	return 0;
}

