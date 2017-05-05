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
#include <boost/algorithm/string/replace.hpp>
#include <math.h>  // fabsf

#include "clSComplex.h"
#include "clMathConst_impl.h"
#include "clMathOp_impl.h"
#include "clFilter_impl.h"
#include "clQuadratureDemod_impl.h"
#include "clFFT.h"
#include "clMathOpTypes.h"
#include "firdes.h"
#include "clFFT_impl.h"
#include "clLog_impl.h"
#include "clSNR_impl.h"
#include <chrono>
#include <ctime>
#include "clComplexToMag_impl.h"
#include "clComplexToMagPhase_impl.h"
#include "clComplexToArg_impl.h"
#include "clMagPhaseToComplex_impl.h"
#include "clSignalSource_impl.h"
#include "clCostasLoop_impl.h"

#include "window.h"

bool verbose=false;
int largeBlockSize=8192;
int opencltype=OCLTYPE_ANY;
int selectorType=OCLDEVICESELECTOR_FIRST;
int platformId=0;
int devId=0;
int d_vlen = 1;
int iterations = 100;

int ComplexToMagCPU(int noutput_items,
        gr_vector_int &ninput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
	{
    const gr_complex *in = (const gr_complex *) input_items[0];
    float *out = (float *) output_items[0];
    int noi = noutput_items * d_vlen;

    // turned out to be faster than aligned/unaligned switching
    volk_32fc_magnitude_32f_u(out, in, noi);

    return noutput_items;
}

bool testSigSource() {
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing Complex Signal Source" << std::endl;

	gr::clenabled::clSignalSource_impl *test=NULL;
	try {
		test = new gr::clenabled::clSignalSource_impl(DTYPE_COMPLEX,sizeof(gr_complex),opencltype,selectorType,platformId,devId,2400000.0,SIGSOURCE_COS, 1000.0, 1.0,true);
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
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	std::vector<gr_complex> outputItems;
	std::vector<gr_complex> outputItems2;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;
	std::vector<void *> outputPointers2;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<largeBlockSize;i++) {
		outputItems.push_back(grZero);
		outputItems2.push_back(grZero);
	}

	outputPointers.push_back((void *)&outputItems[0]);
	outputPointers2.push_back((void *)&outputItems2[0]);

	// Run empty test
	int noutputitems;

	// Get a test run out of the way.
	noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);
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
	std::cout << "Running on: " << test->getPlatformName() << std::endl;
	std::cout << std::endl;

	float elapsed_time,throughput;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	int j;

	noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	for (j=0;j<iterations;j++) {
		noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "CPU-only Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl;

	std::cout << std::endl;

	// ----------------------------------------------------------------------
	// Clean up
	if (test != NULL) {
		delete test;
	}

	// Test accuracy:
	gr::clenabled::clSignalSource_impl *test2;
	test = new gr::clenabled::clSignalSource_impl(DTYPE_COMPLEX,sizeof(gr_complex),opencltype,selectorType,platformId,devId,2400000.0,SIGSOURCE_COS, 1000.0, 1.0,true);
	test2 = new gr::clenabled::clSignalSource_impl(DTYPE_COMPLEX,sizeof(gr_complex),opencltype,selectorType,platformId,devId,2400000.0,SIGSOURCE_COS, 1000.0, 1.0,true);
	noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);
	noutputitems = test2->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers2);
	float delta_r,delta_i;
	float max_error_r=0.0;
	float max_error_i=0.0;

	for (i=0;i<largeBlockSize;i++) {
		/*
		if ( fabs(outputItems[i].real() - outputItems2[i].real()) > 0.000001) {
			std::cout << i << ": OpenCL: " << outputItems[i].real() << "/" << outputItems[i].imag() << std::endl;
			std::cout << "CPU: " << outputItems2[i].real() << "/" << outputItems2[i].imag() << std::endl;
		}
		*/
		delta_r = fabsf(outputItems[i].real() - outputItems2[i].real());
		delta_i = fabsf(outputItems[i].imag() - outputItems2[i].imag());

		if (delta_r > max_error_r)
			max_error_r = delta_r;

		if (delta_i > max_error_i)
			max_error_i = delta_i;
}

	std::cout << "maximum error OpenCL versus gnuradio table lookup cos/sin: " << max_error_r << "/" << max_error_i << std::endl;

	delete test;
	delete test2;

	outputPointers.clear();
	outputItems.clear();
	ninitems.clear();

	return true;
}

bool testMagPhaseToComplex() {
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing Mag and Phase to Complex" << std::endl;

	gr::clenabled::clMagPhaseToComplex_impl *test=NULL;
	try {
		test = new gr::clenabled::clMagPhaseToComplex_impl(opencltype,selectorType,platformId,devId,true);
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
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	if (verbose) {
		std::cout << "building test array..." << std::endl;
	}

	std::vector<float> inputItems1;
	std::vector<float> inputItems2;
	std::vector<gr_complex> outputItems;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<largeBlockSize;i++) {
		outputItems.push_back(gr_complex(1.0f,0.5f));
		inputItems1.push_back(0.0);
		inputItems2.push_back(0.0);
	}

	inputPointers.push_back((const void *)&inputItems1[0]);
	inputPointers.push_back((const void *)&inputItems2[0]);
	outputPointers.push_back((void *)&outputItems[0]);

	// Run empty test
	int noutputitems;

	// Get a test run out of the way.
	noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);
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

	std::cout << "Running on: " << test->getPlatformName() << std::endl;
	std::cout << std::endl;

	float elapsed_time,throughput;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	int j;

	noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	for (j=0;j<iterations;j++) {
		noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "CPU-only Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	std::cout << std::endl;

	// ----------------------------------------------------------------------
	// Clean up
	if (test != NULL) {
		delete test;
	}

	inputPointers.clear();
	outputPointers.clear();
	outputItems.clear();
	inputItems1.clear();
	inputItems2.clear();
	ninitems.clear();

	return true;
}

bool testComplexToMagPhase() {
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing Complex to Mag and Phase" << std::endl;

	gr::clenabled::clComplexToMagPhase_impl *test=NULL;
	try {
		test = new gr::clenabled::clComplexToMagPhase_impl(opencltype,selectorType,platformId,devId,true);
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
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	if (verbose) {
		std::cout << "building test array..." << std::endl;
	}

	std::vector<gr_complex> inputItems;
	std::vector<float> outputItems1;
	std::vector<float> outputItems2;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<largeBlockSize;i++) {
		inputItems.push_back(gr_complex(1.0f,0.5f));
		outputItems1.push_back(0.0);
		outputItems2.push_back(0.0);
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems1[0]);
	outputPointers.push_back((void *)&outputItems2[0]);

	// Run empty test
	int noutputitems;

	// Get a test run out of the way.
	noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);
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

	float elapsed_time,throughput;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	int j;

	noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	for (j=0;j<iterations;j++) {
		noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "CPU-only Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	std::cout << std::endl;

	// ----------------------------------------------------------------------
	// Clean up
	if (test != NULL) {
		delete test;
	}

	inputPointers.clear();
	outputPointers.clear();
	inputItems.clear();
	outputItems1.clear();
	outputItems2.clear();
	ninitems.clear();

	return true;
}

bool testComplexToArg() {
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing Complex to Arg" << std::endl;

	gr::clenabled::clComplexToArg_impl *test=NULL;
	try {
		test = new gr::clenabled::clComplexToArg_impl(opencltype,selectorType,platformId,devId,true);
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
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	if (verbose) {
		std::cout << "building test array..." << std::endl;
	}

	std::vector<gr_complex> inputItems;
	std::vector<float> outputItems;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<largeBlockSize;i++) {
		inputItems.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(0.0);
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);

	// Run empty test
	int noutputitems;

	// Get a test run out of the way.
	noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);
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
	std::cout << "Running on: " << test->getPlatformName() << std::endl;
	std::cout << std::endl;

	float elapsed_time,throughput;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	int j;

	noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	for (j=0;j<iterations;j++) {
		noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "CPU-only Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	std::cout << std::endl;

	// ----------------------------------------------------------------------
	// Clean up
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

bool testComplexToMag() {
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing Complex to mag" << std::endl;

	gr::clenabled::clComplexToMag_impl *test=NULL;
	try {
		test = new gr::clenabled::clComplexToMag_impl(opencltype,selectorType,platformId,devId,true);
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
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	if (verbose) {
		std::cout << "building test array..." << std::endl;
	}

	std::vector<gr_complex> inputItems;
	std::vector<float> outputItems;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<largeBlockSize;i++) {
		inputItems.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(0.0);
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);

	// Run empty test
	int noutputitems;

	// Get a test run out of the way.
	noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);
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

	float elapsed_time,throughput;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	int j;

	noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	for (j=0;j<iterations;j++) {
		noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "CPU-only Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	std::cout << std::endl;

	// ----------------------------------------------------------------------
	// Clean up
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


bool testFFT(bool runReverse) {
	// Testing/profiling with CLFFT-client:
	// clFFT-client -x 2048 -p 10 -c -o -b 4

	std::cout << "----------------------------------------------------------" << std::endl;

	int fftSize=2048;

	int fftDataSize;

	fftDataSize = (int)((float)largeBlockSize / (float)fftSize) * fftSize;

	if (fftDataSize == 0)
		fftDataSize = fftSize;

	std::cout << "Testing Forward FFT size of " << fftSize << " and " << fftDataSize << " data points." << std::endl;

	gr::clenabled::clFFT_impl *test=NULL;
	try {
		test = new gr::clenabled::clFFT_impl(fftSize,CLFFT_FORWARD,gr::clenabled::window::blackman(fftSize),DTYPE_COMPLEX,sizeof(gr_complex),opencltype,selectorType,platformId,devId,true);
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
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	if (verbose) {
		std::cout << "building test array for FFT..." << std::endl;
	}

	float frequency_signal = 10;
	float frequency_sampling = fftDataSize*frequency_signal;
	float curPhase = 0.0;
	float signal_ang_rate = 2*M_PI*frequency_signal / frequency_sampling;

	std::vector<gr_complex> inputItems;
	std::vector<gr_complex> outputItems;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<fftDataSize;i++) {
		inputItems.push_back(gr_complex(sin(curPhase+(signal_ang_rate*i)),cos(curPhase+(signal_ang_rate*i))));
		outputItems.push_back(grZero);
	}

	std::cout << "First few points of input signal" << std::endl;

	for (i=0;i<4;i++) {
		std::cout << "input[" << i << "]: " << inputItems[i].real() << "," << inputItems[i].imag() << "j" << std::endl;
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);

	// Run empty test
	int noutputitems;

	// Get a test run out of the way.
	noutputitems = test->testOpenCL(fftDataSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(fftDataSize,inputPointers,outputPointers);
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

	std::cout << "Running on: " << test->getPlatformName() << std::endl;
	std::cout << std::endl;
	float elapsed_time,throughput;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	int j;

	noutputitems = test->testCPU(fftDataSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	for (j=0;j<iterations;j++) {
		noutputitems = test->testCPU(fftDataSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "CPU-only Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	std::cout << std::endl;

	if (!runReverse) {
		delete test;
		return true;
	}
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing Reverse FFT" << std::endl;
	delete test;
	test = new gr::clenabled::clFFT_impl(fftSize,CLFFT_BACKWARD,gr::clenabled::window::blackman(fftSize),DTYPE_COMPLEX,sizeof(gr_complex),opencltype,selectorType,platformId,devId,true);

	inputItems.clear();

	// Load previous output items into new input items
	for (i=0;i<fftDataSize;i++) {
		inputItems.push_back(outputItems[i]);
	}

	outputItems.clear();
	// There's a seg fault somewhere.  Give this output buffer more memory.
	for (i=0;i<fftDataSize;i++) {
		outputItems.push_back(grZero);
	}

	inputPointers.clear();
	outputPointers.clear();
	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);

	// Get a test run out of the way.
	noutputitems = test->testOpenCL(fftDataSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(fftDataSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	std::cout << "First few points of FWD->Rev FFT" << std::endl;

	for (i=0;i<4;i++) {
		std::cout << "output[" << i << "]: " << outputItems[i].real() << "," << outputItems[i].imag() << "j" << std::endl;
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

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	noutputitems = test->testCPU(fftDataSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	for (j=0;j<iterations;j++) {
		noutputitems = test->testCPU(fftDataSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "CPU-only Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	std::cout << std::endl;
	// ----------------------------------------------------------------------
	// Clean up
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

bool testQuadDemod() {
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing Quadrature Demodulation (used for FSK)" << std::endl;

	gr::clenabled::clQuadratureDemod_impl *test=NULL;
	try {
		test = new gr::clenabled::clQuadratureDemod_impl(2.0,opencltype,selectorType,platformId,devId,true);
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
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	if (verbose) {
		std::cout << "building test array..." << std::endl;
	}

	std::vector<gr_complex> inputItems;
	std::vector<float> outputItems;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<largeBlockSize;i++) {
		inputItems.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(0.0);
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);

	// Run empty test
	int noutputitems;

	// Get a test run out of the way.
	noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);
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
	std::cout << "Running on: " << test->getPlatformName() << std::endl;
	std::cout << std::endl;

	float elapsed_time,throughput;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	int j;

	noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	for (j=0;j<iterations;j++) {
		noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "CPU-only Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	std::cout << std::endl;

	// ----------------------------------------------------------------------
	// Clean up
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

bool testMultiplyConst() {
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing no-action kernel (return only) constant operation to measure OpenCL overhead" << std::endl;
	std::cout << "This value represent the 'floor' on the selected platform.  Any CPU operations have to be slower than this to even be worthy of OpenCL consideration unless you're just looking to offload." << std::endl;
	gr::clenabled::clMathConst_impl *test=NULL;
	try {
		test = new gr::clenabled::clMathConst_impl(DTYPE_COMPLEX,sizeof(SComplex),opencltype,selectorType,platformId,devId,2.0,MATHOP_EMPTY,true);
	}
	catch (...) {
		std::cout << "ERROR: error setting up OpenCL environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		exit(1);  // Something went wrong with the OpenCL environment, device not found, etc.

		return false;
	}

	std::cout << "Max constant items: " << test->MaxConstItems() << std::endl;
	test->setBufferLength(largeBlockSize);

	int i;
	int numItems=10;
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::vector<int> ninitems;


	if (verbose) {
		std::cout << "building test array..." << std::endl;
	}

	std::vector<gr_complex> inputItems;
	std::vector<gr_complex> outputItems;
	std::vector<float> inputFloats;
	std::vector<float> outputFloats;
	std::vector<const void *> inputFloatPointers;
	std::vector<void *> outputFloatPointers;

	std::vector< const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<largeBlockSize;i++) {
		inputItems.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(gr_complex(0.0,0.0));

		inputFloats.push_back((float)i+1.0);
		outputFloats.push_back(0.0);
	}

	inputPointers.push_back((const void *)&inputItems[0]);
	outputPointers.push_back((void *)&outputItems[0]);

	inputFloatPointers.push_back((const void *)&inputFloats[0]);
	outputFloatPointers.push_back((void *)&outputFloats[0]);

	// Run empty test
	int noutputitems;

	test->set_k(0);
	// Get a test run out of the way.
	noutputitems = test->testOpenCL(numItems,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);
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

	std::cout << "Running on: " << test->getPlatformName() << std::endl;
	std::cout << std::endl;
	float elapsed_time;
	float throughput;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	// switch to empty with copy
	std::cout << "----------------------------------------------------------" << std::endl;
	delete test;

	test = new gr::clenabled::clMathConst_impl(DTYPE_COMPLEX,sizeof(SComplex),opencltype,selectorType,platformId,devId,2.0,MATHOP_EMPTY_W_COPY,true);
	std::cout << "Max constant items: " << test->MaxConstItems() << std::endl;
	test->setBufferLength(largeBlockSize);
	test->set_k(2.0);

	std::cout << "Testing kernel that simply copies in[index]->out[index] " << largeBlockSize << " items..." << std::endl;

	noutputitems = test->testOpenCL(numItems,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);
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
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	std::cout << std::endl;

	// switch to multiply
	std::cout << "----------------------------------------------------------" << std::endl;
	delete test;

	test = new gr::clenabled::clMathConst_impl(DTYPE_COMPLEX,sizeof(SComplex),opencltype,selectorType,platformId,devId,2.0,MATHOP_MULTIPLY,true);
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

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,ninitems,inputPointers,outputPointers);
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
	std::cout << "Running on: " << test->getPlatformName() << std::endl;
	std::cout << std::endl;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	int j;

	noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	for (j=0;j<iterations;j++) {
		noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "CPU-Only Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	std::cout << std::endl;

	// switch to Log10 of float
	std::cout << "----------------------------------------------------------" << std::endl;

	gr::clenabled::clLog_impl *testLog=NULL;
	testLog = new gr::clenabled::clLog_impl(opencltype,selectorType,platformId,devId,20.0,0,true);

	testLog->setBufferLength(largeBlockSize);

	numItems = largeBlockSize;

	std::cout << "Testing Log10 float performance with " << numItems << " items..." << std::endl;
	std::cout << "Note: gnuradio log10 uses the following calculation: 'out[i] = n * log10(std::max(in[i], (float) 1e-18)) + k';" << std::endl;
	std::cout << "the extra max() function adds extra time to the call versus a straight log10." << std::endl;
	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = testLog->testOpenCL(numItems,ninitems,inputFloatPointers,outputFloatPointers);
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
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;


	noutputitems = testLog->testCPU(numItems,ninitems,inputFloatPointers,outputFloatPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = testLog->testCPU(numItems,ninitems,inputFloatPointers,outputFloatPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "CPU-only Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	delete testLog;

	// switch to SNR Helper
	std::cout << "----------------------------------------------------------" << std::endl;
	std::vector<float> inputFloats2;

	for (i=0;i<largeBlockSize;i++) {
		inputFloats2.push_back(((float)i+1)/2.0);
	}

	inputFloatPointers.push_back((const void *)&inputFloats2[0]);


	gr::clenabled::clSNR_impl *testSNR=NULL;
	testSNR = new gr::clenabled::clSNR_impl(opencltype,selectorType,platformId,devId,20.0,0.0,true);

	testSNR->setBufferLength(largeBlockSize);

	numItems = largeBlockSize;

	std::cout << "Testing SNR Helper float performance with " << numItems << " items..." << std::endl;

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = testSNR->testOpenCL(numItems,ninitems,inputFloatPointers,outputFloatPointers);
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
	std::cout << "Running on: " << test->getPlatformName() << std::endl;
	std::cout << std::endl;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;


	// Call CPU to initialize buffer
	noutputitems = testSNR->testCPU(numItems,ninitems,inputFloatPointers,outputFloatPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = testSNR->testCPU(numItems,ninitems,inputFloatPointers,outputFloatPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "CPU-only Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	delete testSNR;

	// ----------------------------------------------------------------------
	// Clean up
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
		test = new gr::clenabled::clMathOp_impl(DTYPE_COMPLEX,sizeof(SComplex),opencltype,selectorType,platformId,devId,MATHOP_MULTIPLY,true);
	}
	catch (...) {
		std::cout << "ERROR: error setting up OpenCL environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		return false;
	}

	std::cout << "Max constant items: " << test->MaxConstItems() << std::endl;
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

	std::chrono::time_point<std::chrono::steady_clock> start, end;

	noutputitems = test->testOpenCL(largeBlockSize,ninitems, inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,ninitems, inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

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
	std::cout << "Running on: " << test->getPlatformName() << std::endl;
	std::cout << std::endl;

	float elapsed_time,throughput;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	int j;

	noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	for (j=0;j<iterations;j++) {
		noutputitems = test->testCPU(largeBlockSize,ninitems,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "CPU-only Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

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

bool testCostasLoop() {
	std::cout << "----------------------------------------------------------" << std::endl;
	std::cout << "Testing Costas Loop performance with " << largeBlockSize << " items..." << std::endl;

	gr::clenabled::clCostasLoop_impl *test=NULL;
	try {
		test = new gr::clenabled::clCostasLoop_impl(opencltype,selectorType,platformId,devId,0.00199,2,true);
	}
	catch (...) {
		std::cout << "ERROR: error setting up OpenCL environment." << std::endl;

		if (test != NULL) {
			delete test;
		}

		return false;
	}

	std::cout << "Max constant items: " << test->MaxConstItems() << std::endl;
	test->setBufferLength(largeBlockSize);

	int i;
	int numItems=10;
	std::vector<gr_complex> inputItems1;
	std::vector<gr_complex> outputItems;
	std::vector<int> ninitems;
//	ninitems.push_back(numItems);

	std::vector< const void *> inputPointers;
	std::vector<void *> outputPointers;

	int noutputitems;

	for (i=1;i<=largeBlockSize;i++) {
		inputItems1.push_back(gr_complex(1.0f,0.5f));
		outputItems.push_back(gr_complex(0.0,0.0));
	}

	inputPointers.push_back((const void *)&inputItems1[0]);
	outputPointers.push_back((void *)&outputItems[0]);

	std::chrono::time_point<std::chrono::steady_clock> start, end;

	noutputitems = test->testOpenCL(largeBlockSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(largeBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

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

	std::cout << "Running on: " << test->getPlatformName() << std::endl;
	std::cout << std::endl;
	float elapsed_time,throughput;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "OpenCL Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	int j;

	noutputitems = test->testCPU(largeBlockSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	for (j=0;j<iterations;j++) {
		noutputitems = test->testCPU(largeBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = largeBlockSize / elapsed_time;

	std::cout << "CPU-only Run Time:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_time << " s  (" << throughput << " sps)" << std::endl << std::endl;

	std::cout << std::endl;

	if (test != NULL) {
		delete test;
	}

	inputPointers.clear();
	outputPointers.clear();
	inputItems1.clear();
	outputItems.clear();
	ninitems.clear();

	return true;
}

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

bool testLowPassFilter() {
	gr::clenabled::clFilter_impl *test=NULL;
	double gain=1.0;
	double samp_rate;
	double cutoff_freq;
	double transition_width;

	int nthreads = 4;

	// -------------------------  TIME DOMAIN FILTER -------------------------------------------------
	samp_rate=10000000;
	cutoff_freq=100000.0;
	transition_width = cutoff_freq * 0.2;
	try {
		std::cout << "------------------------------------------------------------------------------------------------------" << std::endl;
		std::cout << "Testing TIME DOMAIN OpenCL filter performance with 10 MSPS sample rate" << std::endl;
		std::cout << "NOTE: input block sizes need to be adjusted for OpenCL hardware and the number of filter taps." << std::endl;

		test = new gr::clenabled::clFilter_impl(opencltype,selectorType,platformId,devId,nthreads,
				gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width),1,true,true);
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

	int fdBlockSize, tdBufferSize;
	int optimalSize;

	int i;

	std::vector<gr_complex> inputItems;
	std::vector<gr_complex> outputItems;

	std::vector< const void *> inputPointers;
	std::vector<void *> outputPointers;

	int noutputitems;

	verifyBuffers(largeBlockSize,inputItems,outputItems,inputPointers,outputPointers);

	std::chrono::time_point<std::chrono::steady_clock> start, end;

	fdBlockSize = test->freqDomainSampleBlockSize();
	tdBufferSize = test->getCurrentBufferSize();

	std::cout << "Filter parameters: cutoff freq: " << cutoff_freq << " transition width: " << transition_width << " and " << test->taps().size() << " taps..." << std::endl;
	std::cout << "OpenCL and CPU frequency domain filter nsamples block size: " << fdBlockSize << std::endl;
	std::cout << "OpenCL time domain maximum input sample size: " << tdBufferSize << std::endl;

	// if nsamples block size > max input sample size we'll need to go to the next multiple we have a problem.
	optimalSize = (int)((float)tdBufferSize / (float)fdBlockSize) * fdBlockSize;
	if (optimalSize > largeBlockSize)
		optimalSize = largeBlockSize;

	std::cout << "Shared optimal block size: " << optimalSize << " samples." << std::endl;
	// So the number of samples used has to be a value that satisfies both of these

	if (optimalSize > 0) {
		tdBufferSize = optimalSize;
		fdBlockSize = optimalSize;
	}

	verifyBuffers(tdBufferSize,inputItems,outputItems,inputPointers,outputPointers);
	test->setTimeDomainFilterVariables(tdBufferSize);

	noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

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

	std::chrono::duration<double> elapsed_seconds = end-start;
	std::cout << "OpenCL Run Time for 20% transition filter with " << tdBufferSize << " samples:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	// Running with narrower filter:
	transition_width = (int)(cutoff_freq*0.15);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);
	fdBlockSize = test->freqDomainSampleBlockSize();
	tdBufferSize = test->getCurrentBufferSize();

	std::cout << "Rerunning with Filter parameters: cutoff freq: " << cutoff_freq << " transition width: " << transition_width << " and " << test->taps().size() << " taps..." << std::endl;
	std::cout << "OpenCL and CPU frequency domain filter nsamples block size: " << fdBlockSize << std::endl;
	std::cout << "OpenCL time domain maximum input sample size: " << tdBufferSize << std::endl;

	// if nsamples block size > max input sample size we'll need to go to the next multiple we have a problem.
	optimalSize = (int)((float)tdBufferSize / (float)fdBlockSize) * fdBlockSize;
	std::cout << "Shared optimal block size: " << optimalSize << " samples." << std::endl;
	if (optimalSize > 0) {
		tdBufferSize = optimalSize;
		fdBlockSize = optimalSize;
	}

	verifyBuffers(tdBufferSize,inputItems,outputItems,inputPointers,outputPointers);
	test->setTimeDomainFilterVariables(tdBufferSize);

	noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();
	elapsed_seconds = end-start;
	std::cout << "OpenCL Run Time for 15% transition filter with " << tdBufferSize << " samples:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	// Running with narrower filter:
	transition_width = (int)(cutoff_freq*0.1);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);
	fdBlockSize = test->freqDomainSampleBlockSize();
	tdBufferSize = test->getCurrentBufferSize();

	std::cout << "Rerunning with Filter parameters: cutoff freq: " << cutoff_freq << " transition width: " << transition_width << " and " << test->taps().size() << " taps..." << std::endl;
	std::cout << "OpenCL and CPU frequency domain filter nsamples block size: " << fdBlockSize << std::endl;
	std::cout << "OpenCL time domain maximum input sample size: " << tdBufferSize << std::endl;

	// if nsamples block size > max input sample size we'll need to go to the next multiple we have a problem.
	optimalSize = (int)((float)tdBufferSize / (float)fdBlockSize) * fdBlockSize;
	std::cout << "Shared optimal block size: " << optimalSize << " samples." << std::endl;
	if (optimalSize > 0) {
		tdBufferSize = optimalSize;
		fdBlockSize = optimalSize;
	}

	verifyBuffers(tdBufferSize,inputItems,outputItems,inputPointers,outputPointers);
	test->setTimeDomainFilterVariables(tdBufferSize);

	noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();
	elapsed_seconds = end-start;
	std::cout << "OpenCL Run Time for 10% transition filter with " << tdBufferSize << " samples:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	// Running with narrower filter:
	transition_width = (int)(cutoff_freq*0.05);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);
	std::cout << "Rerunning with Filter parameters: cutoff freq: " << cutoff_freq << " transition width: " << transition_width << " and " << test->taps().size() << " taps..." << std::endl;
	fdBlockSize = test->freqDomainSampleBlockSize();
	tdBufferSize = test->getCurrentBufferSize();

	std::cout << "OpenCL and CPU frequency domain filter nsamples block size: " << fdBlockSize << std::endl;
	std::cout << "OpenCL time domain maximum input sample size: " << tdBufferSize << std::endl;

	// if nsamples block size > max input sample size we'll need to go to the next multiple we have a problem.
	optimalSize = (int)((float)tdBufferSize / (float)fdBlockSize) * fdBlockSize;
	std::cout << "Shared optimal block size: " << optimalSize << " samples." << std::endl;
	if (optimalSize > 0) {
		tdBufferSize = optimalSize;
		fdBlockSize = optimalSize;
	}

	verifyBuffers(tdBufferSize,inputItems,outputItems,inputPointers,outputPointers);
	test->setTimeDomainFilterVariables(tdBufferSize);

	noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();
	elapsed_seconds = end-start;
	std::cout << "OpenCL Run Time for 5% transition filter with " << tdBufferSize << " samples:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	// -------------------------  FREQUENCY DOMAIN FILTER -------------------------------------------------
	int fftSize;

	samp_rate=10000000;
	cutoff_freq=100000.0;
	transition_width = cutoff_freq * 0.2;
	try {
		std::cout << "------------------------------------------------------------------------------------------------------" << std::endl;
		std::cout << "Testing FREQUENCY DOMAIN OpenCL filter performance with 10 MSPS sample rate" << std::endl;
		std::cout << "NOTE: input block sizes need to be adjusted for OpenCL hardware and the number of filter taps." << std::endl;

		delete test;

		test = new gr::clenabled::clFilter_impl(opencltype,selectorType,platformId,devId,1,
				gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width),1,true,false);
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

	std::cout << "Filter parameters: cutoff freq: " << cutoff_freq << " transition width: " << transition_width << " and " << test->taps().size() << " taps..." << std::endl;
	std::cout << "OpenCL and CPU frequency domain filter nsamples block size: " << fdBlockSize << std::endl;

	optimalSize = (int)((float)largeBlockSize / (float)fdBlockSize) * fdBlockSize;
	std::cout << "Shared optimal block size: " << optimalSize << " samples." << std::endl;
	// So the number of samples used has to be a value that satisfies both of these

	if (optimalSize > 0) {
		tdBufferSize = optimalSize;
		fdBlockSize = optimalSize;
	}
	else {
		tdBufferSize = fdBlockSize;  // the block is bigger.  We'll segfault if we don't do this.
		std::cout << "Running with " << tdBufferSize << " samples." << std::endl;
	}

	verifyBuffers(tdBufferSize,inputItems,outputItems,inputPointers,outputPointers);
	noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

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

	elapsed_seconds = end-start;
	std::cout << "OpenCL Run Time for 20% transition filter with " << tdBufferSize << " samples:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	// Running with narrower filter:
	transition_width = (int)(cutoff_freq*0.15);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);

	fdBlockSize = test->freqDomainSampleBlockSize();

	std::cout << "Rerunning with Filter parameters: cutoff freq: " << cutoff_freq << " transition width: " << transition_width << " and " << test->taps().size() << " taps..." << std::endl;
	std::cout << "OpenCL and CPU frequency domain filter nsamples block size: " << fdBlockSize << std::endl;

	optimalSize = (int)((float)largeBlockSize / (float)fdBlockSize) * fdBlockSize;
	std::cout << "Shared optimal block size: " << optimalSize << " samples." << std::endl;
	if (optimalSize > 0) {
		tdBufferSize = optimalSize;
		fdBlockSize = optimalSize;
	}
	else {
		tdBufferSize = fdBlockSize;  // the block is bigger.  We'll segfault if we don't do this.
		std::cout << "Running with " << tdBufferSize << " samples." << std::endl;
	}


//	test->setFilterVariables(tdBufferSize);
	noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();
	elapsed_seconds = end-start;
	std::cout << "OpenCL Run Time for 15% transition filter with " << tdBufferSize << " samples:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	// Running with narrower filter:
	transition_width = (int)(cutoff_freq*0.1);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);

	fdBlockSize = test->freqDomainSampleBlockSize();

	std::cout << "Rerunning with Filter parameters: cutoff freq: " << cutoff_freq << " transition width: " << transition_width << " and " << test->taps().size() << " taps..." << std::endl;
	std::cout << "OpenCL and CPU frequency domain filter nsamples block size: " << fdBlockSize << std::endl;

	optimalSize = (int)((float)largeBlockSize / (float)fdBlockSize) * fdBlockSize;
	std::cout << "Shared optimal block size: " << optimalSize << " samples." << std::endl;
	if (optimalSize > 0) {
		tdBufferSize = optimalSize;
		fdBlockSize = optimalSize;
	}
	else {
		tdBufferSize = fdBlockSize;  // the block is bigger.  We'll segfault if we don't do this.
		std::cout << "Running with " << tdBufferSize << " samples." << std::endl;
	}

	verifyBuffers(tdBufferSize,inputItems,outputItems,inputPointers,outputPointers);
//	test->setFilterVariables(tdBufferSize);
	noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();
	elapsed_seconds = end-start;
	std::cout << "OpenCL Run Time for 10% transition filter with " << tdBufferSize << " samples:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	// Running with narrower filter:
	transition_width = (int)(cutoff_freq*0.05);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);

	// if nsamples block size > max input sample size we'll need to go to the next multiple we have a problem.
	fdBlockSize = test->freqDomainSampleBlockSize();

	std::cout << "Rerunning with Filter parameters: cutoff freq: " << cutoff_freq << " transition width: " << transition_width << " and " << test->taps().size() << " taps..." << std::endl;
	std::cout << "OpenCL and CPU frequency domain filter nsamples block size: " << fdBlockSize << std::endl;


	optimalSize = (int)((float)largeBlockSize / (float)fdBlockSize) * fdBlockSize;
	std::cout << "Shared optimal block size: " << optimalSize << " samples." << std::endl;
	if (optimalSize > 0) {
		tdBufferSize = optimalSize;
		fdBlockSize = optimalSize;
	}
	else {
		tdBufferSize = fdBlockSize;  // the block is bigger.  We'll segfault if we don't do this.
		std::cout << "Running with " << tdBufferSize << " samples." << std::endl;
	}

	verifyBuffers(tdBufferSize,inputItems,outputItems,inputPointers,outputPointers);
//	test->setFilterVariables(tdBufferSize);
	noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testOpenCL(tdBufferSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();
	elapsed_seconds = end-start;
	std::cout << "OpenCL Run Time for 5% transition filter with " << tdBufferSize << " samples:   " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	// ---------------------- CPU TESTS -----------------------------------------
	transition_width = (int)(cutoff_freq*0.20);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);
	fdBlockSize = test->freqDomainSampleBlockSize();
	tdBufferSize = test->getCurrentBufferSize();

	std::cout << "------------------------------------------------------------------------------------------------------" << std::endl;
	std::cout << "Testing CPU-Only filter performance with 10 MSPS sample rate" << std::endl;
	std::cout << "OpenCL and CPU frequency domain filter nsamples block size: " << fdBlockSize << std::endl;
	std::cout << "OpenCL time domain maximum input sample size: " << tdBufferSize << std::endl;

	// if nsamples block size > max input sample size we'll need to go to the next multiple we have a problem.
	optimalSize = (int)((float)tdBufferSize / (float)fdBlockSize) * fdBlockSize;
	std::cout << "Shared optimal block size: " << optimalSize << " samples." << std::endl;
	if (optimalSize > 0) {
		tdBufferSize = optimalSize;
		fdBlockSize = optimalSize;
	}
	else {
		tdBufferSize = fdBlockSize;  // the block is bigger.  We'll segfault if we don't do this.
		std::cout << "Running with " << tdBufferSize << " samples." << std::endl;
	}

	verifyBuffers(tdBufferSize,inputItems,outputItems,inputPointers,outputPointers);
//	test->setFilterVariables(tdBufferSize);
	noutputitems = test->testCPUFFT(fdBlockSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testCPUFFT(fdBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;
	std::cout << "CPU-only Run Time for 20% filter with " << fdBlockSize << " samples: " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	// CPU
	transition_width = (int)(cutoff_freq*0.15);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);

	fdBlockSize = test->freqDomainSampleBlockSize();

	std::cout << "OpenCL and CPU frequency domain filter nsamples block size: " << fdBlockSize << std::endl;

	optimalSize = (int)((float)largeBlockSize / (float)fdBlockSize) * fdBlockSize;
	std::cout << "Shared optimal block size: " << optimalSize << " samples." << std::endl;
	if (optimalSize > 0) {
		tdBufferSize = optimalSize;
		fdBlockSize = optimalSize;
	}
	else {
		tdBufferSize = fdBlockSize;  // the block is bigger.  We'll segfault if we don't do this.
		std::cout << "Running with " << tdBufferSize << " samples." << std::endl;
	}

	verifyBuffers(tdBufferSize,inputItems,outputItems,inputPointers,outputPointers);
//	test->setFilterVariables(tdBufferSize);
	noutputitems = test->testCPUFFT(fdBlockSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testCPUFFT(fdBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;
	std::cout << "CPU-only Run Time for 15% filter with " << fdBlockSize << " samples: " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	// CPU
	transition_width = (int)(cutoff_freq*0.10);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);

	fdBlockSize = test->freqDomainSampleBlockSize();

	std::cout << "OpenCL and CPU frequency domain filter nsamples block size: " << fdBlockSize << std::endl;

	optimalSize = (int)((float)largeBlockSize / (float)fdBlockSize) * fdBlockSize;
	std::cout << "Shared optimal block size: " << optimalSize << " samples." << std::endl;
	if (optimalSize > 0) {
		tdBufferSize = optimalSize;
		fdBlockSize = optimalSize;
	}
	else {
		tdBufferSize = fdBlockSize;  // the block is bigger.  We'll segfault if we don't do this.
		std::cout << "Running with " << tdBufferSize << " samples." << std::endl;
	}

	verifyBuffers(tdBufferSize,inputItems,outputItems,inputPointers,outputPointers);
//	test->setFilterVariables(tdBufferSize);
	noutputitems = test->testCPUFFT(fdBlockSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testCPUFFT(fdBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;
	std::cout << "CPU-only Run Time for 10% filter with " << fdBlockSize << " samples: " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

	transition_width = (int)(cutoff_freq*0.05);
	test->set_taps2(gr::clenabled::firdes::low_pass(gain,samp_rate,cutoff_freq,transition_width));
	test->TestNotifyNewFilter(largeBlockSize);

	fdBlockSize = test->freqDomainSampleBlockSize();

	std::cout << "OpenCL and CPU frequency domain filter nsamples block size: " << fdBlockSize << std::endl;

	optimalSize = (int)((float)largeBlockSize / (float)fdBlockSize) * fdBlockSize;
	std::cout << "Shared optimal block size: " << optimalSize << " samples." << std::endl;
	if (optimalSize > 0) {
		tdBufferSize = optimalSize;
		fdBlockSize = optimalSize;
	}
	else {
		tdBufferSize = fdBlockSize;  // the block is bigger.  We'll segfault if we don't do this.
		std::cout << "Running with " << tdBufferSize << " samples." << std::endl;
	}

	verifyBuffers(tdBufferSize,inputItems,outputItems,inputPointers,outputPointers);
//	test->setFilterVariables(tdBufferSize);
	noutputitems = test->testCPUFFT(fdBlockSize,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->testCPUFFT(fdBlockSize,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;
	std::cout << "CPU-only Run Time for 05% filter with a " << fdBlockSize << " samples: " << std::fixed << std::setw(11)
    << std::setprecision(6) << elapsed_seconds.count()/(float)iterations << " s" << std::endl;

	std::cout << std::endl;

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
//			std::cout << "Usage: [<test buffer size>] [--gpu] [--cpu] [--accel] [--any]" << std::endl;
			std::cout << "Usage: [--gpu] [--cpu] [--accel] [--any] [--device=<platformid>:<device id>] [number of samples (default is 8192)]" << std::endl;
			std::cout << "where: --gpu, --cpu, --accel[erator], or any defines the type of OpenCL device opened." << std::endl;
			std::cout << "The optional --device argument allows for a specific OpenCL platform and device to be chosen.  Use the included clview utility to get the numbers." << std::endl;
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
/*
	CppUnit::TextTestRunner runner;
	std::ofstream xmlfile(get_unittest_path("clenabled.xml").c_str());
	CppUnit::XmlOutputter *xmlout = new CppUnit::XmlOutputter(&runner.result(), xmlfile);

	runner.addTest(qa_clenabled::suite());
	runner.setOutputter(xmlout);

	bool was_successful = runner.run("", false);  was_successful = testMultiply();
*/
/*
	was_successful = test_complex();
	std::cout << std::endl;
*/

	was_successful = testMultiplyConst();
	std::cout << std::endl;

	was_successful = testCostasLoop();
	std::cout << std::endl;

	was_successful = testSigSource();
	std::cout << std::endl;

	was_successful = testMultiply();
	std::cout << std::endl;

	was_successful = testComplexToMag();
	std::cout << std::endl;

	was_successful = testComplexToMagPhase();
	std::cout << std::endl;

	was_successful = testComplexToArg();
	std::cout << std::endl;

	was_successful = testMagPhaseToComplex();
	std::cout << std::endl;

	was_successful = testQuadDemod();
	std::cout << std::endl;

	was_successful = testFFT(true);
	std::cout << std::endl;
/*
	was_successful = testLowPassFilter();
	std::cout << std::endl;
*/
	return was_successful ? 0 : 1;
}

