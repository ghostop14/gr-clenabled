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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// win32 (mingw/msvc) specific
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef O_BINARY
#define	OUR_O_BINARY O_BINARY
#else
#define	OUR_O_BINARY 0
#endif

// should be handled via configure
#ifdef O_LARGEFILE
#define	OUR_O_LARGEFILE	O_LARGEFILE
#else
#define	OUR_O_LARGEFILE 0
#endif


#include "clXEngine_impl.h"

bool verbose=false;
int num_channels=1024;
int num_inputs = 16;
// single_polarization=false = X/Y dual polarization
bool single_polarization = false;
int integration_time = 1024;
int iterations = 4;
int num_procs=0;

int opencltype=OCLTYPE_ANY;
int selectorType=OCLDEVICESELECTOR_FIRST;
int platformId=0;
int devId=0;

using namespace gr::clenabled;

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

int bufferToFile(const char *filename, gr_complex *buffer, long long unsigned int length) {
	FILE *d_fp = NULL;
	int fd;
	int flags;
	flags = O_WRONLY|O_CREAT|O_TRUNC|OUR_O_LARGEFILE|OUR_O_BINARY;
	if((fd = open(filename, flags, 0664)) < 0){
		return 1;
	}

	if((d_fp = fdopen (fd, "wb")) == NULL) {
		fclose(d_fp);        // don't leak file descriptor if fdopen fails.
		return 1;
	}

	// Write the data
    char *inbuf = (char*)buffer;
    int nwritten = 0;
    int d_itemsize = sizeof(gr_complex);

    while(nwritten < length) {
    	// fwrite: returns number of elements written
    	// Takes: ptr to array of elements, element size, count, file stream pointer
      long count = fwrite(inbuf, d_itemsize, length - nwritten, d_fp);
      if(count == 0) {
    	  // Error condition, nothing written for some reason.
        if(ferror(d_fp)) {
          printf("file write error.\n");
          break;
        }
        else { // is EOF?  Probably will never get to this break;
          break;
        }
      }
      nwritten += count;
      inbuf += count * d_itemsize;
    }

	// Clean up
	fclose(d_fp);
	return 0;
}

int bufferFromFile(const char *filename, gr_complex *buffer, long long unsigned int length) {
	FILE *d_fp = NULL;
	FILE *d_new_fp;

	int fd;
    if((fd = open(filename, O_RDONLY | OUR_O_LARGEFILE | OUR_O_BINARY)) < 0) {
		return 1;
	}

	if((d_fp = fdopen (fd, "rb")) == NULL) {
		fclose(d_fp);        // don't leak file descriptor if fdopen fails.
		return 1;
	}

	// Read the data
    char *inbuf = (char*)buffer;
    int nread = 0;
    int d_itemsize = sizeof(gr_complex);

    while (nread < length) {
        int itemsRead = fread(&buffer[nread], d_itemsize, length-nread, (FILE*)d_fp);

        if (itemsRead == 0) {
        	break;
        }

        nread += itemsRead;
    }

    if (nread !=length) {
    	std::cout << "ERROR: Full buffer not read. " << nread << " read, but " << length << " requested." << std::endl;
    }

	// Clean up
    if(d_fp != NULL) {
      fclose(d_fp);
    }

	return 0;
}

bool testXCorrelate() {
	std::cout << "----------------------------------------------------------" << std::endl;

	std::cout << "Testing Full OpenCL X-engine cross-correlation with the following parameters: " << std::endl;
	std::cout << "Number of stations: " << num_inputs << std::endl;
	std::cout << "Number of baselines: " << (num_inputs+1)*num_inputs/2 << std::endl;
	std::cout << "Number of channels: " << num_channels << std::endl;
	std::cout << "Polarization: ";
	if (single_polarization) {
		std::cout << "single" << std::endl;
	}
	else {
		std::cout << "X/Y" << std::endl;
	}

	std::cout << "Integration time (NTIME): " << integration_time << std::endl;
	int polarization;

	gr::clenabled::clXEngine_impl *test=NULL;
	try {
		if (single_polarization) {
			polarization = 1;
		}
		else {
			polarization = 2;
		}

		// The one specifies output triangular order rather than full matrix.
		test = new gr::clenabled::clXEngine_impl(opencltype,selectorType,platformId,devId,true,DTYPE_COMPLEX,sizeof(gr_complex),
				polarization, num_inputs, 1, num_channels, integration_time);
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
	float frequency_sampling = num_channels*frequency_signal;
	float curPhase = 0.0;
	float signal_ang_rate = 2*M_PI*frequency_signal / frequency_sampling;

	std::vector<gr_complex> inputItems_complex;
	std::vector<gr_complex> outputItems;
	std::vector<const void *> inputPointers;
	std::vector<void *> outputPointers;

	gr_complex grZero(0.0,0.0);
	gr_complex newComplex(1.0,0.5);

	for (i=0;i<num_channels*integration_time;i++) {
		inputItems_complex.push_back(gr_complex(sin(curPhase+(signal_ang_rate*i)),cos(curPhase+(signal_ang_rate*i))));
		outputItems.push_back(grZero);
	}

	for (int i=0;i<polarization;i++) {
		for (int i=0;i<num_inputs;i++) {
			inputPointers.push_back((const void *)&inputItems_complex[0]);
		}
	}

	/*
	for (int i=0;i<num_inputs-1;i++) {
		outputPointers.push_back((void *)&outputItems[0]);
	}
	*/

	// Run empty test
	int noutputitems;

	// Get a test run out of the way.
	// Note: output items is 1 since it's expecting vectors in.
	noutputitems = test->work_test(integration_time,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_test(integration_time,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	float elapsed_time,throughput;
	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = num_inputs * polarization * num_channels * integration_time / elapsed_time;
	long input_buffer_total_bytes = test->get_input_buffer_size() * sizeof(gr_complex);
	float bits_throughput = 8 * input_buffer_total_bytes / elapsed_time;

	std::cout << "Elapsed time: " << elapsed_seconds.count() << std::endl;
	std::cout << "Timing Averaging Iterations: " << iterations << std::endl;
	std::cout << "Average Run Time:   " << std::fixed << std::setw(11) << std::setprecision(6) << elapsed_time << " s" << std::endl <<
				"Total throughput: " << std::setprecision(2) << throughput << " float complex samples/sec" << std::endl <<
				"Synchronized stream (" << num_inputs << " inputs) throughput: " << throughput / num_inputs << " float complex samples/sec" << std::endl <<
				"Input processing rate (comparable to xGPU's throughput number): " << bits_throughput << " bps" << std::endl;

	// Reset test
	if (test != NULL) {
		delete test;
	}

	// ------------  Test with char input data --------------------------------------------------------
	// The one specifies output triangular order rather than full matrix.
	test = new gr::clenabled::clXEngine_impl(opencltype,selectorType,platformId,devId,true,DTYPE_COMPLEX,sizeof(gr_complex),
			polarization, num_inputs, 1, num_channels, integration_time);

	// Get a test run out of the way.
	// Note: output items is 1 since it's expecting vectors in.
	noutputitems = test->work_test(integration_time,inputPointers,outputPointers);

	start = std::chrono::steady_clock::now();
	// make iterations calls to get average.
	for (i=0;i<iterations;i++) {
		noutputitems = test->work_test(integration_time,inputPointers,outputPointers);
	}
	end = std::chrono::steady_clock::now();

	elapsed_seconds = end-start;

	elapsed_time = elapsed_seconds.count()/(float)iterations;
	throughput = num_inputs * polarization * num_channels * integration_time / elapsed_time;
	input_buffer_total_bytes = test->get_input_buffer_size() * sizeof(gr_complex);
	bits_throughput = 8 * input_buffer_total_bytes / elapsed_time;

	std::cout << "Elapsed time: " << elapsed_seconds.count() << std::endl;
	std::cout << "Timing Averaging Iterations: " << iterations << std::endl;
	std::cout << "Average Run Time:   " << std::fixed << std::setw(11) << std::setprecision(6) << elapsed_time << " s" << std::endl <<
				"Total throughput: " << std::setprecision(2) << throughput << " byte complex samples/sec" << std::endl <<
				"Synchronized stream (" << num_inputs << " inputs) throughput: " << throughput / num_inputs << " byte complex samples/sec" << std::endl <<
				"Input processing rate (comparable to xGPU's throughput number): " << bits_throughput << " bps" << std::endl;

	// Reset test
	if (test != NULL) {
		delete test;
	}

// #define TEST_GOLDEN_DATA

#ifdef TEST_GOLDEN_DATA
	// ------------------------  Test Golden Data -------------------------
	// The one specifies output triangular order rather than full matrix.
	test = new gr::clenabled::clXEngine_impl(opencltype,selectorType,platformId,devId,true,DTYPE_COMPLEX,sizeof(gr_complex),
			2, 16, 1, 1024, 1024);

	long input_length = test->get_input_buffer_size();
	long output_length = test->get_output_buffer_size();

	gr_complex *input_buffer;
	gr_complex *output_buffer;

	input_buffer = new gr_complex[input_length];
	output_buffer = new gr_complex[output_length];

	std::string input_file = "/opt/tmp/ata/xgpu_input_data.bin";
	std::string output_file = "/opt/tmp/ata/opencl_xengine_output_data.bin";

	std::cout << "Testing golden data..." << std::endl;
	std::cout << "Reading data from input file " << input_file << "..." << std::endl;

	int result = bufferFromFile(input_file.c_str(), input_buffer, input_length);

	if (result != 0) {
		std::cout << "Error reading input file." << std::endl;
	}
	else {
		test->xcorrelate((XComplex *)input_buffer, (XComplex *)output_buffer);
		std::cout << "Writing results to output file " << output_file << "..." << std::endl;
		result = bufferToFile(output_file.c_str(), output_buffer, output_length);

		if (result != 0) {
			std::cout << "Error writing to output file." << std::endl;
		}
	}

	delete[] input_buffer;
	delete[] output_buffer;

	if (test != NULL) {
		delete test;
	}
#endif

	// ----------------------------------------------------------------------
	// Clean up io buffers

	inputPointers.clear();
	outputPointers.clear();
	inputItems_complex.clear();
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
			std::cout << "Usage: [--gpu] [--cpu] [--accel] [--any] [--device=<platformid>:<device id>] [--single-polarization] [--num_inputs=#] [--integration-time=#] [number of channels (default is " << num_channels << ")]" << std::endl;
			std::cout << "where: --gpu, --cpu, --accel[erator], or any defines the type of OpenCL device opened." << std::endl;
			std::cout <<"--num_inputs=n  Number of stations.  Default is " << num_inputs  << "." << std::endl;
			std::cout <<"--integration-time=n  Default is " << integration_time << "." << std::endl;
			std::cout <<"--single-polarization If not specified, correlation will assume X and Y polarizations per input." << std::endl;
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
			else if (param.find("--num_inputs") != std::string::npos) {
				boost::replace_all(param,"--num_inputs=","");
				num_inputs=atoi(param.c_str());
			}
			else if (param.find("--integration-time") != std::string::npos) {
				boost::replace_all(param,"--integration-time=","");
				integration_time=atoi(param.c_str());
			}
			else if (strcmp(argv[i],"--single-polarization")==0) {
				single_polarization=true;
			}
			else if (atoi(argv[i]) > 0) {
				int newVal=atoi(argv[i]);

				num_channels=newVal;
				std::cout << "Running with user-defined channel count of " << num_channels << std::endl;
			}
			else {
				std::cout << "ERROR: Unknown parameter." << std::endl;
				exit(1);

			}
		}
	}
	bool was_successful;

	was_successful = testXCorrelate();

	std::cout << std::endl;

	return 0;
}

