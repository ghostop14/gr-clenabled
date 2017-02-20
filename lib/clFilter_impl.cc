/* -*- c++ -*- */
/* 
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "clFilter_impl.h"

namespace gr {
  namespace clenabled {
    clFilter::sptr
    clFilter::make(int openclPlatform, int decimation,
            const std::vector<float> &taps,
            int nthreads,int setDebug)
    {

      	if (setDebug == 1) {
            return gnuradio::get_initial_sptr
              (new clFilter_impl(openclPlatform,decimation,taps,nthreads,true));
      	}
      	else {
            return gnuradio::get_initial_sptr
              (new clFilter_impl(openclPlatform,decimation,taps,nthreads,false));
      	}

    }

    void
	clFilter_impl::set_taps2(const std::vector<float> &taps) {
        d_new_taps = taps;
        d_updated = true;
    }

    void
	clFilter_impl::TestNotifyNewFilter(int noutput_items) {
    	// This is only used in our test app since work isn't called.
    	int ninput_items = noutput_items * fft_filter_ccf::d_decimation;
        if (d_updated){
        	// set_taps sets d_nsamples so changed this line.
        	d_nsamples = fft_filter_ccf::set_taps(d_new_taps);
			d_updated = false;
			set_output_multiple(d_nsamples);
			setFilterVariables(noutput_items);
			prevTaps = d_ntaps;
			prevInputLength = ninput_items;
        }
    }

    std::vector<float> clFilter_impl::taps() const {
    	return fft_filter_ccf::taps();
    }

    void clFilter_impl::set_nthreads(int n) {
    	fft_filter_ccf::set_nthreads(n);
    }
    /*
     * The private constructor
     */
    clFilter_impl::clFilter_impl(int openclPlatform, int decimation, const std::vector<float> &taps,int nthreads,bool setDebug)
      : gr::sync_decimator("clLowPassFilter",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)),decimation),
			  fft_filter_ccf(decimation, taps,nthreads),
			  GRCLBase(DTYPE_COMPLEX, sizeof(gr_complex),openclPlatform),
			  d_updated(false)
    {
    	prevTaps = d_ntaps;
    	prevInputLength = 8192;

    	int err;

        clfftSetupData fftSetup;
        err = clfftInitSetupData(&fftSetup);
        err = clfftSetup(&fftSetup);

    	// set up for initial 8192 sample input buffer
    	setFilterVariables(prevInputLength);
    }

    /*
     * Our virtual destructor.
     */
    clFilter_impl::~clFilter_impl()
    {
    	if (aBuffer)
    		delete aBuffer;

    	if (bBuffer)
    		delete bBuffer;

    	if (cBuffer)
    		delete cBuffer;

    	if (zeroBuff)
    		delete zeroBuff;

        /* Release clFFT library. */
        clfftTeardown( );

    	// Called in grclbase destructor
    	// cleanup();
    }

    void clFilter_impl::setFilterVariables(int ninput_items) {
		paddingLength = d_ntaps - 1;
		paddingBytes = dataSize*paddingLength;

    	resultLengthPoints = ninput_items + d_ntaps - 1;
		inputLengthBytes = ninput_items*dataSize;
		paddedBufferLengthBytes=resultLengthPoints*dataSize;

		filterLengthBytes=d_taps.size() * sizeof(float);


    	kernelCode = "";
    	kernelCodeWithConst = "";
        std::string fnName = "";

    	if (dataType==DTYPE_COMPLEX) {
    		fnName = "td_FIR_complex";
    		kernelCode +="struct ComplexStruct {\n";
    		kernelCode +="	float real;\n";
    		kernelCode +="	float imag;\n";
    		kernelCode +="};\n";
    		kernelCode +="typedef struct ComplexStruct SComplex;\n";

    		kernelCode +="__kernel void td_FIR_complex\n";
    		kernelCode +="( __global const SComplex *restrict InputArray, // Length N\n";
    		kernelCode +="__global const float *restrict FilterArray, // Length K\n";
    		kernelCode +="__global SComplex *restrict OutputArray // Length N+K-1\n";
    		kernelCode +=")\n";
    		kernelCode +="{\n";

    		kernelCode +="	__local float local_copy_filter_array[K];\n";
    		kernelCode +="  size_t gid=get_global_id(0);\n";
//			kernelCode +="  size_t lid = gId; // get_local_id(0);\n";
    		kernelCode +="	if (gid < K)\n";
    		kernelCode +="		local_copy_filter_array[gid] = FilterArray[gid];\n";
    		kernelCode +="	barrier(CLK_LOCAL_MEM_FENCE);\n";
    		kernelCode +="	// Perform Compute\n";
    		kernelCode +="	SComplex result;\n";
    		kernelCode +="	result.real=0.0f;\n";
    		kernelCode +="	result.imag=0.0f;\n";
//    		kernelCode +="	// Unroll the loop for speed.\n";
    		// Too many taps to unroll this loop.  1000+ taps may decrease performance
    		// due to increased instruction base.
//			kernelCode +="	#pragma unroll\n";
    		kernelCode +="	for (int i=0; i<K; i++) {\n";
			kernelCode +="		result.real += local_copy_filter_array[K-1-i]*InputArray[gid+i].real;\n";
			kernelCode +="		result.imag += local_copy_filter_array[K-1-i]*InputArray[gid+i].imag;\n";
//			kernelCode +="		result.real += FilterArray[K-1-i]*InputArray[lid+i].real;\n";
//			kernelCode +="		result.imag += FilterArray[K-1-i]*InputArray[lid+i].imag;\n";
    		kernelCode +="	}\n";
    		kernelCode +="	OutputArray[gid].real = result.real;\n";
    		kernelCode +="	OutputArray[gid].imag = result.imag;\n";
    		kernelCode +="}\n";

    		// If the filter array is < constant memory, this approach is faster:
    		kernelCodeWithConst +="struct ComplexStruct {\n";
    		kernelCodeWithConst +="	float real;\n";
    		kernelCodeWithConst +="	float imag;\n";
    		kernelCodeWithConst +="};\n";
    		kernelCodeWithConst +="typedef struct ComplexStruct SComplex;\n";

    		kernelCodeWithConst +="__kernel void td_FIR_complex\n";
    		kernelCodeWithConst +="( __global const SComplex *restrict InputArray, // Length N\n";
    		kernelCodeWithConst +="__constant float * FilterArray, // Length K\n";
    		kernelCodeWithConst +="__global SComplex *restrict OutputArray // Length N+K-1\n";
    		kernelCodeWithConst +=")\n";
    		kernelCodeWithConst +="{\n";
    		kernelCodeWithConst +="  size_t gid=get_global_id(0);\n";
    		kernelCodeWithConst +="	// Perform Compute\n";
    		kernelCodeWithConst +="	SComplex result;\n";
    		kernelCodeWithConst +="	result.real=0.0f;\n";
    		kernelCodeWithConst +="	result.imag=0.0f;\n";
    		kernelCodeWithConst +="	for (int i=0; i<K; i++) {\n";
			kernelCodeWithConst +="		result.real += FilterArray[K-1-i]*InputArray[gid+i].real;\n";
			kernelCodeWithConst +="		result.imag += FilterArray[K-1-i]*InputArray[gid+i].imag;\n";
    		kernelCodeWithConst +="	}\n";
    		kernelCodeWithConst +="	OutputArray[gid].real = result.real;\n";
    		kernelCodeWithConst +="	OutputArray[gid].imag = result.imag;\n";
    		kernelCodeWithConst +="}\n";
    	}
    	else {
    		fnName = "td_FIR_float";
    		kernelCode += "__kernel void td_FIR_float\n";
    		kernelCode += "( __global const float *restrict InputArray, // Length N\n";
    		kernelCode += "__global const float *restrict FilterArray, // Length K\n";
    		kernelCode += "__global float *restrict OutputArray // Length N+K-1\n";
    		kernelCode += ")\n";
    		kernelCode += "{\n";
    		kernelCode += "	__local float local_copy_input_array[2*K+N];\n";
    		kernelCode += "	__local float local_copy_filter_array[K];\n";
    		kernelCode += "	InputArray += get_group_id(0) * N;\n";
    		kernelCode += "	FilterArray += get_group_id(0) * K;\n";
    		kernelCode += "	OutputArray += get_group_id(0) * (N+K);\n";
    		kernelCode += "	// Copy from global to local\n";
    		kernelCode += "	local_copy_input_array[get_local_id(0)] = InputArray[get_local_id(0)];\n";
    		kernelCode += "	if (get_local_id(0) < K)\n";
    		kernelCode += "		local_copy_filter_array[get_local_id(0)] = FilterArray[get_local_id(0)];\n";
    		kernelCode += "	barrier(CLK_LOCAL_MEM_FENCE);\n";
    		kernelCode += "	// Perform Compute\n";
    		kernelCode += "	float result=0.0f;\n";
    		kernelCode += "	for (int i=0; i<K; i++) {\n";
    		kernelCode += "		result += local_copy_filter_array[K-1-i]*local_copy_input_array[get_local_id(0)+i];\n";
    		kernelCode += "	}\n";
    		kernelCode += "	OutputArray[get_local_id(0)] = result;\n";
    		kernelCode += "}\n";

    		kernelCodeWithConst += "__kernel void td_FIR_float\n";
    		kernelCodeWithConst += "( __global const float *restrict InputArray, // Length N\n";
    		kernelCodeWithConst += "__global const float *restrict FilterArray, // Length K\n";
    		kernelCodeWithConst += "__global float *restrict OutputArray // Length N+K-1\n";
    		kernelCodeWithConst += ")\n";
    		kernelCodeWithConst += "{\n";
    		kernelCodeWithConst += "	__local float local_copy_input_array[2*K+N];\n";
    		kernelCodeWithConst += "	__local float local_copy_filter_array[K];\n";
    		kernelCodeWithConst += "	InputArray += get_group_id(0) * N;\n";
    		kernelCodeWithConst += "	FilterArray += get_group_id(0) * K;\n";
    		kernelCodeWithConst += "	OutputArray += get_group_id(0) * (N+K);\n";
    		kernelCodeWithConst += "	// Copy from global to local\n";
    		kernelCodeWithConst += "	local_copy_input_array[get_local_id(0)] = InputArray[get_local_id(0)];\n";
    		kernelCodeWithConst += "	if (get_local_id(0) < K)\n";
    		kernelCodeWithConst += "		local_copy_filter_array[get_local_id(0)] = FilterArray[get_local_id(0)];\n";
    		kernelCodeWithConst += "	barrier(CLK_LOCAL_MEM_FENCE);\n";
    		kernelCodeWithConst += "	// Perform Compute\n";
    		kernelCodeWithConst += "	float result=0.0f;\n";
    		kernelCodeWithConst += "	for (int i=0; i<K; i++) {\n";
    		kernelCodeWithConst += "		result += local_copy_filter_array[K-1-i]*local_copy_input_array[get_local_id(0)+i];\n";
    		kernelCodeWithConst += "	}\n";
    		kernelCodeWithConst += "	OutputArray[get_local_id(0)] = result;\n";
    		kernelCodeWithConst += "}\n";

    	}

    	std::string lbDefines;
    	lbDefines = "#define N " + std::to_string(ninput_items) + "\n";
    	lbDefines += "#define K "+ std::to_string(d_ntaps) + "\n";

    	std::string tmpKernelCode;
    	if ((d_ntaps*sizeof(float)) < maxConstMemSize) {
    		tmpKernelCode = lbDefines + kernelCodeWithConst;

    		if (debugMode)
        		std::cout << "OpenCL INFO: Filter is using kernel code with faster constant memory." << std::endl;
    	}
    	else {
    		tmpKernelCode = lbDefines + kernelCode;
    		if (debugMode)
        		std::cout << "OpenCL INFO: The number of taps exceeds OpenCL constant memory space for your device.  Filter is using slower kernel code with filter copy to local memory." << std::endl;
    	}

    	// Try releasing any buffers before compiling a new kernel.
    	if (aBuffer) {
    		delete aBuffer;
    		aBuffer = NULL;
    	}
    	if (bBuffer) {
    		delete bBuffer;
    		bBuffer = NULL;
    	}
    	if (cBuffer) {
    		delete cBuffer;
    		cBuffer = NULL;
    	}

    	GRCLBase::CompileKernel((const char *)tmpKernelCode.c_str(),(const char *)fnName.c_str());

        setBufferLength(ninput_items);

        queue->enqueueWriteBuffer(*bBuffer,CL_TRUE,0,filterLengthBytes,&(d_taps[0]));

        int err;
        /* Setup clFFT. */
        clfftSetupData fftSetup;
        err = clfftInitSetupData(&fftSetup);
        err = clfftSetup(&fftSetup);

        /* Create a default plan for a complex FFT. */
        size_t clLengths[1];
        clLengths[0]=(size_t)ninput_items;


        err = clfftCreateDefaultPlan(&planHandle, (*context)(), dim, clLengths);

        /* Set plan parameters. */
        err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE);
        err = clfftSetLayout(planHandle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
        err = clfftSetResultLocation(planHandle, CLFFT_INPLACE);

        /* Bake the plan. */
        err = clfftBakePlan(planHandle, 1, &(*queue)(), NULL, NULL);

}

void clFilter_impl::setBufferLength(int numItems) {
	if (aBuffer)
		delete aBuffer;
	if (bBuffer)
		delete bBuffer;
	if (cBuffer)
		delete cBuffer;

	if (zeroBuff)
		delete zeroBuff;

	aBuffer = new cl::Buffer(
		*context,
		CL_MEM_READ_ONLY,
		paddedBufferLengthBytes);

	bBuffer = new cl::Buffer(
		*context,
		CL_MEM_READ_ONLY,
		filterLengthBytes);

	cBuffer = new cl::Buffer(
		*context,
		CL_MEM_READ_WRITE,
		paddedBufferLengthBytes);

	zeroBuff=new char[paddedBufferLengthBytes];

	curBufferSize = numItems;

    /* Release the plan. */
	int err;

    err = clfftDestroyPlan( &planHandle );
   /* Release clFFT library. */
    clfftTeardown( );

    }

    int
	clFilter_impl::filterGPU(int ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {

    	if (ninput_items > curBufferSize) {
    		// This could get expensive if we have to rebuild kernels
    		// in GNURadio min input items and max items should be
    		// set to the same value to ensure consistency.
    		setFilterVariables(ninput_items);
    	}

    	int inputBytes=ninput_items*dataSize;
    	// See https://www.altera.com/support/support-resources/design-examples/design-software/opencl/td-fir.html
    	// for reference.  The source code has a PDF describing implementing FIR in FPGA.

        int remaining=paddedBufferLengthBytes - inputBytes;

        queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputBytes,(void *)input_items[0]);
        queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,inputBytes,remaining,(void *)zeroBuff);

		kernel->setArg(0, *aBuffer);
		kernel->setArg(1, *bBuffer);
		kernel->setArg(2, *cBuffer);

		// Do the work
		queue->enqueueNDRangeKernel(
			*kernel,
			cl::NullRange,
			cl::NDRange(ninput_items),
			cl::NullRange);

		/*
		try {
		queue->enqueueNDRangeKernel(
			*kernel,
			cl::NullRange,
			cl::NDRange(ninput_items),
			cl::NullRange);
		}
		catch (cl::Error& err) {
			if (err.err() == CL_OUT_OF_RESOURCES) {
				std::cout << "EnqueueNDRangeKernel Error: -5 CL_OUT_OF_RESOURCES (failure to allocate resources required by implementation)" << std::endl;
			}
			else {
				std::cout << "Error: " << err.what() << " " << err.err() << std::endl;
			}
		}
		*/
		// Map cBuffer to host pointer. This enforces a sync

		cl_int err;
		int retVal;

		if (fft_filter_ccf::d_decimation == 1) {
			queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,inputBytes,(void *)output_items[0]);

			// # in=# out. Do it the quick way
			// memcpy((void *)output_items[0],output,ninput_items*dataSize);
			retVal = ninput_items;
		}
		else {
			void * output = (void *) queue->enqueueMapBuffer(
			*cBuffer,
			CL_TRUE, // block
			CL_MAP_READ,
			0,
			paddedBufferLengthBytes);
			// copy results to output buffer and increment for decimation!
			int j=0;
			int i=0;
			while(j < ninput_items) {
				if (dataType==DTYPE_COMPLEX) {
			        gr_complex *out = (gr_complex *)output_items[0];
			        gr_complex *ResultPtr = (gr_complex *)output;

					out[i++] = ResultPtr[j];
				}
				else {
			        float *out = (float *)output_items[0];
			        float *ResultPtr = (float *)output;

					out[i++] = ResultPtr[j];
				}

				j += fft_filter_ccf::d_decimation;
			}

			err = queue->enqueueUnmapMemObject(
			*cBuffer,
			(void *) output);

			retVal = i;
		}

		// Finally release our hold on accessing the memory

		return retVal;  // Accounts for decimation.
    }

    int clFilter_impl::testCPU(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return filterCPU(noutput_items,input_items,output_items);
    }

    int clFilter_impl::testOpenCL(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return filterGPU(noutput_items,input_items,output_items);;
    }

    int
	clFilter_impl::filterCPU(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        int retVal=0;
        try {
        retVal = fft_filter_ccf::filter(noutput_items,in,out);
        }
        catch (...) {
        	std::cout << "Exception in fft_filter_ccf::filter()" << std::endl;
        }
		return retVal;
    }

    int
	clFilter_impl::filterCPU2(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        int tapId;

        for (int N=0;N<noutput_items;N++) {
    		SComplex result;
    		result.real=0.0f;
    		result.imag=0.0f;
    		for (int i=0; i<d_ntaps; i++) {
    			tapId=d_ntaps-1-i;
    			result.real += d_taps[tapId]*in[N+i].real();
    			result.imag += d_taps[tapId]*in[N+i].imag();
    		}
    		out[N]=gr_complex(result.real,result.imag);
        }

		return noutput_items;
    }

    int
    clFilter_impl::work (int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
    	int ninput_items = noutput_items * fft_filter_ccf::d_decimation;
        if (d_updated){
        	// set_taps sets d_nsamples so changed this line.
        	d_nsamples = fft_filter_ccf::set_taps(d_new_taps);
			d_updated = false;
			set_output_multiple(d_nsamples);
			setFilterVariables(noutput_items);
			prevTaps = d_ntaps;
			prevInputLength = ninput_items;
        }
        else {
        	if (prevInputLength != noutput_items) {
        		// input length changed from the previous cycle.
        		// NOTE: THIS SHOULD BE AVOIDED AS TAIL DATA FROM THE PREVIOUS
        		// CYCLE WILL BE LOST and a new kernel will be built which will take some time.
        		setFilterVariables(noutput_items);
    			prevTaps = d_ntaps;
    			prevInputLength = ninput_items;
        	}
        }

        // filterCPU(noutput_items, input_items,output_items);

        filterGPU(noutput_items * fft_filter_ccf::d_decimation,input_items,output_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace clenabled */
} /* namespace gr */

