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
#include "clQuadratureDemod_impl.h"
#include "fast_atan2f.h"

namespace gr {
  namespace clenabled {

    clQuadratureDemod::sptr
    clQuadratureDemod::make(float gain, int openCLPlatformType,int devSelector,int platformId, int devId,int setDebug)
    {
      if (setDebug == 1)
		  return gnuradio::get_initial_sptr
			(new clQuadratureDemod_impl(gain, openCLPlatformType,devSelector,platformId,devId,true));
      else
		  return gnuradio::get_initial_sptr
			(new clQuadratureDemod_impl(gain, openCLPlatformType,devSelector,platformId,devId,false));
    }

    /*
     * The private constructor
     */
    clQuadratureDemod_impl::clQuadratureDemod_impl(float gain, int openCLPlatformType,int devSelector,int platformId, int devId, bool setDebug)
      : gr::sync_block("clQuadratureDemod",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(float))),
	  	  	  GRCLBase(DTYPE_COMPLEX, sizeof(gr_complex),openCLPlatformType,devSelector,platformId,devId,setDebug)
   {
    	f_gain = gain;

    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

		setBufferLength(imaxItems+1);  // have 1 history item there.  set_history(N) saves N-1 items.

        // And finally optimize the data we get based on the preferred workgroup size.
        // Note: We can't do this until the kernel is compiled and since it's in the block class
        // it has to be done here.
        // Note: for CPU's adjusting the workgroup size away from 1 seems to decrease performance.
        // For GPU's setting it to the preferred size seems to have the best performance.
		try {
			if (contextType != CL_DEVICE_TYPE_CPU) {
				gr::block::set_output_multiple(preferredWorkGroupSizeMultiple);
			}
			else {
				// Keep the IO somewhat aligned
				gr::block::set_output_multiple(32);
			}
		}
        catch (...) {

        }

        // we need to look at the previous value.
        // If you set the history to 'N', this means you always have the last (N-1)
        // input values kept in your buffer.
        set_history(2);
}

    void clQuadratureDemod_impl::buildKernel(int numItems) {
    	maxConstItems = (int)((float)maxConstMemSize / ((float)dataSize));

    	bool useConst;
    	if (numItems > maxConstItems)
    		useConst = false;
    	else
    		useConst = true;
/*
		if (debugMode) {
			if (useConst)
				std::cout << "OpenCL INFO: QuadratureDemod building kernel with __constant params..." << std::endl;
			else
				std::cout << "OpenCL INFO: QuadratureDemod - too many items for constant memory.  Building kernel with __global params..." << std::endl;
		}
*/

        // Now we set up our OpenCL kernel
        std::string srcStdStr="";
        std::string fnName = "quadDemod";

        // Debug Code
        // srcStdStr += "#pragma OPENCL EXTENSION cl_intel_printf : enable\n";

        if (f_gain != 1.0) {
        	srcStdStr += "#define GAIN " + std::to_string(f_gain) + "\n";
        }

    	srcStdStr += "struct ComplexStruct {\n";
    	srcStdStr += "float real;\n";
    	srcStdStr += "float imag; };\n";
    	srcStdStr += "typedef struct ComplexStruct SComplex;\n";

    	// Has to be in global memory because of the +1 on the atan call.  constant memory doesn't like it.
/*
    	if (useConst)
    		srcStdStr += "__kernel void quadDemod(__constant SComplex * a, __global float * restrict c) {\n";
    	else
    		srcStdStr += "__kernel void quadDemod(__global SComplex * restrict a, __global float * restrict c) {\n";
    */
		srcStdStr += "__kernel void quadDemod(__global SComplex * restrict a, __global float * restrict c) {\n";

    	srcStdStr += "    size_t index =  get_global_id(0);\n";
    	if (hasDoublePrecisionSupport) {
        	srcStdStr += "    double a_r=a[index+1].real;\n";
        	srcStdStr += "    double a_i=a[index+1].imag;\n";
        	srcStdStr += "    double b_r=a[index].real;\n";
        	srcStdStr += "    double b_i=-1.0 * a[index].imag;\n";

    		if (hasDoubleFMASupport) {
        		srcStdStr += "	  double multCCreal = fma(a_r,b_r,-(a_i*b_i));\n";
        		srcStdStr += "	  double multCCimag = fma(a_r,b_i,a_i*b_r);\n";
    		}
    		else {
            	srcStdStr += "    double multCCreal = (a_r * b_r) - (a_i*b_i);\n";
            	srcStdStr += "    double multCCimag = (a_r * b_i) + (a_i * b_r);\n";
    		}

            if (f_gain != 1.0)
            	srcStdStr += "    c[index] = (float)(GAIN * atan2(multCCimag,multCCreal));\n";
            else
            	srcStdStr += "    c[index] = (float)(atan2(multCCimag,multCCreal));\n";
    /*// Debug Code
        	srcStdStr += "    if (a_r != 0.0 || a_i != 0.0) {\n";
        	srcStdStr += "       printf(\"Kernel: input real=%f input imag=%f\\n\",a_r,a_i);\n";
        	srcStdStr += "       printf(\"Kernel: multCC real=%f multCC imag=%f\\n\",multCCreal,multCCimag);\n";
        	srcStdStr += "       printf(\"Kernel: atan2=%f\\n\",c[index]);\n";
        	srcStdStr += "    }\n";
    */
        	srcStdStr += "}\n";
    	}
    	else {
        	srcStdStr += "    float a_r=a[index+1].real;\n";
        	srcStdStr += "    float a_i=a[index+1].imag;\n";
        	srcStdStr += "    float b_r=a[index].real;\n";
        	srcStdStr += "    float b_i=-1.0 * a[index].imag;\n";

    		if (hasSingleFMASupport) {
        		srcStdStr += "	  float multCCreal = fma(a_r,b_r,-(a_i*b_i));\n";
        		srcStdStr += "	  float multCCimag = fma(a_r,b_i,a_i*b_r);\n";
    		}
    		else {
            	srcStdStr += "    float multCCreal = (a_r * b_r) - (a_i*b_i);\n";
            	srcStdStr += "    float multCCimag = (a_r * b_i) + (a_i * b_r);\n";
    		}

        	if (f_gain != 1.0)
            	srcStdStr += "    c[index] = GAIN * atan2(multCCimag,multCCreal);\n";
            else
            	srcStdStr += "    c[index] = atan2(multCCimag,multCCreal);\n";
    /*// Debug Code
        	srcStdStr += "    if (a_r != 0.0 || a_i != 0.0) {\n";
        	srcStdStr += "       printf(\"Kernel: input real=%f input imag=%f\\n\",a_r,a_i);\n";
        	srcStdStr += "       printf(\"Kernel: multCC real=%f multCC imag=%f\\n\",multCCreal,multCCimag);\n";
        	srcStdStr += "       printf(\"Kernel: atan2=%f\\n\",c[index]);\n";
        	srcStdStr += "    }\n";
    */
        	srcStdStr += "}\n";
    	}

        GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());

    }

    void clQuadratureDemod_impl::setBufferLength(int numItems) {
    	if (aBuffer)
    		delete aBuffer;

    	if (cBuffer)
    		delete cBuffer;

    	aBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_ONLY,
			(numItems+1) * sizeof(gr_complex));  // complex conj is of index+1.

    	/*
    	 * Didn't understand set_history.  Don't need to do this.
    	// Zero out the last entry
    	float zeroBuff[2];
    	zeroBuff[0] = zeroBuff[1] = 0.0;

    	queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,numItems*sizeof(gr_complex),sizeof(gr_complex),(void *)&zeroBuff[0]);
		*/

        cBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_WRITE,
			numItems * sizeof(float));

        buildKernel(numItems);
        curBufferSize=numItems;
    }

    /*
     * Our virtual destructor.
     */
    clQuadratureDemod_impl::~clQuadratureDemod_impl()
    {
    	if (curBufferSize > 0)
    		stop();
    }

    bool clQuadratureDemod_impl::stop() {
    	curBufferSize = 0;

    	if (aBuffer) {
    		delete aBuffer;
    		aBuffer = NULL;
    	}

    	if (cBuffer) {
    		delete cBuffer;
    		cBuffer = NULL;
    	}

    	return GRCLBase::stop();
    }


    /*
     * *********************************************
     * 			TestCPU
     * *********************************************
     */
    int clQuadratureDemod_impl::testCPU(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    	{
        gr_complex *in = (gr_complex*)input_items[0];
        float *out = (float*)output_items[0];

        std::vector<gr_complex> tmp(noutput_items);
        volk_32fc_x2_multiply_conjugate_32fc(&tmp[0], &in[1], &in[0], noutput_items);
        for(int i = 0; i < noutput_items; i++) {
          out[i] = f_gain * gr::clenabled::fast_atan2f(imag(tmp[i]), real(tmp[i]));
        }

        return noutput_items;
    }

    int clQuadratureDemod_impl::testOpenCL(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return processOpenCL(noutput_items,ninput_items,input_items, output_items);
    }

    int clQuadratureDemod_impl::processOpenCL(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    {

		if (kernel == NULL) {
			return 0;
		}

		int items_with_history = noutput_items + 1;

    	if (items_with_history > curBufferSize) {
    		setBufferLength(items_with_history);
    	}

    	int inputSize = items_with_history*sizeof(gr_complex);

    	// Protect context from switching
        gr::thread::scoped_lock guard(d_mutex);

        queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputSize,input_items[0]);

		// Do the work

		// Set kernel args
		kernel->setArg(0, *aBuffer);
		kernel->setArg(1, *cBuffer);

		cl::NDRange localWGSize=cl::NullRange;

		// There's something about this call that's messing it up.  Possibly the +1 on the buffer access.
		if (contextType!=CL_DEVICE_TYPE_CPU) {
			if (noutput_items % preferredWorkGroupSizeMultiple == 0) {
				// for some reason problems start to happen when we're no longer using constant memory
				localWGSize=cl::NDRange(preferredWorkGroupSizeMultiple);
			}
		}

		// Do the work
		queue->enqueueNDRangeKernel(
			*kernel,
			cl::NullRange,
			cl::NDRange(noutput_items),
			localWGSize);

    // Map cBuffer to host pointer. This enforces a sync with
    // the host

	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,noutput_items*sizeof(float),(void *)output_items[0]);
/*
	gr_complex *in = (gr_complex*)input_items[0];
	float *out = (float*)output_items[0];
	for (int i=0;i<noutput_items;i++)
		if (in[i].real() != 0.0 || in[i].imag() != 0.0)
			printf("[%f %fj -> %f]\n",in[i].real(),in[i].imag(),out[i]);
*/

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }


    int
    clQuadratureDemod_impl::work (int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
    	if (debugMode && CLPRINT_NITEMS)
    		std::cout << "clQuadratureDemod noutput_items: " << noutput_items << std::endl;

        int retVal = processOpenCL(noutput_items,d_ninput_items,input_items,output_items);
        // int retVal = testCPU(noutput_items,ninput_items,input_items,output_items);

      // Tell runtime system how many output items we produced.
      return retVal;
    }



  } /* namespace clenabled */
} /* namespace gr */

