/* -*- c++ -*- */
/*
 * Copyright 2012 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cppunit/TextTestRunner.h>
#include <cppunit/XmlOutputter.h>

#include <gnuradio/unittests.h>
#include <gnuradio/block.h>
#include "qa_clenabled.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <boost/algorithm/string/replace.hpp>
#include <math.h>  // fabsf
#include <chrono>
#include <ctime>

#include "clSComplex.h"
#include "clMathOpTypes.h"

#include "clFilter_impl.h"

bool verbose=false;
int largeBlockSize=8192;
int opencltype=OCLTYPE_ANY;
int selectorType=OCLDEVICESELECTOR_FIRST;
int platformId=0;
int devId=0;
int d_vlen = 1;
int iterations = 100;
int ntaps = 0;

// For comma-separated output (#include <iomanip> and the local along with this class takes care of it

class comma_numpunct : public std::numpunct<char>
{
  protected:
    virtual char do_thousands_sep() const
    {
        return ',';
    }

    virtual std::string do_grouping() const
    {
        return "\03";
    }
};

void verifyBuffers(int requiredSize,
				std::vector<gr_complex>& inputItems, std::vector<gr_complex>& outputItems,
				std::vector< const void *>& inputPointers,std::vector<void *>& outputPointers) {

	while (inputItems.size() < requiredSize) {
		inputItems.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(gr_complex(0.0,0.0));
	}

	inputPointers.clear();
	outputPointers.clear();

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);
}

bool testFilter() {
	gr::clenabled::clFilter_impl *test=NULL;
	double gain=1.0;
	int i;

	int nthreads = 4;

	// set up fake filter.  Values don't matter since we're just looking at timing.
	std::vector<float> filtertaps;

	for (i=0;i<ntaps;i++) {
		filtertaps.push_back(((float)i)/1000.0);
	}

	// Add comma's to numbers
	 std::locale comma_locale(std::locale(), new comma_numpunct());

	    // tell cout to use our new locale.
	 std::cout.imbue(comma_locale);

	// -------------------------  OPENCL TIME DOMAIN FILTER -------------------------------------------------
	try {
		test = new gr::clenabled::clFilter_impl(opencltype,selectorType,platformId,devId,1,
				filtertaps,1,false,true);
	}
	catch(const std::runtime_error& re)
	{
	    // specific handling for runtime_error
	    std::cerr << "Runtime error: " << re.what() << std::endl;
	}
	catch(const std::exception& ex)
	{
	    // specific handling for all exceptions extending std::exception, except
	    // std::runtime_error which is handled explicitly
	    std::cerr << "Error occurred: " << ex.what() << std::endl;
	}
	catch (...) {
		std::cout << "ERROR: error setting up filter OpenCL environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		return false;
	}

	std::cout << std::endl;

	switch(test->GetContextType()) {
	case CL_DEVICE_TYPE_GPU:
		std::cout << "OpenCL Context: GPU" << std::endl;
	break;
	case CL_DEVICE_TYPE_ACCELERATOR:
		std::cout << "OpenCL Context: Accelerator" << std::endl;
	break;
	case CL_DEVICE_TYPE_CPU:
		std::cout << "OpenCL Context: CPU" << std::endl;
	break;
	case CL_DEVICE_TYPE_ALL:
		std::cout << "OpenCL Context: ALL" << std::endl;
	break;
	}
	std::cout << "Running on: " << test->getPlatformName() << std::endl;
	std::cout << std::endl;

	std::cout << "Test Type              	Throughput (sps)	Samples Tested" << std::endl;

	int fdBlockSize, tdBufferSize;
	int optimalSize;

	std::vector<gr_complex> inputItems;
	std::vector<gr_complex> outputItems;

	std::vector< const void *> inputPointers;
	std::vector<void *> outputPointers;
	float throughput;

	int noutputitems;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<largeBlockSize;i++) {
		outputItems.push_back(gr_complex(1.0f,0.5f));
		inputItems.push_back(gr_complex(0.0f,0.0f));
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);

	verifyBuffers(largeBlockSize,inputItems,outputItems,inputPointers,outputPointers);
	std::chrono::time_point<std::chrono::steady_clock> start, end;

	fdBlockSize = test->freqDomainSampleBlockSize();
	tdBufferSize = test->getCurrentBufferSize();

	// OpenCL FIR Filter
	verifyBuffers(largeBlockSize,inputItems,outputItems,inputPointers,outputPointers);
	test->setTimeDomainFilterVariables(largeBlockSize);

	noutputitems = test->testOpenCL(largeBlockSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	std::chrono::duration<double> elapsed_seconds = end-start;
	throughput = largeBlockSize / (elapsed_seconds.count()/(float)iterations);
	std::cout << "OpenCL FIR Filter	" << std::fixed << std::setw(11)
    << std::setprecision(2) << throughput << "      	" << std::setprecision(0) << largeBlockSize << std::endl;

	// CPU FIR Filter
	noutputitems = test->testCPUFIR(largeBlockSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testCPUFIR(largeBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;
	throughput = largeBlockSize / (elapsed_seconds.count()/(float)iterations);
	std::cout << "GNURadio FIR Filter	" << std::fixed << std::setw(11)
    << std::setprecision(2) << throughput << "      	" << std::setprecision(0) << largeBlockSize << std::endl;

	// -------------------------  FREQUENCY DOMAIN FILTER -------------------------------------------------
	int fftSize;

	try {
		delete test;

		test = new gr::clenabled::clFilter_impl(opencltype,selectorType,platformId,devId,1,
				filtertaps,1,false,false);
	}
	catch(const std::runtime_error& re)
	{
	    // specific handling for runtime_error
	    std::cerr << "Runtime error: " << re.what() << std::endl;
	}
	catch(const std::exception& ex)
	{
	    // specific handling for all exceptions extending std::exception, except
	    // std::runtime_error which is handled explicitly
	    std::cerr << "Error occurred: " << ex.what() << std::endl;
	}
	catch (...) {
		std::cout << "ERROR: error setting up filter OpenCL environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		return false;
	}

	fdBlockSize = test->freqDomainSampleBlockSize();
	test->TestNotifyNewFilter(fdBlockSize);

	verifyBuffers(fdBlockSize,inputItems,outputItems,inputPointers,outputPointers);
	noutputitems = test->testOpenCL(fdBlockSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(fdBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;
	throughput = fdBlockSize / (elapsed_seconds.count()/(float)iterations);
	std::cout << "OpenCL FFT Filter	" << std::fixed << std::setw(11)
    << std::setprecision(2) << throughput << "      	" << std::setprecision(0) << fdBlockSize << std::endl;

	// ---------------------- CPU TESTS -----------------------------------------
	test->TestNotifyNewFilter(fdBlockSize);
	verifyBuffers(fdBlockSize,inputItems,outputItems,inputPointers,outputPointers);
//	test->setFilterVariables(tdBufferSize);
	noutputitems = test->testCPUFFT(fdBlockSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testCPUFFT(fdBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;
	throughput = fdBlockSize / (elapsed_seconds.count()/(float)iterations);
	std::cout << "GNURadio FFT Filter    	" << std::fixed << std::setw(11)
    << std::setprecision(2) << throughput << "      	" << std::setprecision(0) << fdBlockSize << std::endl;

	if (test != NULL) {
		delete test;
	}

	return true;
}


void printHelp() {
	std::cout << std::endl;
//			std::cout << "Usage: [<test buffer size>] [--gpu] [--cpu] [--accel] [--any]" << std::endl;
	std::cout << "Usage: [--gpu] [--cpu] [--accel] [--any] [--device=<platformid>:<device id>] --ntaps=<# of filter taps> [number of samples (default is 8192)]" << std::endl;
	std::cout << "where: --gpu, --cpu, --accel[erator], or any defines the type of OpenCL device opened." << std::endl;
	std::cout << "The optional --device argument allows for a specific OpenCL platform and device to be chosen.  Use the included clview utility to get the numbers." << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "You can create a filter by hand and see how many taps it would create from an interactive python command-line like this:" << std::endl;
	std::cout << std::endl;
	std::cout << "python" << std::endl;
	std::cout << "from gnuradio.filter import firdes" << std::endl;
	std::cout << "# parameters are gain, sample rate, cutoff freq, transition width for this low_pass filter." << std::endl;
	std::cout << "taps=firdes.low_pass(1, 10e6, 500e3, 0.2*500e3)" << std::endl;
	std::cout << "len(taps)" << std::endl;
	std::cout << std::endl << "For this example 241 taps were created." << std::endl;
	std::cout << std::endl;
}

int
main (int argc, char **argv)
{
	if (argc > 1) {
		// 1 is the file name
		if (strcmp(argv[1],"--help")==0) {
			printHelp();
			exit(0);
		}

		for (int i=1;i<argc;i++) {
			std::string param = argv[i];

			if (strcmp(argv[i],"--gpu")==0) {
				opencltype=OCLTYPE_GPU;
			}
			else if (strcmp(argv[i],"--cpu")==0) {
				opencltype=OCLTYPE_CPU;
			}
			else if (strcmp(argv[i],"--accel")==0) {
				opencltype=OCLTYPE_ACCELERATOR;
			}
			else if (param.find("--ntaps") != std::string::npos) {
				boost::replace_all(param,"--ntaps=","");
				ntaps=atoi(param.c_str());
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

	if (ntaps == 0) {
		std::cout << "ERROR: please specify --ntaps=<# of taps>." << std::endl << std::endl;
		printHelp();
		exit(1);
	}

	bool was_successful;

	was_successful = testFilter();

	std::cout << std::endl;

	return 0;
}

