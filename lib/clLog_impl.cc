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
#include "clLog_impl.h"

namespace gr {
  namespace clenabled {

    clLog::sptr
    clLog::make(int openCLPlatformType,int devSelector,int platformId, int devId,float nValue,float kValue,int setDebug)
    {

      	if (setDebug == 1) {
            return gnuradio::get_initial_sptr
              (new clLog_impl(openCLPlatformType,devSelector,platformId,devId,nValue,kValue,true));
      	}
      	else {
            return gnuradio::get_initial_sptr
              (new clLog_impl(openCLPlatformType,devSelector,platformId,devId,nValue,kValue,false));
      	}
    }

    /*
     * The private constructor
     */
    clLog_impl::clLog_impl(int openCLPlatformType,int devSelector,int platformId, int devId,float nValue,float kValue,bool setDebug)
      : gr::sync_block("clLog",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(1, 1, sizeof(float))),
			  GRCLBase(DTYPE_FLOAT, sizeof(float),openCLPlatformType,devSelector,platformId,devId,setDebug)

    {
    	n_val = nValue;
    	k_val = kValue;

    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

        setBufferLength(imaxItems);

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
}

    void clLog_impl::buildKernel(int numItems) {
    	maxConstItems = (int)((float)maxConstMemSize / ((float)dataSize));
    	bool useConst;

    	if (numItems > maxConstItems)
    		useConst = false;
    	else
    		useConst = true;

		if (debugMode) {
			if (useConst)
				std::cout << "OpenCL INFO: Log10 Const building kernel with __constant params..." << std::endl;
			else
				std::cout << "OpenCL INFO: Log10 - too many items for constant memory.  Building kernel with __global params..." << std::endl;
		}

    	srcStdStr="";
    	fnName = "op_log10";

    	if (n_val != 1.0) {
        	srcStdStr += "#define n_val " + std::to_string(n_val) + "\n";
    	}

    	if (k_val != 0.0) {
    		srcStdStr += "#define k_val " + std::to_string(k_val) + "\n";
    	}

    	if (useConst)
    		srcStdStr += "__kernel void op_log10(__constant float * a, __global float * restrict c) {\n";
    	else
    		srcStdStr += "__kernel void op_log10(__global float * restrict a, __global float * restrict c) {\n";

    	srcStdStr += "    size_t index =  get_global_id(0);\n";

    	if (k_val != 0.0) {
    		if (n_val != 1.0) {
            	srcStdStr += "    c[index] = n_val * log10(a[index]) + k_val;\n";
    		}
    		else {
            	srcStdStr += "    c[index] = log10(a[index]) + k_val;\n";
    		}
    	}
    	else {
    		// Don't even bother with the k math op.
    		if (n_val != 1.0) {
            	srcStdStr += "    c[index] = n_val * log10(a[index]);\n";
    		}
    		else {
            	srcStdStr += "    c[index] = log10(a[index]);\n";
    		}
    	}

    	srcStdStr += "}\n";

        GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());
    }

    void clLog_impl::setBufferLength(int numItems) {
    	if (aBuffer)
    		delete aBuffer;

    	if (cBuffer)
    		delete cBuffer;

    	aBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_ONLY,
			numItems * dataSize);

        cBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_WRITE,
			numItems * dataSize);

        buildKernel(numItems);

        curBufferSize=numItems;
    }

    /*
     * Our virtual destructor.
     */
    clLog_impl::~clLog_impl()
    {
    	if (curBufferSize > 0)
    		stop();
    }

    bool clLog_impl::stop() {
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

    int clLog_impl::testCPU(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    	{

        const float *in1 = (const float *) input_items[0];
        float *out = (float *) output_items[0];

		for (int i=0;i<noutput_items;i++) {
			out[i] = n_val * log10(in1[i]) + k_val;
		}

    	return noutput_items;
    }

    int clLog_impl::testOpenCL(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return processOpenCL(noutput_items,ninput_items,input_items, output_items);
    }

    int clLog_impl::processOpenCL(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    {

		if (kernel == NULL) {
			return 0;
		}

    	if (noutput_items > curBufferSize) {
    		setBufferLength(noutput_items);
    	}

    	int inputSize = noutput_items*dataSize;

    	// Protect context from switching
        gr::thread::scoped_lock guard(d_mutex);

        queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputSize,input_items[0]);

		// Do the work

		// Set kernel args
		kernel->setArg(0, *aBuffer);
		kernel->setArg(1, *cBuffer);


		cl::NDRange localWGSize=cl::NullRange;

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
	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,inputSize,(void *)output_items[0]);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }


    int
    clLog_impl::work (int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
    	if (debugMode && CLPRINT_NITEMS)
    		std::cout << "clLog noutput_items: " << noutput_items << std::endl;

        int retVal = processOpenCL(noutput_items,d_ninput_items,input_items,output_items);

        // Tell runtime system how many output items we produced.
        return retVal;
    }

  } /* namespace clenabled */
} /* namespace gr */

