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
#include "clMathConst_impl.h"

#ifndef MATHOP_MULTIPLY
#define MATHOP_MULTIPLY 1
#define MATHOP_ADD 2
#endif

namespace gr {
  namespace clenabled {

    clMathConst::sptr
    clMathConst::make(int idataType,int openCLPlatformType,float fValue,int operatorType,int setDebug)
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
  	        (new clMathConst_impl(idataType, dsize,openCLPlatformType,fValue,operatorType,true));
    	}
    	else {
  	      return gnuradio::get_initial_sptr
  	        (new clMathConst_impl(idataType, dsize,openCLPlatformType,fValue,operatorType,false));
    	}
    }

    /*
     * The private constructor
     */
    clMathConst_impl::clMathConst_impl(int idataType, size_t dsize,int openCLPlatformType,float fValue,int operatorType,bool setDebug)
      : gr::block("clMathConst",
              gr::io_signature::make(1, 1, dsize),
              gr::io_signature::make(1, 1, dsize)),
			  GRCLBase(idataType, dsize,openCLPlatformType)
	{
    debugMode = setDebug;

	value = fValue;
	mathOperatorType = operatorType;

	// Now we set up our OpenCL kernel
	std::string srcStdStr;
	std::string fnName = "";

	switch(dataType) {
	case DTYPE_FLOAT:
		switch (mathOperatorType) {
		fnName = "opconst_float";
		srcStdStr = "__kernel void opconst_float(__constant float * a, const float multiplier, __global float * restrict c) {\n";
    	if (operatorType != MATHOP_EMPTY)
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
		srcStdStr = "__kernel void opconst_int(__constant int * a, const int multiplier, __global int * restrict c) {\n";
    	if (operatorType != MATHOP_EMPTY)
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

		srcStdStr += "__kernel void opconst_complex(__constant SComplex * a, const float multiplier, __global SComplex * restrict c) {\n";
    	if (operatorType != MATHOP_EMPTY)
    		srcStdStr += "    size_t index =  get_global_id(0);\n";
    	else
    		srcStdStr += "return;\n";

		switch (mathOperatorType) {
		case MATHOP_MULTIPLY:
		srcStdStr += "    c[index].real = a[index].real * multiplier;\n";
		srcStdStr += "    c[index].imag = a[index].imag * multiplier;\n";
		break;
		case MATHOP_ADD:
		srcStdStr += "    c[index].real = a[index].real + val.real;\n";
		srcStdStr += "    c[index].imag = a[index].imag + val.imag;\n";
		break;
		case MATHOP_SUBTRACT:
		srcStdStr += "    c[index].real = a[index].real - val.real;\n";
		srcStdStr += "    c[index].imag = a[index].imag - val.imag;\n";
		break;
		}
		srcStdStr += "}\n";
	break;
	}

	int imaxItems=gr::block::max_noutput_items();
	if (imaxItems==0)
		imaxItems=8192;

	int maxItemsForConst = (int)((float)maxConstMemSize / ((float)dataSize));

	if (maxItemsForConst < imaxItems || imaxItems == 0) {
		gr::block::set_max_noutput_items(maxItemsForConst);

		imaxItems = maxItemsForConst;

		if (debugMode)
			std::cout << "OpenCL INFO: Math Op Const adjusting output buffer for " << maxItemsForConst << " due to OpenCL constant memory restrictions" << std::endl;
	}
	else {
		if (debugMode)
			std::cout << "OpenCL INFO: Math Op Const using default output buffer of " << imaxItems << "..." << std::endl;
	}

	GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());

	setBufferLength(imaxItems);
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

        curBufferSize = numItems;
    }
  /*
     * Our virtual destructor.
     */
    clMathConst_impl::~clMathConst_impl()
    {
    	if (aBuffer)
    		delete aBuffer;

    	if (cBuffer)
    		delete cBuffer;

    	// Called in grclbase destructor
    	// cleanup();
    }

    void
    clMathConst_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int clMathConst_impl::testCPU(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    	{

        const SComplex *in = (const SComplex *) input_items[0];
        SComplex *out = (SComplex *) output_items[0];

		for (int i=0;i<noutput_items;i++) {
			out[i].real = in[i].real * value;
			out[i].imag = in[i].imag * value;
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

        // Set kernel args
        queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputSize,input_items[0]);

        kernel->setArg(0, *aBuffer);

        kernel->setArg(1, value);
        kernel->setArg(2, *cBuffer);

        cl::NDRange localWGSize=cl::NullRange;
        // localWGSize = cl::NDRange(preferredWorkGroupSizeMultiple);

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
      // Tell runtime system how many input items we consumed on
      // each input stream.
      // consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

    int
	clMathConst_impl::general_work (int noutput_items,
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

    void
	clMathConst_impl::setup_rpc()
    {
#ifdef GR_CTRLPORT
      add_rpc_variable(
        rpcbasic_sptr(new rpcbasic_register_get<clMathConst, float>(
	  alias(), "Constant",
	  &clMathConst::k,
	  -1024.0f,
          1024.0f,
          0.0f,
	  "", "Constant", RPC_PRIVLVL_MIN,
          DISPTIME | DISPOPTCPLX | DISPOPTSTRIP)));

      add_rpc_variable(
        rpcbasic_sptr(new rpcbasic_register_set<clMathConst, float>(
	  alias(), "Constant",
	  &clMathConst::set_k,
	  -1024.0f,
          1024.0f,
          0.0f,
	  "", "Constant",
	  RPC_PRIVLVL_MIN, DISPNULL)));
#endif /* GR_CTRLPORT */
    }
  } /* namespace clenabled */
} /* namespace gr */

