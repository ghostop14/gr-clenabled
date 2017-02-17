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
#include "clMathOp_impl.h"
#include "clMathOpTypes.h"

namespace gr {
  namespace clenabled {

    clMathOp::sptr
    clMathOp::make(int idataType, int openCLPlatformType,int operatorType,int setDebug)
    {
      int dsize=sizeof(float);

      switch(idataType) {
      case DTYPE_FLOAT:
    	  dsize=sizeof(float);
      break;
      case DTYPE_INT:
    	  dsize=sizeof(int);
      break;
      case DTYPE_COMPLEX:
    	  dsize=sizeof(gr_complex);
      break;
      }

  	if (setDebug == 1) {
        return gnuradio::get_initial_sptr
          (new clMathOp_impl(idataType,dsize,openCLPlatformType,operatorType,true));
  	}
  	else {
        return gnuradio::get_initial_sptr
          (new clMathOp_impl(idataType,dsize,openCLPlatformType,operatorType,false));
  	}
}

    /*
     * The private constructor
     */
    clMathOp_impl::clMathOp_impl(int idataType, size_t dsize,int openCLPlatformType,int operatorType, bool setDebug)
      : gr::block("clMathOp",
              gr::io_signature::make(2, 2, dsize),
              gr::io_signature::make(1, 1, dsize)),
			  GRCLBase(idataType, dsize,openCLPlatformType)
    {
    	debugMode=setDebug;

    	// Now we set up our OpenCL kernel
        std::string srcStdStr;
        std::string fnName = "";

        numParams = 2;

        int numConstParams = 2;

        switch(dataType) {
        case DTYPE_FLOAT:
        	// Float data type
        	fnName = "op_float";
        	srcStdStr = "__kernel void op_float(__constant float * a, __constant float * b, __global float * restrict c) {\n";
        	if (operatorType != MATHOP_EMPTY)
        		srcStdStr += "    size_t index =  get_global_id(0);\n";

        	switch (operatorType) {
        	case MATHOP_MULTIPLY:
            	srcStdStr += "    c[index] = a[index] * b[index];\n";
        	break;

        	case MATHOP_ADD:
            	srcStdStr += "    c[index] = a[index] + b[index];\n";
        	break;

        	case MATHOP_SUBTRACT:
            	srcStdStr += "    c[index] = a[index] - b[index];\n";
        	break;

        	// MATHOP_EMPTY will just fall through

        	case MATHOP_LOG:
                numParams = 1;
        		// restart function... only have 1 param
            	srcStdStr = "__kernel void op_float(__constant float * a, __global float * restrict c) {\n";
            	srcStdStr += "    size_t index =  get_global_id(0);\n";
            	srcStdStr += "    c[index] = log(a[index]);\n";
            	numConstParams = 1;
        	break;

        	case MATHOP_LOG10:
                numParams = 1;

        		// restart function... only have 1 param
            	srcStdStr = "__kernel void op_float(__constant float * a, __global float * restrict c) {\n";
            	srcStdStr += "    size_t index =  get_global_id(0);\n";
            	srcStdStr += "    c[index] = log10(a[index]);\n";
            	numConstParams = 1;
        	break;
        	}
        	srcStdStr += "}\n";
        break;


        case DTYPE_INT:
        	fnName = "op_int";
        	srcStdStr = "__kernel void op_int(__constant int * a, __constant * b, __global int * restrict c) {\n";
        	if (operatorType != MATHOP_EMPTY)
        		srcStdStr += "    size_t index =  get_global_id(0);\n";

        	switch (operatorType) {
        	// MATHOP_EMPTY will just fall through

        	case MATHOP_MULTIPLY:
            	srcStdStr += "    c[index] = a[index] * b[index];\n";
           	break;
        	case MATHOP_ADD:
            	srcStdStr += "    c[index] = a[index] + b[index];\n";
        	break;
        	case MATHOP_SUBTRACT:
            	srcStdStr += "    c[index] = a[index] - b[index];\n";
        	break;
        	}
        	srcStdStr += "}\n";
        break;
        case DTYPE_COMPLEX:
        	srcStdStr = "struct ComplexStruct {\n";
        	srcStdStr += "float real;\n";
        	srcStdStr += "float imag; };\n";
        	srcStdStr += "typedef struct ComplexStruct SComplex;\n";

        	fnName = "op_complex";
        	srcStdStr += "__kernel void op_complex(__constant SComplex * a, __constant SComplex * b, __global SComplex * restrict c) {\n";
        	if (operatorType != MATHOP_EMPTY)
        		srcStdStr += "    size_t index =  get_global_id(0);\n";
        	switch (operatorType) {
        	case MATHOP_MULTIPLY:
            	srcStdStr += "    float a_r=a[index].real;\n";
            	srcStdStr += "    float a_i=a[index].imag;\n";
            	srcStdStr += "    float b_r=b[index].real;\n";
            	srcStdStr += "    float b_i=b[index].imag;\n";
            	srcStdStr += "    c[index].real = a_r * b_r - (a_i*b_i);\n";
            	srcStdStr += "    c[index].imag = a_r * b_i + a_i * b_r;\n";
        	break;
        	case MATHOP_ADD:
            	srcStdStr += "    c[index].real = a[index].real + b[index].real;\n";
            	srcStdStr += "    c[index].imag = a[index].imag + b[index].imag;\n";
        	break;
        	case MATHOP_SUBTRACT:
            	srcStdStr += "    c[index].real = a[index].real - b[index].real;\n";
            	srcStdStr += "    c[index].imag = a[index].imag - b[index].imag;\n";
        	break;

        	case MATHOP_COMPLEX_CONJUGATE:
                numParams = 1;
            	srcStdStr += "__kernel void op_complex(__constant SComplex * a, __global SComplex * restrict c) {\n";
            	srcStdStr += "    size_t index =  get_global_id(0);\n";
            	srcStdStr += "    c[index].real = a[index].real;\n";
            	srcStdStr += "    c[index].imag = -1.0 * a[index].imag;\n";
            	numConstParams = 1;
        	break;

        	case MATHOP_MULTIPLY_CONJUGATE:
                numParams = 1;
            	srcStdStr += "__kernel void op_complex(__constant SComplex * a, __global SComplex * restrict c) {\n";
            	srcStdStr += "    size_t index =  get_global_id(0);\n";
            	srcStdStr += "    c[index].real = a[index].real;\n";
            	srcStdStr += "    c[index].imag = -1.0 * a[index].imag;\n";
            	srcStdStr += "    float a_r=a[index].real;\n";
            	srcStdStr += "    float a_i=a[index].imag;\n";
            	srcStdStr += "    float b_r=a[index].real;\n";
            	srcStdStr += "    float b_i=-1.0 * a[index].imag;\n";
            	srcStdStr += "    c[index].real = a_r * b_r - (a_i*b_i);\n";
            	srcStdStr += "    c[index].imag = a_r * b_i + a_i * b_r;\n";
            	numConstParams = 1;
        	break;
        	}
        	srcStdStr += "}\n";
        break;
        }

    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

    	int maxItemsForConst = (int)((float)maxConstMemSize / ((float)dataSize*numConstParams));

    	if (maxItemsForConst < imaxItems || imaxItems == 0) {
    		gr::block::set_max_noutput_items(maxItemsForConst);

    		if (debugMode)
    			std::cout << "OpenCL INFO: Math Op adjusting output buffer for " << maxItemsForConst << " due to OpenCL constant memory restrictions" << std::endl;
		}
		else {
			if (debugMode)
				std::cout << "OpenCL INFO: Math Op using default output buffer of " << imaxItems << "..." << std::endl;
		}

        GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());

        setBufferLength(maxItemsForConst);
    }

    void clMathOp_impl::setBufferLength(int numItems) {
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
    clMathOp_impl::~clMathOp_impl()
    {
    	if (aBuffer)
    		delete aBuffer;

    	if (bBuffer)
    		delete bBuffer;

    	if (cBuffer)
    		delete cBuffer;

    	cleanup();
    }

    void
    clMathOp_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int clMathOp_impl::testCPU(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    	{

        const SComplex *in1 = (const SComplex *) input_items[0];
        const SComplex *in2 = (const SComplex *) input_items[1];
        SComplex *out = (SComplex *) output_items[0];

		for (int i=0;i<noutput_items;i++) {
			out[i].real = in1[i].real * in2[i].real - (in1[i].imag*in2[i].imag);
			out[i].imag = in1[i].real * in2[i].imag + in1[i].imag * in2[i].real;
		}

    	return noutput_items;
    }

    int clMathOp_impl::testOpenCL(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return processOpenCL(noutput_items,ninput_items,input_items, output_items);
    }

    int clMathOp_impl::processOpenCL(int noutput_items,
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
/*
		// Create buffer for A and copy host contents
		cl::Buffer aBuffer = cl::Buffer(
			*context,
			(cl_mem_flags) (CL_MEM_READ_ONLY | optimalBufferType),  //CL_MEM_COPY_HOST_PTR
			noutput_items * dataSize,
			(void *) in1);

		// Create buffer for that uses the host ptr C
		cl::Buffer cBuffer = cl::Buffer(
			*context,
			(cl_mem_flags) (CL_MEM_WRITE_ONLY | optimalBufferType),
			noutput_items * dataSize,
			out);
*/
    	int inputSize = noutput_items*dataSize;
        queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputSize,input_items[0]);



		// Do the work

		// Set kernel args
		kernel->setArg(0, *aBuffer);

		if (numParams == 2) {
//	        const void *in2 = (const void *) input_items[1];
	        /*
			bBuffer = cl::Buffer(
				*context,
				(cl_mem_flags) (CL_MEM_READ_ONLY | optimalBufferType),  //CL_MEM_COPY_HOST_PTR
				noutput_items * dataSize,
				(void *) in2);
			*/
	        queue->enqueueWriteBuffer(*bBuffer,CL_TRUE,0,inputSize,input_items[1]);

			kernel->setArg(1, *bBuffer);
			kernel->setArg(2, *cBuffer);
		}
		else {
			kernel->setArg(1, *cBuffer);
		}

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
/*
    void * output = (void *) queue->enqueueMapBuffer(
        *cBuffer,
        CL_TRUE, // block
        CL_MAP_READ,
        0,
		inputSize);

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
	clMathOp_impl::general_work (int noutput_items,
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

