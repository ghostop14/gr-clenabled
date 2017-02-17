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
#include <fstream>

#include "clSComplex.h"
#include "clMathConst_impl.h"
#include "clMathOp_impl.h"
#include "clFilter_impl.h"
#include "clMathOpTypes.h"
#include "firdes.h"
#include <iostream>
#include <chrono>
#include <ctime>

bool verbose=true;
int largeBlockSize=8192;
int opencltype=OCLTYPE_ANY;

bool test_complex() {
	std::cout << "Testing complex number structure alignment..." << std::endl;

	size_t size_SComplex = sizeof(SComplex);
	size_t size_gr_complex = sizeof(gr_complex);

	std::cout << "SComplex struct size: " << sizeof(SComplex) << std::endl;
	std::cout << "gr_complex size: " << sizeof(gr_complex) << std::endl;

	if (size_SComplex == size_gr_complex) {
		std::cout << "OK." << std::endl;
	}
	else {
		std::cout << "ERROR: Sizes don't match." << std::endl;
		return false;
	}

	std::cout << "Testing structure copy alignment..." << std::endl;

	gr_complex newcomplex(1,2);
	SComplex *complexStruct;
	complexStruct=(SComplex *)&newcomplex;

	std::cout << "Real: gr_complex=" << newcomplex.real() << " struct=" << complexStruct->real << std::endl;
	std::cout << "Imag: gr_complex=" << newcomplex.imag() << " struct=" << complexStruct->imag << std::endl;

	if ( (newcomplex.real() != complexStruct->real) ||
		 (newcomplex.imag() != complexStruct->imag) ) {
		std::cout << "ERROR: structure mapping mismatch." << std::endl;
		return false;
	}
	else {
		std::cout << "OK." << std::endl;
	}

	return true;
}

int testFn(int noutput_items,
        gr_vector_int &ninput_items,
        gr_vector_const_void_star &input_items)
{
	return 0;
}

bool testMultiplyConst() {
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing no-action kernel (return only) constant operation to measure OpenCL overhead" << std::endl;
	std::cout << "This value represent the 'floor' on the selected platform.  Any CPU operations have to be slower than this to even be worthy of OpenCL consideration unless you're just looking to offload." << std::endl;

	gr::clenabled::clMathConst_impl *test=NULL;
	try {
		test = new gr::clenabled::clMathConst_impl(DTYPE_COMPLEX,sizeof(SComplex),opencltype,2.0,MATHOP_EMPTY,true);
	}
	catch (...) {
		std::cout << "ERROR: error setting up OpenCL environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		return false;
	}

	test->setBufferLength(largeBlockSize);

	int i;
	int numItems=10;
	std::chrono::time_point<std::chrono::system_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	int iterations=100;
	std::vector<int> ninitems;


	if (verbose) {
		std::cout << "building test array..." << std::endl;
	}

	std::vector<gr_complex> inputItems;
	std::vector<gr_complex> outputItems;

	std::vector< const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<largeBlockSize;i++) {
		inputItems.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(gr_complex(0.0,0.0));
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);

	// Run empty test
	int noutputitems;

	test->set_k(0);
	// Get a test run out of the way.
	noutputitems = test->testOpenCL(numItems,ninitems,inputPointers,outputPointers);

	start = std::chrono::system_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);
	}
	end = std::chrono::system_clock::now();

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
	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl << std::endl;

	// switch to multiply
	std::cout << "----------------------------------------------------------" << std::endl;
	delete test;

	test = new gr::clenabled::clMathConst_impl(DTYPE_COMPLEX,sizeof(SComplex),opencltype,2.0,MATHOP_MULTIPLY,true);
	test->setBufferLength(largeBlockSize);
	test->set_k(2.0);

	std::cout << "Testing complex Multiply/Add Const performance with " << largeBlockSize << " items..." << std::endl;

	noutputitems = test->testOpenCL(numItems,ninitems,inputPointers,outputPointers);

	if (verbose) {
		std::cout << "Multiplier: 2" << std::endl;
	    std::cout << "Sample Multiply Results: " << std::endl;

		for (i=0;i<numItems;i++) {
			std::cout << "Input [" << i << "]: " << inputItems[i].real() << "+" << inputItems[i].imag() << "j"<< std::endl;
			std::cout << "Output [" << i << "]: " << outputItems[i].real() << "+" << outputItems[i].imag() << "j" << std::endl;
		}

		if ( (outputItems[0].real() != (2.0*inputItems[0].real())) ||
				(outputItems[0].imag() != (2.0*inputItems[0].imag())) )	{
			std::cout << "Error: calculations failed!" << std::endl;

			return false;
		}

		inputItems.clear();
		outputItems.clear();
	}

	start = std::chrono::system_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);
	}
	end = std::chrono::system_clock::now();

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
	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;


	int j;

	start = std::chrono::system_clock::now();
	for (j=0;j<iterations;j++) {
		noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);
	}
	end = std::chrono::system_clock::now();

	elapsed_seconds = end-start;

	std::cout << "CPU-only Run Time: " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	// std::cout << "OK." << std::endl;

	if (test != NULL) {
		delete test;
	}

	inputPointers.clear();
	outputPointers.clear();
	inputItems.clear();
	outputItems.clear();
	ninitems.clear();

	return true;
}

bool testMultiply() {
	std::cout << "----------------------------------------------------------" << std::endl;
	std::cout << "Testing complex operation (add/multiply/complex conj/mult conj) performance with " << largeBlockSize << " items..." << std::endl;

	gr::clenabled::clMathOp_impl *test=NULL;
	try {
		test = new gr::clenabled::clMathOp_impl(DTYPE_COMPLEX,sizeof(SComplex),opencltype,MATHOP_MULTIPLY,true);
	}
	catch (...) {
		std::cout << "ERROR: error setting up OpenCL environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		return false;
	}

	test->setBufferLength(largeBlockSize);

	int i;
	int numItems=10;
	std::vector<gr_complex> inputItems1;
	std::vector<gr_complex> inputItems2;
	std::vector<gr_complex> outputItems;
	std::vector<int> ninitems;
//	ninitems.push_back(numItems);

	std::vector< const void *> inputPointers;
	std::vector<void *> outputPointers;

	int noutputitems;

	for (i=1;i<=largeBlockSize;i++) {
		inputItems1.push_back(gr_complex(1.0f,0.5f));
		inputItems2.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(gr_complex(0.0,0.0));
	}

	inputPointers.push_back((const void *)&inputItems1[0]);
	inputPointers.push_back((const void *)&inputItems2[0]);
	outputPointers.push_back((void *)&outputItems[0]);

	if (verbose) {
		std::cout << "building test arrays..." << std::endl;


		noutputitems = test->testOpenCL(numItems,ninitems, inputPointers,outputPointers);

		std::cout << "Sample Multiply Results: " << std::endl;

		for (i=0;i<numItems;i++) {
			std::cout << "Input 1/2[" << i << "]: " << inputItems1[i].real() << "+" << inputItems1[i].imag() << "j"<< std::endl;
			std::cout << "Output [" << i << "]: " << outputItems[i].real() << "+" << outputItems[i].imag() << "j" << std::endl;
		}

	}

	std::chrono::time_point<std::chrono::system_clock> start, end;


	int iterations=100;

	start = std::chrono::system_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,ninitems, inputPointers,outputPointers);
	}
	end = std::chrono::system_clock::now();

	std::chrono::duration<double> elapsed_seconds = end-start;

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
	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	int j;

	start = std::chrono::system_clock::now();
	for (j=0;j<iterations;j++) {
		noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);
	}
	end = std::chrono::system_clock::now();

	elapsed_seconds = end-start;

	std::cout << "CPU-only Run Time: " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	if (test != NULL) {
		delete test;
	}

	inputPointers.clear();
	outputPointers.clear();
	inputItems1.clear();
	inputItems2.clear();
	outputItems.clear();
	ninitems.clear();

	return true;
}

bool testLowPassFilter() {
	gr::clenabled::clFilter_impl *test=NULL;
	double gain=1.0;
	double samp_rate=10000000;
	double cutoff_freq=100000.0;
	double transition_width = 15000.0;
	try {
		std::cout << "----------------------------------------------------------" << std::endl;
		std::cout << "Testing filter performance with 10 MSPS sample rate and " << largeBlockSize << " items" << std::endl;

		test = new gr::clenabled::clFilter_impl(opencltype,1,
				gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
		std::cout << "Filter parameters: cutoff freq: " << cutoff_freq << " transition width: " << transition_width << " and " << test->taps().size() << " taps..." << std::endl;
	}
	catch (...) {
		std::cout << "ERROR: error setting up filter OpenCL environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		return false;
	}

	int i;

	std::vector<gr_complex> inputItems;
	std::vector<gr_complex> outputItems;

	std::vector< const void *> inputPointers;
	std::vector<void *> outputPointers;

	int noutputitems;

	for (i=1;i<=largeBlockSize;i++) {
		inputItems.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(gr_complex(0.0,0.0));
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);

	std::chrono::time_point<std::chrono::system_clock> start, end;

	int iterations=100;

	start = std::chrono::system_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::system_clock::now();

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

	std::chrono::duration<double> elapsed_seconds = end-start;
	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;
	// Running with narrower filter:
	transition_width = (int)(cutoff_freq*0.1);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);
	std::cout << "Rerunning with Filter parameters: cutoff freq: " << cutoff_freq << " transition width: " << transition_width << " and " << test->taps().size() << " taps..." << std::endl;

	start = std::chrono::system_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::system_clock::now();
	elapsed_seconds = end-start;
	std::cout << "OpenCL Run Time for 10% transition filter:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;
	// Running with narrower filter:
	transition_width = (int)(cutoff_freq*0.05);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);
	std::cout << "Rerunning with Filter parameters: cutoff freq: " << cutoff_freq << " transition width: " << transition_width << " and " << test->taps().size() << " taps..." << std::endl;

	start = std::chrono::system_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::system_clock::now();
	elapsed_seconds = end-start;
	std::cout << "OpenCL Run Time for 5% transition filter:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	if (test->ActiveContextType() == CL_DEVICE_TYPE_CPU) {
		std::cout << std::endl << "Skipping 3% filter test on Open CPU configuration." << std::endl;
	}
	else {
		// Running with narrower filter:
		transition_width = (int)(cutoff_freq*0.03);
		test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
		test->TestNotifyNewFilter(largeBlockSize);
		std::cout << "Rerunning with Filter parameters: cutoff freq: " << cutoff_freq << " transition width: " << transition_width << " and " << test->taps().size() << " taps..." << std::endl;

		start = std::chrono::system_clock::now();
		// make iterations calls to get average.
		for (i=0;i<iterations;i++) {
			noutputitems = test->testOpenCL(largeBlockSize,inputPointers,outputPointers);
		}
		end = std::chrono::system_clock::now();
		elapsed_seconds = end-start;
		std::cout << "OpenCL Run Time for 3% transition filter:   " << std::fixed << std::setw(11)
	    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;
	}

	std::cout << std::endl;
	// CPU
	transition_width = (int)(cutoff_freq*0.15);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);
	start = std::chrono::system_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testCPU(largeBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::system_clock::now();

	elapsed_seconds = end-start;
	std::cout << "CPU-only Run Time for 15% filter: " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;
	// CPU
	transition_width = (int)(cutoff_freq*0.10);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);
	start = std::chrono::system_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testCPU(largeBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::system_clock::now();

	elapsed_seconds = end-start;
	std::cout << "CPU-only Run Time for 10% filter: " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	// CPU
	/*
	 * NOTE: The fftw calls ARE NOT thread safe!  It works within gnuradio but
	 * seems to be seg faulting here.
	 */
	transition_width = (int)(cutoff_freq*0.5);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);
	start = std::chrono::system_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testCPU(largeBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::system_clock::now();

	elapsed_seconds = end-start;
	std::cout << "CPU-only Run Time for 5% filter: " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;


	if (test != NULL) {
		delete test;
	}

	return true;
}

int
main (int argc, char **argv)
{
	if (argc > 1) {
		// 1 is the file name
		if (strcmp(argv[1],"--help")==0) {
			std::cout << std::endl;
			std::cout << "Usage: [<test buffer size>] [--gpu] [--cpu] [--accel] [--any]" << std::endl;
			std::cout << "where gpu, cpu, accel[erator], or any defines the type of OpenCL device opened." << std::endl;
			std::cout << "It is recomended that the size be a multiple of the 'Preferred work group size multiple' visible from the clinfo command." << std::endl;
			std::cout << std::endl;
			exit(0);
		}

		for (int i=1;i<argc;i++) {
			if (strcmp(argv[i],"--gpu")==0) {
				opencltype=OCLTYPE_GPU;
			}
			else if (strcmp(argv[i],"--cpu")==0) {
				opencltype=OCLTYPE_CPU;
			}
			else if (strcmp(argv[i],"--accel")==0) {
				opencltype=OCLTYPE_ACCELERATOR;
			}
			else if (strcmp(argv[i],"--any")==0) {
				opencltype=OCLTYPE_ANY;
			}
			else {
				int newVal=atoi(argv[i]);

				if (newVal > 0) {
					largeBlockSize=newVal;
					std::cout << "Running with user-defined test buffer size of " << largeBlockSize << std::endl;
				}
			}
		}
	}
	bool was_successful;
/*
	CppUnit::TextTestRunner runner;
	std::ofstream xmlfile(get_unittest_path("clenabled.xml").c_str());
	CppUnit::XmlOutputter *xmlout = new CppUnit::XmlOutputter(&runner.result(), xmlfile);

	runner.addTest(qa_clenabled::suite());
	runner.setOutputter(xmlout);

	bool was_successful = runner.run("", false);  was_successful = testMultiply();
*/

	was_successful = test_complex();
	std::cout << std::endl;

	was_successful = testMultiplyConst();
	std::cout << std::endl;
	was_successful = testMultiply();
	std::cout << std::endl;

	try {
	was_successful = testLowPassFilter();
	}
	catch(...) {

	}
	std::cout << std::endl;


	return was_successful ? 0 : 1;
}

