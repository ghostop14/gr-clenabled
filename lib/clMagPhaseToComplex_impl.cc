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
#include "clMagPhaseToComplex_impl.h"

namespace gr {
  namespace clenabled {

    clMagPhaseToComplex::sptr
    clMagPhaseToComplex::make(int openCLPlatformType,int devSelector,int platformId, int devId,int setDebug)
    {
    	if (setDebug == 1)
		  return gnuradio::get_initial_sptr
			(new clMagPhaseToComplex_impl(openCLPlatformType,devSelector,platformId,devId,true));
    	else
  		  return gnuradio::get_initial_sptr
  			(new clMagPhaseToComplex_impl(openCLPlatformType,devSelector,platformId,devId,false));
    }

    /*
     * The private constructor
     */
    clMagPhaseToComplex_impl::clMagPhaseToComplex_impl(int openCLPlatformType,int devSelector,int platformId, int devId,bool setDebug)
      : gr::sync_block("clMagPhaseToComplex",
              gr::io_signature::make(2, 2, sizeof(float)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
  	  	  	  GRCLBase(DTYPE_FLOAT, sizeof(float),openCLPlatformType,devSelector,platformId,devId,setDebug)
    {
    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;
/*
    	if (imaxItems > maxConstItems) {
    		imaxItems = maxConstItems;
    	}

		try {
			// optimize for constant memory space
			gr::block::set_max_noutput_items(imaxItems);
		}
		catch(...) {

		}
*/
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

    /*
     * Our virtual destructor.
     */
    clMagPhaseToComplex_impl::~clMagPhaseToComplex_impl()
    {
    	if (curBufferSize > 0)
    		stop();
    }

    bool clMagPhaseToComplex_impl::stop() {
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

    	return GRCLBase::stop();
    }

    void clMagPhaseToComplex_impl::buildKernel(int numItems) {
    	maxConstItems = (int)((float)maxConstMemSize / ((float)dataSize));
    	bool useConst;
/*
    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

    	if (maxConstItems < imaxItems) {
    		try {
    			gr::block::set_max_noutput_items(maxConstItems);
    		}
    		catch(...) {

    		}

    		imaxItems = maxConstItems;

    		if (debugMode)
    			std::cout << "OpenCL INFO: MagPhaseToComplex adjusting gnuradio output buffer for " << maxConstItems << " due to OpenCL constant memory restrictions" << std::endl;
		}
		else {
			if (debugMode)
				std::cout << "OpenCL INFO: MagPhaseToComplex using default gnuradio output buffer of " << imaxItems << "..." << std::endl;
		}
*/
    	if (numItems > maxConstItems)
    		useConst = false;
    	else
    		useConst = true;

    	if (!hasDoublePrecisionSupport) {
    		std::cout << "OpenCL Mag/Phase to Complex Warning: Your selected OpenCL platform doesn't support double precision math.  The resulting output from this block is going to contain potentially impactful 'noise' (plot it on a frequency plot versus native block for comparison)." << std::endl;
    	}

		if (debugMode) {
			if (useConst)
				std::cout << "OpenCL INFO: MagPhaseToComplex building kernel with __constant params..." << std::endl;
			else
				std::cout << "OpenCL INFO: MagPhaseToComplex - too many items for constant memory.  Building kernel with __global params..." << std::endl;
		}

        // Now we set up our OpenCL kernel
        std::string srcStdStr="";
        std::string fnName = "magphasetocomplex";

    	srcStdStr += "struct ComplexStruct {\n";
    	srcStdStr += "float real;\n";
    	srcStdStr += "float imag; };\n";
    	srcStdStr += "typedef struct ComplexStruct SComplex;\n";
    	if (useConst)
    		srcStdStr += "__kernel void magphasetocomplex(__constant float * a, __constant float * b, __global SComplex * restrict c) {\n";
    	else
    		srcStdStr += "__kernel void magphasetocomplex(__global float * restrict a, __global float * restrict b, __global SComplex * restrict c) {\n";

    	srcStdStr += "    size_t index =  get_global_id(0);\n";

    	if (hasDoublePrecisionSupport) {
        	srcStdStr += "    double mag = (double)a[index];\n";
        	srcStdStr += "    double phase = (double)b[index];\n";

        	srcStdStr += "    float real = (float)(mag*cos(phase));\n";
        	srcStdStr += "    float imag = (float)(mag*sin(phase));\n";
    	}
    	else {
        	srcStdStr += "    float mag = a[index];\n";
        	srcStdStr += "    float phase = b[index];\n";

        	srcStdStr += "    float real = mag*cos(phase);\n";
        	srcStdStr += "    float imag = mag*sin(phase);\n";
    	}

    	srcStdStr += "    c[index].real = real;\n";
    	srcStdStr += "    c[index].imag = imag;\n";
    	srcStdStr += "}\n";

        GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());
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

        buildKernel(numItems);

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

    	// Protect context from switching
        gr::thread::scoped_lock guard(d_mutex);

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

	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,noutput_items*sizeof(gr_complex),(void *)output_items[0]);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }


    int
    clMagPhaseToComplex_impl::work (int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
    	if (debugMode && CLPRINT_NITEMS)
    		std::cout << "clMagPhaseToComplex noutput_items: " << noutput_items << std::endl;

        int retVal = processOpenCL(noutput_items,d_ninput_items,input_items,output_items);

      // Tell runtime system how many output items we produced.
      return retVal;
    }

  } /* namespace clenabled */
} /* namespace gr */

