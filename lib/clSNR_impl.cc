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
#include "clSNR_impl.h"

namespace gr {
  namespace clenabled {

    clSNR::sptr
    clSNR::make(int openCLPlatformType,float nValue,float kValue,int setDebug)
    {
      	if (setDebug == 1) {
            return gnuradio::get_initial_sptr
              (new clSNR_impl(openCLPlatformType,nValue,kValue,true));
      	}
      	else {
            return gnuradio::get_initial_sptr
              (new clSNR_impl(openCLPlatformType,nValue,kValue,false));
      	}
    }

    /*
     * The private constructor
     */
    clSNR_impl::clSNR_impl(int openCLPlatformType,float nValue,float kValue,bool setDebug)
      : gr::block("clSNR",
              gr::io_signature::make(2, 2, sizeof(float)),
              gr::io_signature::make(1, 1, sizeof(float))),
			  GRCLBase(DTYPE_FLOAT, sizeof(float),openCLPlatformType,setDebug)
    {
    	srcStdStr="";
    	fnName = "op_snr";

    	n_val = nValue;
    	k_val = kValue;

    	srcStdStr += "#define n_val " + std::to_string(nValue) + "\n";
		srcStdStr += "#define k_val " + std::to_string(kValue) + "\n";
    	srcStdStr += "__kernel void op_snr(__constant float * a, __constant float * b, __global float * restrict c) {\n";
    	srcStdStr += "    size_t index =  get_global_id(0);\n";
    	srcStdStr += "    float tmpVal = a[index] / b[index];\n";
    	srcStdStr += "    tmpVal = n_val * log10(tmpVal) + k_val;\n";
    	srcStdStr += "    c[index] = fabs(tmpVal);\n";
    	srcStdStr += "}\n";

    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

    	// Let's keep this efficient by restricting the number of items to how many we can put in a float
        maxConstItems = (int)((float)maxConstMemSize / ((float)dataSize));
        maxConstItems = maxConstItems / 2; // 2 inputs

    	if (maxConstItems < imaxItems) {
    		imaxItems = maxConstItems;
    	}

    	set_max_noutput_items(imaxItems);

        GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());

        setBufferLength(imaxItems);
        // And finally optimize the data we get based on the preferred workgroup size.
        // Note: We can't do this until the kernel is compiled and since it's in the block class
        // it has to be done here.
        // Note: for CPU's adjusting the workgroup size away from 1 seems to decrease performance.
        // For GPU's setting it to the preferred size seems to have the best performance.
        if (contextType != CL_DEVICE_TYPE_CPU) {
        	gr::block::set_output_multiple(preferredWorkGroupSizeMultiple);
        }
}

    void clSNR_impl::setBufferLength(int numItems) {
    	if (aBuffer)
    		delete aBuffer;

    	if (bBuffer)
    		delete bBuffer;

    	if (cBuffer)
    		delete cBuffer;

    	aBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_ONLY,
			numItems * dataSize);

        bBuffer = new cl::Buffer(
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
    clSNR_impl::~clSNR_impl()
    {
    	if (aBuffer)
    		delete aBuffer;

    	if (bBuffer)
    		delete bBuffer;

    	if (cBuffer)
    		delete cBuffer;
    }

    void
    clSNR_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int clSNR_impl::testCPU(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    	{

        const float *in1 = (const float *) input_items[0];
        const float *in2 = (const float *) input_items[1];
        float *out = (float *) output_items[0];

        float tmpVal;

		for (int i=0;i<noutput_items;i++) {
	    	tmpVal = in1[i] / in2[i];
	    	tmpVal = n_val * log10(tmpVal) + k_val;
	    	out[i] = fabs(tmpVal);
		}

    	return noutput_items;
    }

    int clSNR_impl::testOpenCL(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return processOpenCL(noutput_items,ninput_items,input_items, output_items);
    }

    int clSNR_impl::processOpenCL(int noutput_items,
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
        queue->enqueueWriteBuffer(*bBuffer,CL_TRUE,0,inputSize,input_items[1]);

		// Do the work

		// Set kernel args
		kernel->setArg(0, *aBuffer);
		kernel->setArg(1, *bBuffer);
		kernel->setArg(2, *cBuffer);


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
    clSNR_impl::general_work (int noutput_items,
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

