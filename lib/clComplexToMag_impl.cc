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
#include "clComplexToMag_impl.h"
#include "fast_atan2f.h"

namespace gr {
  namespace clenabled {

    clComplexToMag::sptr
    clComplexToMag::make(int openCLPlatformType,int setDebug)
    {
    	if (setDebug == 1)
		  return gnuradio::get_initial_sptr
			(new clComplexToMag_impl(openCLPlatformType,true));
    	else
  		  return gnuradio::get_initial_sptr
  			(new clComplexToMag_impl(openCLPlatformType,false));
    }

    /*
     * The private constructor
     */
    clComplexToMag_impl::clComplexToMag_impl(int openCLPlatformType,bool setDebug)
      : gr::block("clComplexToMag",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(float))),
	  GRCLBase(DTYPE_COMPLEX, sizeof(gr_complex),openCLPlatformType,setDebug)
{
    	// Now we set up our OpenCL kernel
        std::string srcStdStr="";
        std::string fnName = "complextomag";

    	srcStdStr += "struct ComplexStruct {\n";
    	srcStdStr += "float real;\n";
    	srcStdStr += "float imag; };\n";
    	srcStdStr += "typedef struct ComplexStruct SComplex;\n";
    	srcStdStr += "__kernel void complextomag(__constant SComplex * a, __global float * restrict c) {\n";
    	srcStdStr += "    size_t index =  get_global_id(0);\n";
    	srcStdStr += "    float aval = a[index].imag;\n";
    	srcStdStr += "    float bval = a[index].real;\n";
    	srcStdStr += "    c[index] = sqrt(aval*aval+bval*bval);\n";
    	srcStdStr += "}\n";

    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

    	maxConstItems = (int)((float)maxConstMemSize / ((float)sizeof(gr_complex)));

    	if (maxConstItems < imaxItems || imaxItems == 0) {
    		gr::block::set_max_noutput_items(maxConstItems);
    		imaxItems = maxConstItems;

    		if (debugMode)
    			std::cout << "OpenCL INFO: ComplexToMag adjusting output buffer for " << maxConstItems << " due to OpenCL constant memory restrictions" << std::endl;
		}
		else {
			if (debugMode)
				std::cout << "OpenCL INFO: ComplexToMag using default output buffer of " << imaxItems << "..." << std::endl;
		}

        GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());

        setBufferLength(imaxItems);

}
    /*
     * Our virtual destructor.
     */
    clComplexToMag_impl::~clComplexToMag_impl()
    {
    	if (aBuffer)
    		delete aBuffer;

    	if (cBuffer)
    		delete cBuffer;
    }


    void clComplexToMag_impl::setBufferLength(int numItems) {
    	if (aBuffer)
    		delete aBuffer;

    	if (cBuffer)
    		delete cBuffer;

    	aBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_ONLY,
			numItems * sizeof(gr_complex));

        cBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_WRITE,
			numItems * sizeof(float));

        curBufferSize=numItems;
    }

    /*
     * Our virtual destructor.
     */
    int clComplexToMag_impl::testCPU(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    	{
        gr_complex *in = (gr_complex*)input_items[0];
        float *out = (float*)output_items[0];

        for(int i = 0; i < noutput_items; i++) {
        	out[i] = sqrt(in[i].imag()*in[i].imag()+in[i].real()*in[i].real());
        }

        return noutput_items;
    }

    int clComplexToMag_impl::testOpenCL(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return processOpenCL(noutput_items,ninput_items,input_items, output_items);
    }

    int clComplexToMag_impl::processOpenCL(int noutput_items,
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

    	int inputSize = noutput_items*sizeof(gr_complex);
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

	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,noutput_items*sizeof(float),(void *)output_items[0]);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

    void
    clComplexToMag_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    clComplexToMag_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        int retVal = processOpenCL(noutput_items,ninput_items,input_items,output_items);

      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return retVal;
    }

  } /* namespace clenabled */
} /* namespace gr */

