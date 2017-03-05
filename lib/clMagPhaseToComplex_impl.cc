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
#include "clMagPhaseToComplex_impl.h"

namespace gr {
  namespace clenabled {

    clMagPhaseToComplex::sptr
    clMagPhaseToComplex::make(int openCLPlatformType,int setDebug)
    {
    	if (setDebug == 1)
		  return gnuradio::get_initial_sptr
			(new clMagPhaseToComplex_impl(openCLPlatformType,true));
    	else
  		  return gnuradio::get_initial_sptr
  			(new clMagPhaseToComplex_impl(openCLPlatformType,false));
    }

    /*
     * The private constructor
     */
    clMagPhaseToComplex_impl::clMagPhaseToComplex_impl(int openCLPlatformType,bool setDebug)
      : gr::block("clMagPhaseToComplex",
              gr::io_signature::make(2, 2, sizeof(float)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
  	  	  	  GRCLBase(DTYPE_FLOAT, sizeof(float),openCLPlatformType,setDebug)
    {
    	// Now we set up our OpenCL kernel
        std::string srcStdStr="";
        std::string fnName = "magphasetocomplex";

    	srcStdStr += "struct ComplexStruct {\n";
    	srcStdStr += "float real;\n";
    	srcStdStr += "float imag; };\n";
    	srcStdStr += "typedef struct ComplexStruct SComplex;\n";
    	srcStdStr += "__kernel void magphasetocomplex(__constant float * a, __constant float * b, __global SComplex * restrict c) {\n";
    	srcStdStr += "    size_t index =  get_global_id(0);\n";
    	srcStdStr += "    float mag = a[index];\n";
    	srcStdStr += "    float phase = b[index];\n";
    	srcStdStr += "    float real = mag*cos(phase);\n";
    	srcStdStr += "    float imag = mag*sin(phase);\n";
    	srcStdStr += "    c[index].real = real;\n";
    	srcStdStr += "    c[index].imag = imag;\n";
    	srcStdStr += "}\n";

    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

    	maxConstItems = (int)((float)maxConstMemSize / ((float)sizeof(float)*2.0));

    	if (maxConstItems < imaxItems || imaxItems == 0) {
    		gr::block::set_max_noutput_items(maxConstItems);
    		imaxItems = maxConstItems;

    		if (debugMode)
    			std::cout << "OpenCL INFO: MagPhaseToComplex adjusting output buffer for " << maxConstItems << " due to OpenCL constant memory restrictions" << std::endl;
		}
		else {
			if (debugMode)
				std::cout << "OpenCL INFO: MagPhaseToComplex using default output buffer of " << imaxItems << "..." << std::endl;
		}

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

    /*
     * Our virtual destructor.
     */
    clMagPhaseToComplex_impl::~clMagPhaseToComplex_impl()
    {
    	if (aBuffer)
    		delete aBuffer;

    	if (bBuffer)
    		delete bBuffer;

    	if (cBuffer)
    		delete cBuffer;
    }

    void clMagPhaseToComplex_impl::setBufferLength(int numItems) {
    	if (aBuffer)
    		delete aBuffer;

    	if (bBuffer)
    		delete bBuffer;

    	if (cBuffer)
    		delete cBuffer;

    	aBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_ONLY,
			numItems * sizeof(float));

        bBuffer = new cl::Buffer(
            *context,
			CL_MEM_READ_ONLY,
			numItems * sizeof(float));

        cBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_WRITE,
			numItems * sizeof(gr_complex));

        curBufferSize=numItems;
    }

    /*
     * Our virtual destructor.
     */
    int clMagPhaseToComplex_impl::testCPU(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    	{
        float        *mag = (float *)input_items[0];
        float        *phase = (float *)input_items[1];
        gr_complex *out = (gr_complex *) output_items[0];

        int d_vlen = 1;

        for (size_t j = 0; j < noutput_items*d_vlen; j++)
          out[j] = gr_complex (mag[j]*cos(phase[j]),mag[j]*sin(phase[j]));

        return noutput_items;
    }

    int clMagPhaseToComplex_impl::testOpenCL(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return processOpenCL(noutput_items,ninput_items,input_items, output_items);
    }

    int clMagPhaseToComplex_impl::processOpenCL(int noutput_items,
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

    	int inputSize = noutput_items*sizeof(float);
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

	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,noutput_items*sizeof(gr_complex),(void *)output_items[0]);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }


    void
    clMagPhaseToComplex_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    clMagPhaseToComplex_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const float *mag = (const float *) input_items[0];
      const float *phase = (const float *) input_items[1];
      gr_complex *out = (gr_complex *) output_items[0];

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace clenabled */
} /* namespace gr */

