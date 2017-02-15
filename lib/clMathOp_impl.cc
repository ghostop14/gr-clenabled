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
    clMathOp::make(int idataType, int openCLPlatformType,int operatorType)
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
      return gnuradio::get_initial_sptr
        (new clMathOp_impl(idataType,dsize,openCLPlatformType,operatorType));
    }

    /*
     * The private constructor
     */
    clMathOp_impl::clMathOp_impl(int idataType, size_t dsize,int openCLPlatformType,int operatorType)
      : gr::block("clMathOp",
              gr::io_signature::make(2, 2, dsize),
              gr::io_signature::make(1, 1, dsize)),
			  GRCLBase(idataType, dsize,openCLPlatformType)
    {
    	// Now we set up our OpenCL kernel
        std::string srcStdStr;
        std::string fnName = "";

        numParams = 2;

        switch(dataType) {
        case DTYPE_FLOAT:
        	// Float data type
        	fnName = "op_float";
        	srcStdStr = "__kernel void op_float(__global float * restrict a, __global float * restrict b, __global float * restrict c) {\n";
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

        	case MATHOP_LOG:
                numParams = 1;
        		// restart function... only have 1 param
            	srcStdStr = "__kernel void op_float(__global float * restrict a, __global float * restrict c) {\n";
            	srcStdStr += "    size_t index =  get_global_id(0);\n";
            	srcStdStr += "    c[index] = log(a[index]);\n";
        	break;

        	case MATHOP_LOG10:
                numParams = 1;

        		// restart function... only have 1 param
            	srcStdStr = "__kernel void op_float(__global float * restrict a, __global float * restrict c) {\n";
            	srcStdStr += "    size_t index =  get_global_id(0);\n";
            	srcStdStr += "    c[index] = log10(a[index]);\n";
        	break;
        	}
        	srcStdStr += "}\n";
        break;
        case DTYPE_INT:
        	fnName = "op_int";
        	srcStdStr = "__kernel void op_int(__global int * restrict a, __global int * restrict b, __global int * restrict c) {\n";
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
        	}
        	srcStdStr += "}\n";
        break;
        case DTYPE_COMPLEX:
        	srcStdStr = "struct ComplexStruct {\n";
        	srcStdStr += "float real;\n";
        	srcStdStr += "float imag; };\n";
        	srcStdStr += "typedef struct ComplexStruct SComplex;\n";

        	fnName = "op_complex";
        	srcStdStr += "__kernel void op_complex(__global const SComplex * restrict a, __global const SComplex * restrict b, __global SComplex * restrict c) {\n";
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
            	srcStdStr += "__kernel void op_complex(__global const SComplex * restrict a, __global SComplex * restrict c) {\n";
            	srcStdStr += "    size_t index =  get_global_id(0);\n";
            	srcStdStr += "    c[index].real = a[index].real;\n";
            	srcStdStr += "    c[index].imag = -1.0 * a[index].imag;\n";
        	break;

        	case MATHOP_MULTIPLY_CONJUGATE:
                numParams = 1;
            	srcStdStr += "__kernel void op_complex(__global const SComplex * restrict a, __global SComplex * restrict c) {\n";
            	srcStdStr += "    size_t index =  get_global_id(0);\n";
            	srcStdStr += "    c[index].real = a[index].real;\n";
            	srcStdStr += "    c[index].imag = -1.0 * a[index].imag;\n";
            	srcStdStr += "    float a_r=a[index].real;\n";
            	srcStdStr += "    float a_i=a[index].imag;\n";
            	srcStdStr += "    float b_r=a[index].real;\n";
            	srcStdStr += "    float b_i=-1.0 * a[index].imag;\n";
            	srcStdStr += "    c[index].real = a_r * b_r - (a_i*b_i);\n";
            	srcStdStr += "    c[index].imag = a_r * b_i + a_i * b_r;\n";
        	break;
        	}
        	srcStdStr += "}\n";
        break;
        }

        GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());
    }

    /*
     * Our virtual destructor.
     */
    clMathOp_impl::~clMathOp_impl()
    {
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
            gr_vector_void_star &output_items)
    {
		if (kernel == NULL) {
			return 0;
		}

        const void *in1 = (const void *) input_items[0];
        const void *in2 = (const void *) input_items[1];
        void *out = (void *) output_items[0];

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


		// Do the work

		// Set kernel args
		kernel->setArg(0, aBuffer);
		cl::Buffer bBuffer;

		if (numParams == 2) {
	        const void *in2 = (const void *) input_items[1];
			bBuffer = cl::Buffer(
				*context,
				(cl_mem_flags) (CL_MEM_READ_ONLY | optimalBufferType),  //CL_MEM_COPY_HOST_PTR
				noutput_items * dataSize,
				(void *) in2);

			kernel->setArg(1, bBuffer);
			kernel->setArg(2, cBuffer);
		}
		else {
			kernel->setArg(1, cBuffer);
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
    // the host backing space, remember we choose GPU device.
    void * output = (void *) queue->enqueueMapBuffer(
        cBuffer,
        CL_TRUE, // block
        CL_MAP_READ,
        0,
			noutput_items * dataSize);

    cl_int err;

    // Finally release our hold on accessing the memory
    err = queue->enqueueUnmapMemObject(
        cBuffer,
        (void *) output);

      // Tell runtime system how many input items we consumed on
      // each input stream.
//      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

    int
	clMathOp_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
		if (kernel == NULL) {
			return 0;
		}

        const void *in1 = (const void *) input_items[0];
        void *out = (void *) output_items[0];

		// Create buffer for A and copy host contents
		cl::Buffer aBuffer = cl::Buffer(
			*context,
			CL_MEM_READ_ONLY | optimalBufferType,  //CL_MEM_COPY_HOST_PTR
			noutput_items * dataSize,
			(void *) in1);

		// Create buffer for that uses the host ptr C
		cl::Buffer cBuffer = cl::Buffer(
			*context,
			CL_MEM_WRITE_ONLY | optimalBufferType,
			noutput_items * dataSize,
			out);


		// Do the work

		// Set kernel args
		kernel->setArg(0, aBuffer);
		cl::Buffer bBuffer;

		if (numParams == 2) {
	        const void *in2 = (const void *) input_items[1];
			bBuffer = cl::Buffer(
				*context,
				CL_MEM_READ_ONLY | optimalBufferType,
				noutput_items * dataSize,
				(void *) in2);

			kernel->setArg(1, bBuffer);
			kernel->setArg(2, cBuffer);
		}
		else {
			kernel->setArg(1, cBuffer);
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
    // the host backing space, remember we choose GPU device.
    void * output = (void *) queue->enqueueMapBuffer(
        cBuffer,
        CL_TRUE, // block
        CL_MAP_READ,
        0,
			noutput_items * dataSize);

    cl_int err;

    // Finally release our hold on accessing the memory
    err = queue->enqueueUnmapMemObject(
        cBuffer,
        (void *) output);

      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace clenabled */
} /* namespace gr */

