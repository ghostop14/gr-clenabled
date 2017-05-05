/* -*- c++ -*- */
/* 
 * Copyright 2017 ghostop14.
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
            int nthreads,int setDebug, bool use_time)
    {
      	if (setDebug == 1) {
            return gnuradio::get_initial_sptr
              (new clFilter_impl(openclPlatform,devSelector,platformId,devId,decimation,taps,nthreads,true,use_time));
      	}
      	else {
            return gnuradio::get_initial_sptr
              (new clFilter_impl(openclPlatform,devSelector,platformId,devId,decimation,taps,nthreads,false,use_time));
      	}
    }

    // NOTE: ONLY COMPLEX IS SUPPORTED BY THE XML GRC BLCOK RIGHT NOW.
    // IF THIS GETS UPDATED TO SUPPORT FLOATS TOO, iDataSize would need to be set up like in MathOp

    clFilter_impl::clFilter_impl(int openclPlatform,int devSelector,int platformId, int devId, int decimation, const std::vector<float> &taps,int nthreads,bool setDebug,bool bUseTimeDomain)
      : gr::sync_decimator("clFilter",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)),decimation),
			  GRCLBase(DTYPE_COMPLEX, sizeof(gr_complex),openclPlatform,devSelector,platformId,devId, setDebug)
    {
        d_fir = new fft_filter_ccf(decimation,taps,nthreads);

    	// -------------------------------------------------------------
    	// Set up depending on freq or time-domain filter
    	USE_TIME_DOMAIN = bUseTimeDomain;

    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

    	if (USE_TIME_DOMAIN) {
        	// Buffer sizes need to match up with blocks based on the number of taps...
        	// This matches up with FFT Size
        	prevInputLength = (int) (2 * pow(2.0, ceil(log(double(taps.size())) / log(2.0))));;

        	// So the amount of items we have for the constant space is:
        	// maxConstMemSize >= (noutput_items+n_taps)*datasize + n_taps*sizeof(float)
        	// Since we can't control the n_taps, we can control the noutput_items.
        	maxConstItems = (int)((float)(maxConstMemSize - d_fir->ntaps()*sizeof(float))/(float)dataSize) - d_fir->ntaps();

        	setTimeDomainFilterVariables(imaxItems);
    	}
    	else {
    		setFreqDomainFilterVariables(imaxItems);
    	}

    	// -------------------------------------------------------------
    	// straight from fir_filter_XXX_impl.cc.t
        d_updated = false;
        set_history(d_fir->ntaps());

        const int alignment_multiple = volk_get_alignment() / sizeof(float);
        set_alignment(std::max(1, alignment_multiple));
    	// -------------------------------------------------------------
}

void clFilter_impl::setFreqDomainFilterVariables(int ninput_items) {

	// -------------------------------------------------------------
	// Set up the FFT's
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
    clLengths[0]=(size_t)d_fir->d_fftsize;  // calculated in set_taps then compute_sizes

    err = clfftCreateDefaultPlan(&planHandle, (*context)(), dim, clLengths);

    /* Set plan parameters. */
    err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE);

    if (dataType==DTYPE_COMPLEX) {
        err = clfftSetLayout(planHandle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
    }
    else {
        err = clfftSetLayout(planHandle, CLFFT_REAL, CLFFT_REAL);
    }

   	clfftSetPlanScale(planHandle, CLFFT_FORWARD, 1.0f); // By default reverse plan scale is 1/N
   	clfftSetPlanScale(planHandle, CLFFT_BACKWARD, 1.0f); // By default reverse plan scale is 1/N

    err = clfftSetResultLocation(planHandle, CLFFT_OUTOFPLACE);

    /* Bake the plan. */
    err = clfftBakePlan(planHandle, 1, &(*queue)(), NULL, NULL);

    // -------------------------------------------------------------

    // Set up memory buffers
	setBufferLength(ninput_items);
}

void clFilter_impl::setTimeDomainFilterVariables(int ninput_items) {

	// remember sethistory(N) saves N-1.  sethistory(1) disables history.
	// Therefore sethistory(d_fir->ntaps()) saves paddingLength samples since paddingLenght = ntaps - 1

	paddingLength = d_fir->ntaps() - 1;
	paddingBytes = dataSize*paddingLength;

	resultLengthPoints = ninput_items + d_fir->ntaps() - 1;

	inputLengthBytes = ninput_items*dataSize;
	paddedBufferLengthBytes=resultLengthPoints*dataSize;

	filterLengthBytes=d_fir->ntaps() * sizeof(float);

	kernelCode = "";
	kernelCodeWithConst = "";
    std::string fnName = "";

    hasDoublePrecisionSupport = false;
    hasSingleFMASupport = false;

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
		if (hasSingleFMASupport) {
			kernelCode +="		result.real = fma(FilterArray[K-1-i],InputArray[gid+i].real,result.real);\n";
			kernelCode +="		result.imag = fma(FilterArray[K-1-i],InputArray[gid+i].imag,result.imag);\n";
		}
		else {
			kernelCode +="		result.real += FilterArray[K-1-i]*InputArray[gid+i].real;\n";
			kernelCode +="		result.imag += FilterArray[K-1-i]*InputArray[gid+i].imag;\n";
		}
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
		kernelCodeWithConst +="( __global const SComplex * restrict InputArray, // Length N\n";
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
		if (hasSingleFMASupport) {
			kernelCodeWithConst +="		result.real = fma(FilterArray[K-1-i],InputArray[gid+i].real,result.real);\n";
			kernelCodeWithConst +="		result.imag = fma(FilterArray[K-1-i],InputArray[gid+i].imag,result.imag);\n";
		}
		else {
			kernelCodeWithConst +="		result.real += FilterArray[K-1-i]*InputArray[gid+i].real;\n";
			kernelCodeWithConst +="		result.imag += FilterArray[K-1-i]*InputArray[gid+i].imag;\n";
		}
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
		kernelCode +="__global const float * restrict FilterArray, // Length K\n";
		kernelCode +="__global float *restrict OutputArray // Length N+K-1\n";
		kernelCode +=")\n";
		kernelCode +="{\n";
		kernelCode +="  size_t gid=get_global_id(0);\n";
		kernelCode +="	// Perform Compute\n";
		kernelCode +="	float result=0.0f;\n";
		kernelCode +="	for (int i=0; i<K; i++) {\n";
		if (hasSingleFMASupport) {
			kernelCode +="		result = fma(FilterArray[K-1-i],InputArray[gid+i],result);\n";
		}
		else {
			kernelCode +="		result = FilterArray[K-1-i]*InputArray[gid+i] + result;\n";
		}
		kernelCode +="	}\n";
		kernelCode +="	OutputArray[gid] = result;\n";
		kernelCode +="}\n";

		kernelCodeWithConst +="__kernel void td_FIR_float\n";
		kernelCodeWithConst +="( __global const float * restrict InputArray, // Length N\n";
		kernelCodeWithConst +="__constant float * FilterArray, // Length K\n";
		kernelCodeWithConst +="__global float *restrict OutputArray // Length N+K-1\n";
		kernelCodeWithConst +=")\n";
		kernelCodeWithConst +="{\n";
		kernelCodeWithConst +="  size_t gid=get_global_id(0);\n";
		kernelCodeWithConst +="	// Perform Compute\n";
		kernelCodeWithConst +="	float result=0.0f;\n";
		kernelCodeWithConst +="	for (int i=0; i<K; i++) {\n";
		if (hasSingleFMASupport) {
			kernelCodeWithConst +="		result = fma(FilterArray[K-1-i],InputArray[gid+i],result);\n";
		}
		else {
			kernelCodeWithConst +="		result = FilterArray[K-1-i]*InputArray[gid+i] + result;\n";
		}
		kernelCodeWithConst +="	}\n";
		kernelCodeWithConst +="	OutputArray[gid] = result;\n";
		kernelCodeWithConst +="}\n";
	}

	std::string lbDefines;
	lbDefines = "#define K "+ std::to_string(d_fir->ntaps()) + "\n";

	std::string tmpKernelCode;
	float tapConstMemUsage = maxConstMemSize - d_fir->d_ntaps*sizeof(float);
	bool useConst;

	if (tapConstMemUsage > 0)
		useConst = true;
	else
		useConst = false;

	if (useConst) {
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

	if (debugMode) {
		std::cout << "OpenCL filter: max input items to use constant memory with the specified " << d_fir->d_ntaps << " taps: " << maxConstItems << std::endl;
	}

	setBufferLength(ninput_items);
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
			(numItems+d_fir->ntaps())*dataSize);

		zeroBuff=new char[paddedBufferLengthBytes];
		memset(zeroBuff,0x00,paddedBufferLengthBytes);
		// This is our tap buffer.
		bBuffer = new cl::Buffer(
			*context,
			CL_MEM_READ_ONLY,
			d_fir->ntaps()*sizeof(float));

        queue->enqueueWriteBuffer(*bBuffer,CL_TRUE,0,d_fir->d_ntaps*sizeof(float),&(d_fir->d_taps[0]));

		cBuffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,
			numItems*dataSize);

		tmpFFTBuff = new char[paddedBufferLengthBytes];
	}
	else {
		// Frequency Domain.
        if (ifftBuff) {
			if (dataType == DTYPE_COMPLEX) {
				delete[] (gr_complex *)ifftBuff;
			}
			else {
				delete [] (float *)ifftBuff;
			}
        }

        zeroBuff=new char[(d_fir->d_fftsize-d_fir->d_nsamples)*dataSize];
		memset(zeroBuff,0x00,(d_fir->d_fftsize-d_fir->d_nsamples)*dataSize);

		// std::cout << "d_fir->d_nsamples: " << d_fir->d_nsamples << " d_fir->d_fftsize: " << d_fir->d_fftsize << std::endl;

		aBuffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,  // needs to be read/write for clFFT
			d_fir->d_fftsize*dataSize);

		cBuffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,
			d_fir->d_fftsize*dataSize);

		tmpFFTBuff = new char[d_fir->d_fftsize*dataSize];
        if (dataType == DTYPE_COMPLEX) {
        	ifftBuff = new gr_complex[d_fir->d_fftsize];
        }
        else {
        	ifftBuff = new float[d_fir->d_fftsize];
        }

	}
	curBufferSize = numItems;
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
    	if (d_fir) {
    		delete d_fir;
    		d_fir = NULL;
    	}

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

    std::vector<float> clFilter_impl::taps() const {
    	return d_fir->taps();
    }

    void clFilter_impl::set_nthreads(int n) {
    	d_fir->set_nthreads(n);
    }
    /*
     * The private constructor
     */


int
clFilter_impl::set_taps(const std::vector<float> &taps)
{
	// Protect context from switching
    gr::thread::scoped_lock guard(d_mutex);

    d_fir->set_taps(taps);
    d_updated = true;

	int imaxItems=gr::block::max_noutput_items();
	if (imaxItems==0)
		imaxItems=8192;

    if (USE_TIME_DOMAIN) {
    	// Buffer sizes need to match up with blocks based on the number of taps...
    	// This matches up with FFT Size
    	prevInputLength = (int) (2 * pow(2.0, ceil(log(double(d_fir->ntaps())) / log(2.0))));;

    	// So the amount of items we have for the constant space is:
    	// maxConstMemSize >= (noutput_items+n_taps)*datasize + n_taps*sizeof(float)
    	// Since we can't control the n_taps, we can control the noutput_items.
    	maxConstItems = (int)((float)(maxConstMemSize - d_fir->ntaps()*sizeof(float))/(float)dataSize) - d_fir->ntaps();

    	setTimeDomainFilterVariables(imaxItems);
    }
    else {
    	setFreqDomainFilterVariables(imaxItems);
    }

    return d_fir->ntaps();
}

void
clFilter_impl::set_taps2(const std::vector<float> &taps) {
    gr::thread::scoped_lock l(d_setlock);
    d_updated = true;
    d_fir->set_taps(taps);

    // FFT and tap size may have changed.  Recalc
	int imaxItems=gr::block::max_noutput_items();
	if (imaxItems==0)
		imaxItems=8192;

    if (USE_TIME_DOMAIN) {
    	// Buffer sizes need to match up with blocks based on the number of taps...
    	// This matches up with FFT Size
    	prevInputLength = (int) (2 * pow(2.0, ceil(log(double(d_fir->ntaps())) / log(2.0))));;

    	// So the amount of items we have for the constant space is:
    	// maxConstMemSize >= (noutput_items+n_taps)*datasize + n_taps*sizeof(float)
    	// Since we can't control the n_taps, we can control the noutput_items.
    	maxConstItems = (int)((float)(maxConstMemSize - d_fir->ntaps()*sizeof(float))/(float)dataSize) - d_fir->ntaps();

    	setTimeDomainFilterVariables(imaxItems);
    }
    else {
    	setFreqDomainFilterVariables(imaxItems);
    }
}

void
clFilter_impl::TestNotifyNewFilter(int noutput_items) {
	// This is only used in our test app since work isn't called.
    if (d_updated){
    	// set_taps sets d_fir->d_nsamples so changed this line.
		d_updated = false;

		if (USE_TIME_DOMAIN) {
			setTimeDomainFilterVariables(noutput_items);
			prevInputLength = noutput_items;
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
    		setTimeDomainFilterVariables(ninput_items);
    		if (debugMode) {
    			std::cout << "ninput_items > curBufferSize.  Adjusting buffer to match..." << std::endl;
    		}
    	}

    	int inputBytes=ninput_items*dataSize;
    	// See https://www.altera.com/support/support-resources/design-examples/design-software/opencl/td-fir.html
    	// for reference.  The source code has a PDF describing implementing FIR in FPGA.


        int remaining=(curBufferSize+d_fir->ntaps())*dataSize - inputBytes;


    	// Protect context from switching
        gr::thread::scoped_lock guard(d_mutex);

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

		if (d_fir->d_decimation == 1) {
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

				j += d_fir->d_decimation;
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

    	if (!d_fir->d_fwdfft || !d_fir->d_invfft)
    		return 0;

    	int dec_ctr = 0;
    	int j = 0;
    	int k;
    	int err;
    	const gr_complex *in = (const gr_complex *) input_items[0];

  	  	ninput_items = ninput_items * d_fir->d_decimation;

    	for(int i = 0; i < ninput_items; i += d_fir->d_nsamples) {
    	  // Move block of data to forward FFT buffer
    	/*
    	  memcpy(d_fwdfft->get_inbuf(), &input[i], d_fir->d_nsamples * sizeof(gr_complex));

    	  // zero out any data past d_fir->d_nsamples to fft_size
    	  for(j = d_fir->d_nsamples; j < d_fir->d_fftsize; j++)
    		d_fwdfft->get_inbuf()[j] = 0;
    	  // Run the transform
    	  d_fwdfft->execute();	// compute fwd xform
    */

    	  queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,d_fir->d_nsamples*dataSize,(void *)&in[i]);
    	  queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,d_fir->d_nsamples*dataSize,(d_fir->d_fftsize-d_fir->d_nsamples)*dataSize,(void *)zeroBuff);
    	  err = clfftEnqueueTransform(planHandle, CLFFT_FORWARD, 1, &(*queue)(), 0, NULL, NULL, &(*aBuffer)(), &(*cBuffer)(), NULL);
    	  err = clFinish((*queue)());

    	  // Get the fwd FFT data out
    //	  gr_complex *a = d_fwdfft->get_outbuf();
    	  queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,d_fir->d_fftsize*dataSize,(void *)tmpFFTBuff);
    	  gr_complex *a;
    	  a=(gr_complex *)tmpFFTBuff;

    	  gr_complex *b = d_fir->d_xformed_taps;

    	  // set up the inv FFT buffer to receive the complex multiplied data
    //	  gr_complex *c = d_invfft->get_inbuf();
    	  gr_complex *c;
    	  c=(gr_complex *)ifftBuff;

    	  // Original volk call.  Might as well use SIMD / SSE
    	  volk_32fc_x2_multiply_32fc_a(c, a, b, d_fir->d_fftsize);
    	  /*
    	  for (k=0;k<d_fir->d_fftsize;k++) {
    		  c[k] = a[k] * b[k];
    	  }
    	  */

 //    	  memcpy(d_invfft->get_inbuf(),(void *)c,d_fir->d_fftsize*dataSize);
    	  queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,d_fir->d_fftsize*dataSize,(void *)ifftBuff);

    	  // Run the inverse FFT
    	  //  d_invfft->execute();	// compute inv xform
    	  err = clfftEnqueueTransform(planHandle, CLFFT_BACKWARD, 1, &(*queue)(), 0, NULL, NULL, &(*aBuffer)(), &(*cBuffer)(), NULL);
    	  err = clFinish((*queue)());

      	  // outdata = (gr_complex *)d_invfft->get_outbuf();
    	  queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,d_fir->d_fftsize*dataSize,(void *)tmpFFTBuff);
      	  gr_complex *outdata;
    	  outdata=(gr_complex *)tmpFFTBuff;

    	  // ------------------------------------------------------------------
    	  // Unmodified GNURadio flow
    	  // add in the overlapping tail
    	  for(j = 0; j < d_fir->tailsize(); j++)
    		outdata[j] += d_fir->d_tail[j];

    	  // copy d_fir->d_nsamples to output buffer and increment for decimation!
    	  j = dec_ctr;
    	  while(j < d_fir->d_nsamples) {
    		*output++ = outdata[j];
    		j += decimation();
    	  }
    	  dec_ctr = (j - d_fir->d_nsamples);

    	  // ------------------------------------------------------------------
    	  // stash the tail
    	  // memcpy(&d_tail[0], outdata + d_fir->d_nsamples,tailsize() * sizeof(gr_complex));
    	  memcpy(&d_fir->d_tail[0], outdata + d_fir->d_nsamples,d_fir->tailsize() * dataSize);
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
        retVal = d_fir->filter(noutput_items,in,out);
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
    		for (int i=0; i<d_fir->ntaps(); i++) {
    			tapId=d_fir->ntaps()-1-i;
    			result.real += d_fir->d_taps[tapId]*in[N+i].real();
    			result.imag += d_fir->d_taps[tapId]*in[N+i].imag();
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
    	// Protect context from switching
        gr::thread::scoped_lock guard(d_mutex);

    	int ninput_items = noutput_items * d_fir->d_decimation;
        if (d_updated){
        	// GNURadio filter code
        	// set_taps sets d_fir->d_nsamples so changed this line.
			d_updated = false;

			// Additions for our filter
			if (USE_TIME_DOMAIN) {
				setTimeDomainFilterVariables(ninput_items);
				prevInputLength = ninput_items;
			}

			set_history(d_fir->ntaps());
			return 0;
        }

    	if (debugMode && CLPRINT_NITEMS)
    		std::cout << "clFilter_impl ninput_items: " << ninput_items << std::endl;

        if (USE_TIME_DOMAIN)
        	filterCPU(ninput_items, input_items,output_items);
        else
        	filterGPU(ninput_items,input_items,output_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace clenabled */
} /* namespace gr */

