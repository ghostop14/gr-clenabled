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
#include "clMathConst_impl.h"

#ifndef MATHOP_MULTIPLY
#define MATHOP_MULTIPLY 1
#define MATHOP_ADD 2
#endif

namespace gr {
  namespace clenabled {

    clMathConst::sptr
    clMathConst::make(int idataType,int openCLPlatformType, int devSelector,int platformId, int devId,float fValue,int operatorType,int setDebug)
    {
    	size_t dsize = sizeof(float);

    	switch(idataType) {
    	case DTYPE_FLOAT: // float
    		dsize=sizeof(float);
    	break;
    	case DTYPE_INT: // int
    		dsize=sizeof(int);
    	break;
    	case DTYPE_COMPLEX:
    		dsize=sizeof(gr_complex);
    	break;
    	}

    	if (setDebug == 1) {
  	      return gnuradio::get_initial_sptr
  	        (new clMathConst_impl(idataType, dsize,openCLPlatformType,devSelector,platformId,devId,fValue,operatorType,true));
    	}
    	else {
  	      return gnuradio::get_initial_sptr
  	        (new clMathConst_impl(idataType, dsize,openCLPlatformType,devSelector,platformId,devId,fValue,operatorType,false));
    	}
    }

    /*
     * The private constructor
     */
    clMathConst_impl::clMathConst_impl(int idataType, size_t dsize,int openCLPlatformType, int devSelector,int platformId, int devId,float fValue,int operatorType,bool setDebug)
      : gr::sync_block("clMathConst",
              gr::io_signature::make(1, 1, dsize),
              gr::io_signature::make(1, 1, dsize)),
			  GRCLBase(idataType, dsize,openCLPlatformType,devSelector,platformId,devId,setDebug)
	{
	value = fValue;
	mathOperatorType = operatorType;

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

    void clMathConst_impl::buildKernel(int numItems) {
    	maxConstItems = (int)((float)maxConstMemSize / ((float)dataSize));
    	bool useConst;

    	if (numItems > maxConstItems)
    		useConst = false;
    	else
    		useConst = true;

		if (debugMode) {
			if (useConst)
				std::cout << "OpenCL INFO: Math Op Const building kernel with __constant params..." << std::endl;
			else
				std::cout << "OpenCL INFO: Math Op Const - too many items for constant memory.  Building kernel with __global params..." << std::endl;
		}

		// Now we set up our OpenCL kernel
		srcStdStr = "";
		fnName = "";

		switch(dataType) {
		case DTYPE_FLOAT:
			switch (mathOperatorType) {
			fnName = "opconst_float";
			if (useConst)
				srcStdStr = "__kernel void opconst_float(__constant float * a, const float multiplier, __global float * restrict c) {\n";
			else
				srcStdStr = "__kernel void opconst_float(__global float * restrict a, const float multiplier, __global float * restrict c) {\n";

	    	if (mathOperatorType != MATHOP_EMPTY)
	    		srcStdStr += "    size_t index =  get_global_id(0);\n";
			case MATHOP_MULTIPLY:
			srcStdStr += "    c[index] = a[index] * multiplier;\n";
			break;

			case MATHOP_ADD:
			srcStdStr += "    c[index] = a[index] + multiplier;\n";
			break;
			case MATHOP_SUBTRACT:
			srcStdStr += "    c[index] = a[index] - multiplier;\n";
			break;
			}
			srcStdStr += "}\n";
		break;
		case DTYPE_INT:
			fnName = "opconst_int";
			if (useConst)
				srcStdStr = "__kernel void opconst_int(__constant int * a, const int multiplier, __global int * restrict c) {\n";
			else
				srcStdStr = "__kernel void opconst_int(__global int * restrict a, const int multiplier, __global int * restrict c) {\n";

	    	if (mathOperatorType != MATHOP_EMPTY)
	    		srcStdStr += "    size_t index =  get_global_id(0);\n";
			switch (mathOperatorType) {
			case MATHOP_MULTIPLY:
			srcStdStr += "    c[index] = a[index] * multiplier;\n";
			break;

			case MATHOP_ADD:
			srcStdStr += "    c[index] = a[index] + multiplier;\n";
			break;

			case MATHOP_SUBTRACT:
			srcStdStr += "    c[index] = a[index] - multiplier;\n";
			break;
			}
			srcStdStr += "}\n";
		break;
		case DTYPE_COMPLEX:
			fnName = "opconst_complex";
			srcStdStr = "struct ComplexStruct {\n";
			srcStdStr += "float real;\n";
			srcStdStr += "float imag; };\n";
			srcStdStr += "typedef struct ComplexStruct SComplex;\n";

			if (useConst)
				srcStdStr += "__kernel void opconst_complex(__constant SComplex * a, const float multiplier, __global SComplex * restrict c) {\n";
			else
				srcStdStr += "__kernel void opconst_complex(__global SComplex * restrict a, const float multiplier, __global SComplex * restrict c) {\n";

	    	if (mathOperatorType != MATHOP_EMPTY)
	    		srcStdStr += "    size_t index =  get_global_id(0);\n";
	    	else
	    		srcStdStr += "return;\n";

			switch (mathOperatorType) {
			case MATHOP_EMPTY_W_COPY:
				srcStdStr += "    c[index].real = a[index].real;\n";
				srcStdStr += "    c[index].imag = a[index].imag;\n";
			case MATHOP_MULTIPLY:
			srcStdStr += "    c[index].real = a[index].real * multiplier;\n";
			srcStdStr += "    c[index].imag = a[index].imag * multiplier;\n";
			break;
			case MATHOP_ADD:
			srcStdStr += "    c[index].real = a[index].real + multiplier;\n";
			srcStdStr += "    c[index].imag = a[index].imag + multiplier;\n";
			break;
			case MATHOP_SUBTRACT:
			srcStdStr += "    c[index].real = a[index].real - multiplier;\n";
			srcStdStr += "    c[index].imag = a[index].imag - multiplier;\n";
			break;

        	case MATHOP_COMPLEX_CONJUGATE:
            	srcStdStr = "struct ComplexStruct {\n";
            	srcStdStr += "float real;\n";
            	srcStdStr += "float imag; };\n";
            	srcStdStr += "typedef struct ComplexStruct SComplex;\n";

            	fnName = "op_complex";
            	if (useConst)
            		srcStdStr += "__kernel void op_complex(__constant SComplex * a, __global SComplex * restrict c) {\n";
            	else
            		srcStdStr += "__kernel void op_complex(__global SComplex * restrict a, __global SComplex * restrict c) {\n";

            	srcStdStr += "    size_t index =  get_global_id(0);\n";
            	srcStdStr += "    c[index].real = a[index].real;\n";
            	srcStdStr += "    c[index].imag = -1.0 * a[index].imag;\n";
        	break;

			}
			srcStdStr += "}\n";
		break;
		}

    }


    void clMathConst_impl::setBufferLength(int numItems) {
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
    	GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());

        curBufferSize = numItems;
    }
  /*
     * Our virtual destructor.
     */
    clMathConst_impl::~clMathConst_impl()
    {
    	if (curBufferSize > 0)
    		stop();
    }

    bool clMathConst_impl::stop() {
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

    int clMathConst_impl::testCPU(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    	{

        const SComplex *in = (const SComplex *) input_items[0];
        SComplex *out = (SComplex *) output_items[0];

		switch (mathOperatorType) {
		case MATHOP_EMPTY_W_COPY:
			for (int i=0;i<noutput_items;i++) {
				out[i].real = in[i].real;
				out[i].imag = in[i].imag;
			}
		break;
		case MATHOP_MULTIPLY:
			for (int i=0;i<noutput_items;i++) {
				out[i].real = in[i].real * value;
				out[i].imag = in[i].imag * value;
			}
		break;
		}


    	return noutput_items;
    }


    int clMathConst_impl::testOpenCL(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return processOpenCL(noutput_items,ninput_items,input_items, output_items);
    }

    int clMathConst_impl::processOpenCL(int noutput_items,
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

        // Set kernel args
        queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputSize,input_items[0]);

        kernel->setArg(0, *aBuffer);

        if (mathOperatorType == MATHOP_COMPLEX_CONJUGATE) {
            kernel->setArg(1, *cBuffer);
        }
        else {
            kernel->setArg(1, value);
            kernel->setArg(2, *cBuffer);
        }

        cl::NDRange localWGSize=cl::NullRange;
        // localWGSize = cl::NDRange(preferredWorkGroupSizeMultiple);

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

    	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,inputSize,(void *)output_items[0]);
/*
      // Map cBuffer to host pointer. This enforces a sync with
      // the host backing space, remember we choose GPU device.
      void * output = (void *) queue->enqueueMapBuffer(
          *cBuffer,
          CL_TRUE, // block
          CL_MAP_READ,
          0,
			noutput_items * dataSize);

      memcpy((void *)output_items[0],output,inputSize);

      cl_int err;

      // Finally release our hold on accessing the memory
      err = queue->enqueueUnmapMemObject(
          *cBuffer,
          (void *) output);
*/

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

    int
	clMathConst_impl::work (int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
    	if (debugMode && CLPRINT_NITEMS)
    		std::cout << "clMathConst noutput_items: " << noutput_items << std::endl;

        int retVal = processOpenCL(noutput_items,d_ninput_items,input_items,output_items);

        // Tell runtime system how many output items we produced.
        return retVal;
    }

    void
	clMathConst_impl::setup_rpc()
    {
#ifdef GR_CTRLPORT
      add_rpc_variable(
        rpcbasic_sptr(new rpcbasic_register_get<clMathConst, float>(
	  alias(), "Constant",
	  &clMathConst::k,
	  pmt::from_complex(-4.29e9, 0),
          pmt::from_complex(4.29e9, 0),
          pmt::from_complex(0, 0),
	  "", "Constant", RPC_PRIVLVL_MIN,
          DISPTIME | DISPOPTCPLX | DISPOPTSTRIP)));

      add_rpc_variable(
        rpcbasic_sptr(new rpcbasic_register_set<clMathConst, float>(
	  alias(), "Constant",
	  &clMathConst::set_k,
	  pmt::from_complex(-4.29e9, 0),
          pmt::from_complex(4.29e9, 0),
          pmt::from_complex(0, 0),
	  "", "Constant",
	  RPC_PRIVLVL_MIN, DISPNULL)));
#endif /* GR_CTRLPORT */
    }
  } /* namespace clenabled */
} /* namespace gr */

