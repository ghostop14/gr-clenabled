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
#include "clLog_impl.h"

namespace gr {
  namespace clenabled {

    clLog::sptr
    clLog::make(int openCLPlatformType,float nValue,float kValue,int setDebug)
    {

      	if (setDebug == 1) {
            return gnuradio::get_initial_sptr
              (new clLog_impl(openCLPlatformType,nValue,kValue,true));
      	}
      	else {
            return gnuradio::get_initial_sptr
              (new clLog_impl(openCLPlatformType,nValue,kValue,false));
      	}
    }

    /*
     * The private constructor
     */
    clLog_impl::clLog_impl(int openCLPlatformType,float nValue,float kValue,bool setDebug)
      : gr::block("clLog",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(1, 1, sizeof(float))),
			  GRCLBase(DTYPE_FLOAT, sizeof(float),openCLPlatformType,setDebug)

    {
    	n_val = nValue;
    	k_val = kValue;

    	srcStdStr="";
    	fnName = "op_log10";

    	if (nValue != 1.0) {
        	srcStdStr += "#define n_val " + std::to_string(nValue) + "\n";
    	}

    	if (kValue != 0.0) {
    		srcStdStr += "#define k_val " + std::to_string(kValue) + "\n";
    	}

    	srcStdStr += "__kernel void op_log10(__constant float * a, __global float * restrict c) {\n";
    	srcStdStr += "    size_t index =  get_global_id(0);\n";

    	if (kValue != 0.0) {
    		if (nValue != 1.0) {
            	srcStdStr += "    c[index] = n_val * log10(a[index]) + k_val;\n";
    		}
    		else {
            	srcStdStr += "    c[index] = log10(a[index]) + k_val;\n";
    		}
    	}
    	else {
    		// Don't even bother with the k math op.
    		if (nValue != 1.0) {
            	srcStdStr += "    c[index] = n_val * log10(a[index]);\n";
    		}
    		else {
            	srcStdStr += "    c[index] = log10(a[index]);\n";
    		}
    	}

    	srcStdStr += "}\n";

    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

    	// Let's keep this efficient by restricting the number of items to how many we can put in a float
        maxConstItems = (int)((float)maxConstMemSize / ((float)dataSize));

    	if (maxConstItems < imaxItems) {
    		imaxItems = maxConstItems;
    	}

    	set_max_noutput_items(imaxItems);

        GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());

        setBufferLength(imaxItems);

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

        curBufferSize=numItems;
    }

    /*
     * Our virtual destructor.
     */
    clLog_impl::~clLog_impl()
    {
    	if (aBuffer)
    		delete aBuffer;

    	if (cBuffer)
    		delete cBuffer;
    }

    void
    clLog_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
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
        queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputSize,input_items[0]);

		// Do the work

		// Set kernel args
		kernel->setArg(0, *aBuffer);
		kernel->setArg(1, *cBuffer);


		cl::NDRange localWGSize=cl::NullRange;

		if (contextType!=CL_DEVICE_TYPE_CPU) {
			if (noutput_items % preferredWorkGroupSizeMultiple == 0) {
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
    clLog_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        int retVal = processOpenCL(noutput_items,ninput_items,input_items,output_items);
        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return retVal;
    }

  } /* namespace clenabled */
} /* namespace gr */

