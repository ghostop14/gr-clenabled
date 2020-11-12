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
#include <iostream>
#include <fstream>
#include <boost/algorithm/string/replace.hpp>
#include <math.h>  // fabsf
#include <chrono>
#include <ctime>

#include <clenabled/clSComplex.h>
#include <clenabled/clMathOpTypes.h>

#include "clXCorrelate_impl.h"
#include "clxcorrelate_fft_vcf_impl.h"
#include "window.h"

bool verbose=false;
int largeBlockSize=8192;
int opencltype=OCLTYPE_ANY;
int selectorType=OCLDEVICESELECTOR_FIRST;
int platformId=0;
int devId=0;
int d_vlen = 1;
int iterations = 500;
int maxsearch=512;
int num_inputs = 2;
int decimation = 1;
int input_type=DTYPE_FLOAT;
bool fftonly = false;

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

bool testXCorrelate() {
	std::cout << "----------------------------------------------------------" << std::endl;

	int block_as_bytes = largeBlockSize * sizeof(float) * num_inputs; // size in bytes
	int block_as_bits = block_as_bytes * 8;

	std::cout << "Testing time-domain float-input xcorrelate with: " << std::endl <<
			"Float input block size: " << largeBlockSize << " data points." << std::endl <<
			"Number of inputs: " << num_inputs << std::endl <<
			"Total input buffer size: " << block_as_bytes << " bytes" << std::endl;

	int input_size;

	gr::clenabled::clXCorrelate_impl *test=NULL;
	try {
		if (input_type == DTYPE_COMPLEX) {
			input_size = sizeof(gr_complex);
		}
		else {
			input_size=sizeof(float);
		}

		test = new gr::clenabled::clXCorrelate_impl(opencltype,selectorType,platformId,devId,true,
				num_inputs, largeBlockSize, input_type, input_size, maxsearch, decimation, true);
	}
	catch (...) {
		std::cout << "ERROR: error setting up OpenCL environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		return false;
	}

	int i;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds;
	std::vector<int> ninitems;


	float frequency_signal = 10;
	float frequency_sampling = largeBlockSize*frequency_signal;
	float curPhase = 0.0;
	float signal_ang_rate = 2*M_PI*frequency_signal / frequency_sampling;

	std::vector<gr_complex> inputItems_complex;
	std::vector<gr_complex> inputItems_real;
	std::vector<gr_complex> outputItems;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<largeBlockSize;i++) {
		inputItems_complex.push_back(gr_complex(sin(curPhase+(signal_ang_rate*i)),cos(curPhase+(signal_ang_rate*i))));
		inputItems_real.push_back(signal_ang_rate*i);
		outputItems.push_back(grZero);
	}

	for (int i=0;i<num_inputs;i++) {
		if (input_type == DTYPE_COMPLEX) {
			inputPointers.push_back((const void *)&inputItems_complex[0]);
		}
		else {
			inputPointers.push_back((const void *)&inputItems_real[0]);
		}
	}

	for (int i=0;i<num_inputs-1;i++) {
		outputPointers.push_back((void *)&outputItems[0]);
	}

	// Run empty test
	int noutputitems;

	// Get a test run out of the way.
	noutputitems = test->work_test(largeBlockSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_test(largeBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

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

	float elapsed_time;
	float throughput;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	float byte_throughput;
	float bit_throughput;
	byte_throughput = (float)block_as_bytes / elapsed_time;
	bit_throughput = (float)block_as_bits / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " seconds" << std::endl <<
	std::setprecision(2) <<
	"Throughput metrics: " << std::endl <<
	"Total float samples/sec: " << throughput*num_inputs << std::endl <<
	"Synchronized float stream samples/sec: " << throughput << std::endl <<
	"Bytes/sec transferred in/out: " << byte_throughput << std::endl <<
	"Bits/sec transferred in/out: " << bit_throughput << std::endl <<
	std::endl;

	// ----------------------------------------------------------------------
	// Clean up
	if (test != NULL) {
		delete test;
	}

	inputPointers.clear();
	outputPointers.clear();
	inputItems_complex.clear();
	inputItems_real.clear();
	outputItems.clear();
	ninitems.clear();

	return true;
}

bool testFFTXCorrelate() {
	std::cout << "----------------------------------------------------------" << std::endl;

	int block_as_bytes = largeBlockSize * sizeof(gr_complex) * num_inputs; // size in bytes
	int block_as_bits = block_as_bytes * 8;

	std::cout << "Testing Frequency domain complex-input xcorrelate with: " << std::endl <<
			"Complex input block size: " << largeBlockSize << " data points." << std::endl <<
			"Number of inputs: " << num_inputs << std::endl <<
			"Total input buffer size: " << block_as_bytes << " bytes" << std::endl;

	int input_size;

	float p2 = log2(largeBlockSize);

	int new_largeBlockSize = (int)pow(2,ceil(p2));
	if (new_largeBlockSize != largeBlockSize) {
		std::cout << "Adjusting large block size to " << new_largeBlockSize << " for power-of-2 boundary" << std::endl;
		largeBlockSize = new_largeBlockSize;
	}

	gr::clenabled::clxcorrelate_fft_vcf_impl *test=NULL;
	try {
		test = new gr::clenabled::clxcorrelate_fft_vcf_impl(largeBlockSize,num_inputs, opencltype,selectorType,platformId,devId);
	}
	catch (...) {
		std::cout << "ERROR: error setting up environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		return false;
	}

	int i;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	float frequency_signal = 10;
	float frequency_sampling = largeBlockSize*frequency_signal;
	float curPhase = 0.0;
	float signal_ang_rate = 2*M_PI*frequency_signal / frequency_sampling;

	std::vector<gr_complex> inputItems_complex;
	std::vector<gr_complex> inputItems_real;
	std::vector<gr_complex> outputItems;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<largeBlockSize;i++) {
		inputItems_complex.push_back(gr_complex(sin(curPhase+(signal_ang_rate*i)),cos(curPhase+(signal_ang_rate*i))));
		inputItems_real.push_back(signal_ang_rate*i);
		outputItems.push_back(grZero);
	}

	for (int i=0;i<num_inputs;i++) {
		inputPointers.push_back((const void *)&inputItems_complex[0]);
	}

	for (int i=0;i<num_inputs-1;i++) {
		outputPointers.push_back((void *)&outputItems[0]);
	}

	// Run empty test
	int noutputitems;

	// Get a test run out of the way.
	// Note: working with vectors so work length = 1.
	noutputitems = test->work_test(1,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_test(1,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	float elapsed_time,throughput;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	float byte_throughput;
	float bit_throughput;
	byte_throughput = (float)block_as_bytes / elapsed_time;
	bit_throughput = (float)block_as_bits / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " seconds" << std::endl <<
	std::setprecision(2) <<
	"Throughput metrics: " << std::endl <<
	"Total complex Samples/sec: " << throughput*num_inputs << std::endl <<
	"Synchronized complex stream samples/sec: " << throughput << std::endl <<
	"Bytes/sec transferred in/out: " << byte_throughput << std::endl <<
	"Bits/sec transferred in/out: " << bit_throughput << std::endl <<
	std::endl;

	// ----------------------------------------------------------------------
	// Clean up
	if (test != NULL) {
		delete test;
	}

	inputPointers.clear();
	outputPointers.clear();
	inputItems_complex.clear();
	inputItems_real.clear();
	outputItems.clear();
	ninitems.clear();

	return true;
}

int
main (int argc, char **argv)
{
	// Add comma's to numbers
	std::locale comma_locale(std::locale(), new comma_numpunct());

	// tell cout to use our new locale.
	std::cout.imbue(comma_locale);

	if (argc > 1) {
		// 1 is the file name
		if (strcmp(argv[1],"--help")==0) {
			std::cout << std::endl;
//			std::cout << "Usage: [<test buffer size>] [--gpu] [--cpu] [--accel] [--any]" << std::endl;
			std::cout << "Usage: [--gpu] [--cpu] [--accel] [--any] [--device=<platformid>:<device id>] [--fftonly] [--input_complex] [--num_inputs=<num inputs>] [--maxsearch=<search_depth>] [input buffer/vector size (default is 8192)]" << std::endl;
			std::cout << "where: --gpu, --cpu, --accel[erator], or any defines the type of OpenCL device opened." << std::endl;
			std::cout << "If not specified, maxsearch for time-domain will default to 512." << std::endl;
			std::cout << "--input_complex will switch the time-domain test from float to complex inputs to the test routine." << std::endl;
			std::cout << std::endl;
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
			else if (param.find("--maxsearch") != std::string::npos) {
				boost::replace_all(param,"--maxsearch=","");
				maxsearch=atoi(param.c_str());
			}
			else if (param.find("--num_inputs") != std::string::npos) {
				boost::replace_all(param,"--num_inputs=","");
				num_inputs=atoi(param.c_str());
			}
			else if (strcmp(argv[i],"--input_complex")==0) {
				input_type = DTYPE_COMPLEX;
			}
			else if (strcmp(argv[i],"--fftonly")==0) {
				fftonly=true;
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
	bool was_successful;

	if (!fftonly)
		was_successful = testXCorrelate();

	was_successful = testFFTXCorrelate();
	std::cout << std::endl;

	return 0;
}

