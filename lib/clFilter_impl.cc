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
#include <volk/volk.h>

namespace gr {
  namespace clenabled {
    clFilter::sptr
    clFilter::make(int openclPlatform,int devSelector,int platformId, int devId, int decimation,
            const std::vector<float> &taps,
            int nthreads,int setDebug)
    {
      	if (setDebug == 1) {
            return gnuradio::get_initial_sptr
              (new clFilter_impl(openclPlatform,devSelector,platformId,devId,decimation,taps,nthreads,true));
      	}
      	else {
            return gnuradio::get_initial_sptr
              (new clFilter_impl(openclPlatform,devSelector,platformId,devId,decimation,taps,nthreads,false));
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
			prevTaps = d_ntaps;
        	d_nsamples = set_taps(d_new_taps);
			d_updated = false;
			try {
			set_output_multiple(d_nsamples);
			}
			catch (...) {

			}
			setFilterVariables(ninput_items);
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


    // NOTE: ONLY COMPLEX IS SUPPORTED BY THE XML GRC BLCOK RIGHT NOW.
    // IF THIS GETS UPDATED TO SUPPORT FLOATS TOO, iDataSize would need to be set up like in MathOp

    clFilter_impl::clFilter_impl(int openclPlatform,int devSelector,int platformId, int devId, int decimation, const std::vector<float> &taps,int nthreads,bool setDebug,bool bUseTimeDomain)
      : gr::sync_decimator("clFilter",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)),decimation),
			  fft_filter_ccf(decimation, taps,nthreads),
			  GRCLBase(DTYPE_COMPLEX, sizeof(gr_complex),openclPlatform,devSelector,platformId,devId, setDebug),
			  d_updated(false)
    {
        set_history(1);
        try {
        set_output_multiple(d_nsamples);
        }
        catch (...) {
        }

    	USE_TIME_DOMAIN = bUseTimeDomain;
    	prevTaps = d_ntaps;
    	// Buffer sizes need to match up with blocks based on the number of taps...
    	// This matches up with FFT Size
    	prevInputLength = (int) (2 * pow(2.0, ceil(log(double(taps.size())) / log(2.0))));;
    	d_taps = taps;

    	if (USE_TIME_DOMAIN) {
        	int imaxItems=gr::block::max_noutput_items();
        	if (imaxItems==0)
        		imaxItems=8192;

        	// So the amount of items we have for the constant space is:
        	// maxConstMemSize >= (noutput_items+n_taps)*datasize + n_taps*sizeof(float)
        	// Since we can't control the n_taps, we can control the noutput_items.
        	maxConstItems = (int)((float)(maxConstMemSize - d_taps.size()*sizeof(float))/(float)dataSize) - d_taps.size();

        	if (maxConstItems < imaxItems) {
        		imaxItems = maxConstItems;
        	}

        	set_max_noutput_items(imaxItems);
			setFilterVariables(imaxItems);
    	}
    	else {
//			set_output_multiple(prevInputLength);

			setFilterVariables(prevInputLength);
    	}
}

    /*
     * Our virtual destructor.
     */
    clFilter_impl::~clFilter_impl()
    {
    	if (curBufferSize > 0)
    		stop();
    }

    bool clFilter_impl::stop()
    {
		if (dataType==DTYPE_FLOAT) {
			if (transformedTaps_float) {
				delete[] transformedTaps_float;
				transformedTaps_float = NULL;
			}
		}

		curBufferSize = 0;

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

    	if (zeroBuff) {
    		delete[] zeroBuff;
    		zeroBuff = NULL;
    	}

    	if (tmpFFTBuff) {
    		delete[] tmpFFTBuff;
    		tmpFFTBuff = NULL;
    	}

    	if (!USE_TIME_DOMAIN) {
            /* Release the plan. */
        	int err;

            err = clfftDestroyPlan( &planHandle );
           /* Release clFFT library. */

            try {
            	clfftTeardown( );
            }
            catch(...) {
            	// safety catch.
            }
            if (ifftBuff) {
				if (dataType == DTYPE_COMPLEX) {
					delete[] (gr_complex *)ifftBuff;
					ifftBuff = NULL;
				}
				else {
					delete [] (float *)ifftBuff;
					ifftBuff = NULL;
				}
            }
    	}

    	return GRCLBase::stop();
    }

    void clFilter_impl::setFilterVariables(int ninput_items) {
        // From fft_filter.cc
    	// NOTE: MUST BE CALLED BEFORE setFilterVariables setup since some values are assumed set in there
    	// This also fills d_xformed_taps which is a gr_complex vector of size d_fftsize
        set_taps(d_taps);

        // These are setups for the time domain filtering
    	paddingLength = d_ntaps - 1;
		paddingBytes = dataSize*paddingLength;

    	resultLengthPoints = ninput_items + d_ntaps - 1;
		inputLengthBytes = ninput_items*dataSize;
		paddedBufferLengthBytes=resultLengthPoints*dataSize;

		filterLengthBytes=d_ntaps * sizeof(float);

    	if (USE_TIME_DOMAIN) {
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

        		// This version of code is for data points that exceed the constant memory size.
        		// This performance won't be very good.
        		kernelCode +="__kernel void td_FIR_complex\n";
        		kernelCode +="( __global const SComplex *restrict InputArray, // Length N\n";
        		kernelCode +="__constant float * FilterArray, // Length K\n";
        		kernelCode +="__global SComplex *restrict OutputArray // Length N+K-1\n";
        		kernelCode +=")\n";
        		kernelCode +="{\n";
        		kernelCode +="  size_t gid=get_global_id(0);\n";
        		kernelCode +="	// Perform Compute\n";
        		kernelCode +="	SComplex result;\n";
        		kernelCode +="	result.real=0.0f;\n";
        		kernelCode +="	result.imag=0.0f;\n";
        		kernelCode +="	for (int i=0; i<K; i++) {\n";
        		kernelCode +="		result.real += FilterArray[K-1-i]*InputArray[gid+i].real;\n";
        		kernelCode +="		result.imag += FilterArray[K-1-i]*InputArray[gid+i].imag;\n";
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
        		kernelCodeWithConst +="( __constant SComplex * InputArray, // Length N\n";
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
        		// This version of code is for data points that exceed the constant memory size.
        		// This performance won't be very good.
        		fnName = "td_FIR_float";
        		kernelCode +="__kernel void td_FIR_float\n";
        		kernelCode +="( __global const float *restrict InputArray, // Length N\n";
        		kernelCode +="__constant float * FilterArray, // Length K\n";
        		kernelCode +="__global float *restrict OutputArray // Length N+K-1\n";
        		kernelCode +=")\n";
        		kernelCode +="{\n";
        		kernelCode +="  size_t gid=get_global_id(0);\n";
        		kernelCode +="	// Perform Compute\n";
        		kernelCode +="	float result=0.0f;\n";
        		kernelCode +="	for (int i=0; i<K; i++) {\n";
        		kernelCode +="		result += FilterArray[K-1-i]*InputArray[gid+i];\n";
        		kernelCode +="	}\n";
        		kernelCode +="	OutputArray[gid] = result;\n";
        		kernelCode +="}\n";

        		kernelCodeWithConst +="__kernel void td_FIR_float\n";
        		kernelCodeWithConst +="( __constant float * InputArray, // Length N\n";
        		kernelCodeWithConst +="__constant float * FilterArray, // Length K\n";
        		kernelCodeWithConst +="__global float *restrict OutputArray // Length N+K-1\n";
        		kernelCodeWithConst +=")\n";
        		kernelCodeWithConst +="{\n";
        		kernelCodeWithConst +="  size_t gid=get_global_id(0);\n";
        		kernelCodeWithConst +="	// Perform Compute\n";
        		kernelCodeWithConst +="	float result=0.0f;\n";
        		kernelCodeWithConst +="	for (int i=0; i<K; i++) {\n";
    			kernelCodeWithConst +="		result += FilterArray[K-1-i]*InputArray[gid+i];\n";
        		kernelCodeWithConst +="	}\n";
        		kernelCodeWithConst +="	OutputArray[gid] = result;\n";
        		kernelCodeWithConst +="}\n";
        	}

        	std::string lbDefines;
        	lbDefines = "#define N " + std::to_string(ninput_items) + "\n";
        	lbDefines += "#define K "+ std::to_string(d_ntaps) + "\n";

        	std::string tmpKernelCode;
        	int requiredConstMemSize = ninput_items*dataSize + d_ntaps*sizeof(float);

        	if (requiredConstMemSize < maxConstMemSize) {
        		tmpKernelCode = lbDefines + kernelCodeWithConst;
        		if (debugMode)
            		std::cout << "OpenCL INFO: Filter is using kernel code with faster constant memory." << std::endl;
        	}
        	else {
        		tmpKernelCode = lbDefines + kernelCode;
        		if (debugMode)
            		std::cout << "OpenCL INFO: The number of taps exceeds OpenCL constant memory space for your device.  Filter is using slower kernel code with filter copy to local memory." << std::endl;
        	}

        	GRCLBase::CompileKernel((const char *)tmpKernelCode.c_str(),(const char *)fnName.c_str());
        	int imaxItems=gr::block::max_noutput_items();
        	if (imaxItems==0)
        		imaxItems=8192;

        	// So the amount of items we have for the constant space is:
        	// maxConstMemSize >= noutput_items*datasize + n_taps*sizeof(float)
        	// Since we can't control the n_taps, we can control the noutput_items.
        	maxConstItems = (int)((float)(maxConstMemSize - d_taps.size()*sizeof(float))/(float)dataSize) - d_taps.size();

        	if (maxConstItems < 0) {
        		std::cout << "OpenCL Filter Error: the number of taps is too large for efficient processing.  Try making the filter transition at least 15% of the cutoff frequency." << std::endl;

        	}

        	if (debugMode) {
        		std::cout << "OpenCL filter: max input items to use constant memory with the specified " << d_taps.size() << " taps: " << maxConstItems << std::endl;
        	}

        	if (maxConstItems < imaxItems) {
        		imaxItems = maxConstItems;
        	}

        	setBufferLength(imaxItems);
        	set_max_noutput_items(imaxItems);
    	}
    	else {
    		// Frequency Domain

        	setBufferLength(ninput_items);
        	// Control how the scheduler feeds us data.
        	set_output_multiple(d_fftsize);
    	}


        if (!USE_TIME_DOMAIN) {
            int err;
            /* Setup clFFT. */
            if (!hasInitialized_clFFT) {
            	hasInitialized_clFFT = true;
                clfftSetupData fftSetup;
                err = clfftInitSetupData(&fftSetup);
                err = clfftSetup(&fftSetup);
            }
            else {
            	// delete the old plan
                err = clfftDestroyPlan( &planHandle );
            }

            size_t clLengths[1];
            clLengths[0]=(size_t)d_fftsize;  // calculated in set_taps then compute_sizes

            err = clfftCreateDefaultPlan(&planHandle, (*context)(), dim, clLengths);

            /* Set plan parameters. */
            err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE);

            if (dataType==DTYPE_COMPLEX) {
                err = clfftSetLayout(planHandle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
            }
            else {
                err = clfftSetLayout(planHandle, CLFFT_REAL, CLFFT_REAL);
            }

           	clfftSetPlanScale(planHandle, CLFFT_BACKWARD, 1.0f); // By default reverse plan scale is 1/N

            err = clfftSetResultLocation(planHandle, CLFFT_OUTOFPLACE);

            /* Bake the plan. */
            err = clfftBakePlan(planHandle, 1, &(*queue)(), NULL, NULL);
        }
}

void clFilter_impl::setBufferLength(int numItems) {
	if (aBuffer)
		delete aBuffer;

	if (bBuffer)
		delete bBuffer;

	if (cBuffer)
		delete cBuffer;

	if (zeroBuff)
		delete[] zeroBuff;

	if (tmpFFTBuff)
		delete[] tmpFFTBuff;


	if (USE_TIME_DOMAIN) {
		aBuffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,
			(numItems+d_ntaps)*dataSize);

		zeroBuff=new char[paddedBufferLengthBytes];
		memset(zeroBuff,0x00,paddedBufferLengthBytes);
		// This is our tap buffer.
		bBuffer = new cl::Buffer(
			*context,
			CL_MEM_READ_ONLY,
			d_ntaps*sizeof(float));

        queue->enqueueWriteBuffer(*bBuffer,CL_TRUE,0,d_ntaps*sizeof(float),&(d_taps[0]));

		cBuffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,
			numItems*dataSize);

		tmpFFTBuff = new char[paddedBufferLengthBytes];
	}
	else {
        if (ifftBuff) {
			if (dataType == DTYPE_COMPLEX) {
				delete[] (gr_complex *)ifftBuff;
			}
			else {
				delete [] (float *)ifftBuff;
			}
        }

        zeroBuff=new char[(d_fftsize-d_nsamples)*dataSize];
		memset(zeroBuff,0x00,(d_fftsize-d_nsamples)*dataSize);

		aBuffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,  // needs to be read/write for clFFT
			d_fftsize*dataSize);

		cBuffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,
			d_fftsize*dataSize);

		tmpFFTBuff = new char[d_fftsize*dataSize];
        if (dataType == DTYPE_COMPLEX) {
        	ifftBuff = new gr_complex[d_fftsize];
        }
        else {
        	ifftBuff = new float[d_fftsize];
        }

	}
	curBufferSize = numItems;
}

/*
 * determines d_ntaps, d_nsamples, d_fftsize, d_xformed_taps
 */
int
clFilter_impl::set_taps(const std::vector<float> &taps)
{
	int i = 0;
	d_taps = taps;
	compute_sizes(taps.size());
	float scale = 1.0 / d_fftsize;

	// tailsize is d_ntaps - 1 which is the remaining data on the tap filter.
	// Input data = fft_size + d_ntaps - 1.   d_ntaps - 1 is the tail that should get zero'd out.
	if (dataType==DTYPE_COMPLEX) {
		d_tail.resize(tailsize());
		for(i = 0; i < tailsize(); i++)
		  d_tail[i] = 0;

		gr_complex *in = d_fwdfft->get_inbuf();
		gr_complex *out = d_fwdfft->get_outbuf();

		// Compute forward xform of taps.
		// Copy taps into first ntaps slots, then pad with zeros
		for(i = 0; i < d_ntaps; i++)
		  in[i] = gr_complex(taps[i] * scale, 0.0f);

		for(i=d_ntaps; i < d_fftsize; i++)
		  in[i] = gr_complex(0.0f, 0.0f);

		d_fwdfft->execute();		// do the xform

		// now copy output to d_xformed_taps
		for(i = 0; i < d_fftsize; i++)
		  d_xformed_taps[i] = out[i];
	}
	else {
		// DTYPE_FLOAT

		d_tail_float.resize(tailsize());
		for(i = 0; i < tailsize(); i++)
		  d_tail_float[i] = 0;

		float tmpTaps[d_ntaps];

		// Have to make sure we have d_fftsize inputs for the filter.
		for (i=0;i<d_ntaps;i++)
			tmpTaps[i] = taps[i] * scale;

		for(i=d_ntaps; i < d_fftsize; i++)
			tmpTaps[i] = 0.0f;

		if (!USE_TIME_DOMAIN) {
	    	cl::Buffer tapInput(
	            *context,
	            CL_MEM_READ_ONLY,
				d_fftsize*dataSize,&tmpTaps[0]);

	        cl::Buffer tapOutput(
	            *context,
	            CL_MEM_READ_WRITE,
				d_fftsize*dataSize);

	        int err;
	        // Execute the plan.
	       	err = clfftEnqueueTransform(planHandle, CLFFT_FORWARD, 1, &(*queue)(), 0, NULL, NULL, &(tapInput()), &(tapOutput()), NULL);

	        // Wait for calculations to be finished.
	        err = clFinish((*queue)());

	        // and outputting FFTSize samples

	        // Fetch results of calculations.
	    	queue->enqueueReadBuffer(tapOutput,CL_TRUE,0,d_fftsize*dataSize,(void *)transformedTaps_float);
		}
	}

	return d_nsamples;
}

// determine and set d_ntaps, d_nsamples, d_fftsize
void
clFilter_impl::compute_sizes(int ntaps)
{
	int old_fftsize = d_fftsize;
	d_ntaps = ntaps;
	d_fftsize = (int) (2 * pow(2.0, ceil(log(double(ntaps)) / log(2.0))));
	d_nsamples = d_fftsize - d_ntaps + 1;
	/*
	if(VERBOSE) {
	  std::cerr << "fft_filter_ccf: ntaps = " << d_ntaps
			<< " fftsize = " << d_fftsize
			<< " nsamples = " << d_nsamples << std::endl;
	}
	*/
	// compute new plans
	if(d_fftsize != old_fftsize) {
		try {
			if (d_fwdfft != NULL) {
				delete d_fwdfft;
				d_fwdfft=NULL;
			}
		}
		catch (...) {
			std::cout << "FFT_Filter_ccf::compute_sizes: exception deleting d_fwdfft" << std::endl;
		}

		try {
			if (d_invfft) {
				delete d_invfft;
				d_invfft = NULL;
			}
		}
		catch (...) {
			std::cout << "FFT_Filter_ccf::compute_sizes: exception deleting d_invfft" << std::endl;
		}

		try {
			if(d_xformed_taps != NULL) {
				try {
				// volk_free(d_xformed_taps);
					delete[] d_xformed_taps;
					d_xformed_taps = NULL;
				}
				catch (...) {

				}
			}
		}
		catch (...) {
			std::cout << "FFT_Filter_ccf::compute_sizes: exception volk_free(d_xformed_taps)" << std::endl;
		}

		if (dataType==DTYPE_COMPLEX) {
			d_fwdfft = new fft_complex(d_fftsize, true, d_nthreads);
			d_invfft = new fft_complex(d_fftsize, false, d_nthreads);

			// d_xformed_taps = (gr_complex*)volk_malloc(sizeof(gr_complex)*d_fftsize,volk_get_alignment());
			d_xformed_taps = new gr_complex[d_fftsize];
		}
		else {
			// DTYPE_FLOAT
			if (transformedTaps_float)
				delete[] transformedTaps_float;

			transformedTaps_float = new float[d_fftsize];
		}
	}
}


int
clFilter_impl::filterGPU(int ninput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items) {
	if (USE_TIME_DOMAIN)
		return filterGPUTimeDomain(ninput_items,input_items,output_items);
	else
		return filterGPUFrequencyDomain(ninput_items,input_items,output_items);

}
    int
	clFilter_impl::filterGPUTimeDomain(int ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {

    	if (ninput_items > curBufferSize) {
    		// This could get expensive if we have to rebuild kernels
    		// in GNURadio min input items and max items should be
    		// set to the same value to ensure consistency.
    		setFilterVariables(ninput_items);
    		if (debugMode) {
    			std::cout << "ninput_items > curBufferSize.  Adjusting buffer to match..." << std::endl;
    		}
    	}

    	int inputBytes=ninput_items*dataSize;
    	// See https://www.altera.com/support/support-resources/design-examples/design-software/opencl/td-fir.html
    	// for reference.  The source code has a PDF describing implementing FIR in FPGA.


        int remaining=(curBufferSize+d_ntaps)*dataSize - inputBytes;

        queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputBytes,(void *)input_items[0]);
        if (remaining > 0)
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

		cl_int err;
		int retVal;

		if (fft_filter_ccf::d_decimation == 1) {
			queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,inputBytes,(void *)output_items[0]);

			// # in=# out. Do it the quick way
			// memcpy((void *)output_items[0],output,ninput_items*dataSize);
			retVal = ninput_items;
		}
		else {

			queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,inputBytes,(void *)tmpFFTBuff);

			// copy results to output buffer and increment for decimation!
			int j=0;
			int i=0;
			while(j < ninput_items) {
				if (dataType==DTYPE_COMPLEX) {
			        gr_complex *out = (gr_complex *)output_items[0];
			        gr_complex *ResultPtr = (gr_complex *)tmpFFTBuff;

					out[i++] = ResultPtr[j];
				}
				else {
			        float *out = (float *)output_items[0];
			        float *ResultPtr = (float *)tmpFFTBuff;

					out[i++] = ResultPtr[j];
				}

				j += fft_filter_ccf::d_decimation;
			}

			retVal = i;
		}

    	return retVal;  // expecting nitems which is ninput_items/decimation
    }

    int
	clFilter_impl::filterGPUFrequencyDomain(int ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	const gr_complex *input = (const gr_complex *) input_items[0];
        gr_complex *output = (gr_complex *) output_items[0];

    	if (!d_fwdfft || !d_invfft)
    		return 0;

    	int dec_ctr = 0;
    	int j = 0;
    	int err;

    	for(int i = 0; i < ninput_items; i += d_nsamples) {
    	  // Move block of data to forward FFT buffer
    	/*
    	  memcpy(d_fwdfft->get_inbuf(), &input[i], d_nsamples * sizeof(gr_complex));

    	  // zero out any data past d_nsamples to fft_size
    	  for(j = d_nsamples; j < d_fftsize; j++)
    		d_fwdfft->get_inbuf()[j] = 0;
    	  // Run the transform
    	  d_fwdfft->execute();	// compute fwd xform
    */

    	  const gr_complex *in = (const gr_complex *) input_items[0];
    	  queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,d_nsamples*dataSize,(void *)&in[i]);
    	  queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,d_nsamples*dataSize,(d_fftsize-d_nsamples)*dataSize,(void *)zeroBuff);
    	  err = clfftEnqueueTransform(planHandle, CLFFT_FORWARD, 1, &(*queue)(), 0, NULL, NULL, &(*aBuffer)(), &(*cBuffer)(), NULL);
    	  err = clFinish((*queue)());

    	  queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,d_fftsize*dataSize,(void *)tmpFFTBuff);

    	  // Get the fwd FFT data out
    //	  gr_complex *a = d_fwdfft->get_outbuf();
    	  gr_complex *a;
    	  a=(gr_complex *)tmpFFTBuff;

    	  gr_complex *b = d_xformed_taps;

    	  // set up the inv FFT buffer to receive the complex multiplied data
    //	  gr_complex *c = d_invfft->get_inbuf();
    	  gr_complex *c;
    	  c=(gr_complex *)ifftBuff;

    	  // THIS Volk Function just does complex multiplication.  c[i]=a[i]*b[i] in the complex domain.
    	  volk_32fc_x2_multiply_32fc_a(c, a, b, d_fftsize);

    	  memcpy(d_invfft->get_inbuf(),(void *)c,d_fftsize*dataSize);

    	  // Run the inverse FFT
    	  //  d_invfft->execute();	// compute inv xform

    	  queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,d_fftsize*dataSize,(void *)ifftBuff);
    	  err = clfftEnqueueTransform(planHandle, CLFFT_BACKWARD, 1, &(*queue)(), 0, NULL, NULL, &(*aBuffer)(), &(*cBuffer)(), NULL);
    	  err = clFinish((*queue)());
    	  queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,d_fftsize*dataSize,(void *)tmpFFTBuff);

      	  gr_complex *outdata;
    	  outdata=(gr_complex *)tmpFFTBuff;
      	  // outdata = (gr_complex *)d_invfft->get_outbuf();

    	  // add in the overlapping tail
    	  for(j = 0; j < tailsize(); j++)
    		outdata[j] += d_tail[j];

    	  // copy d_nsamples to output buffer and increment for decimation!

    	  j = dec_ctr;
    	  while(j < d_nsamples) {
    		*output++ = outdata[j];
    		j += decimation();
    	  }
    	  dec_ctr = (j - d_nsamples);

    	  // stash the tail
    	  memcpy(&d_tail[0], outdata + d_nsamples,
    		 tailsize() * sizeof(gr_complex));
    	}

    	return ninput_items;
    }


    int clFilter_impl::testCPU(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return filterCPU(noutput_items,input_items,output_items);
    }

    int clFilter_impl::testOpenCL(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return filterGPU(noutput_items,input_items,output_items);
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
        	// GNURadio filter code
        	// set_taps sets d_nsamples so changed this line.
        	d_nsamples = fft_filter_ccf::set_taps(d_new_taps);
			d_updated = false;
			set_output_multiple(d_nsamples);

			// Additions for our filter
			setFilterVariables(ninput_items);

			prevTaps = d_ntaps;
			prevInputLength = ninput_items;

			return 0;
        }
        // filterCPU(noutput_items, input_items,output_items);

        filterGPU(ninput_items,input_items,output_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace clenabled */
} /* namespace gr */

