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
    clMathConst::make(int idataType,int openCLPlatformType,float fValue,int operatorType)
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

	      return gnuradio::get_initial_sptr
	        (new clMathConst_impl(idataType, dsize,openCLPlatformType,fValue,operatorType));
    }

    /*
     * The private constructor
     */
    clMathConst_impl::clMathConst_impl(int idataType, size_t dsize,int openCLPlatformType,float fValue,int operatorType)
      : gr::block("clMathConst",
              gr::io_signature::make(1, 1, dsize),
              gr::io_signature::make(1, 1, dsize)),
			  GRCLBase(idataType, dsize,openCLPlatformType)
	{
	value = fValue;
	mathOperatorType = operatorType;

	// Now we set up our OpenCL kernel
	std::string srcStdStr;
	std::string fnName = "";

	switch(dataType) {
	case DTYPE_FLOAT:
		switch (mathOperatorType) {
		case MATHOP_MULTIPLY:
		srcStdStr = "__kernel void multconst_float(__global float * restrict a, float multiplier, __global float * restrict c) {\n";
		srcStdStr += "    size_t index =  get_global_id(0);\n";
		srcStdStr += "    c[index] = a[index] * multiplier;\n";
		srcStdStr += "}\n";
		fnName = "multconst_float";
		break;

		case MATHOP_ADD:
		srcStdStr = "__kernel void addconst_float(__global float * restrict a, float multiplier, __global float * restrict c) {\n";
		srcStdStr += "    size_t index =  get_global_id(0);\n";
		srcStdStr += "    c[index] = a[index] + multiplier;\n";
		srcStdStr += "}\n";
		fnName = "addconst_float";
		break;
		}
	break;
	case DTYPE_INT:
		switch (mathOperatorType) {
		case MATHOP_MULTIPLY:
		srcStdStr = "__kernel void multonst_int(__global int * restrict a, int multiplier, __global int * restrict c) {\n";
		srcStdStr += "    size_t index =  get_global_id(0);\n";
		srcStdStr += "    c[index] = a[index] * multiplier;\n";
		srcStdStr += "}\n";
		fnName = "multconst_int";
		break;

		case MATHOP_ADD:
		srcStdStr = "__kernel void addonst_int(__global int * restrict a, int multiplier, __global int * restrict c) {\n";
		srcStdStr += "    size_t index =  get_global_id(0);\n";
		srcStdStr += "    c[index] = a[index] + multiplier;\n";
		srcStdStr += "}\n";
		fnName = "addconst_int";
		break;
		}
	break;
	case DTYPE_COMPLEX:
		switch (mathOperatorType) {
		case MATHOP_MULTIPLY:
		srcStdStr = "struct ComplexStruct {\n";
		srcStdStr += "float real;\n";
		srcStdStr += "float imag; };\n";
		srcStdStr += "typedef struct ComplexStruct SComplex;\n";
		srcStdStr += "__kernel void multconst_complex(__global SComplex * restrict a, float multiplier, __global SComplex * restrict c) {\n";
		srcStdStr += "    size_t index =  get_global_id(0);\n";
		srcStdStr += "    c[index].real = a[index].real * multiplier;\n";
		srcStdStr += "    c[index].imag = a[index].imag * multiplier;\n";
		srcStdStr += "}\n";
		fnName = "multconst_complex";
		break;
		case MATHOP_ADD:
		srcStdStr = "struct ComplexStruct {\n";
		srcStdStr += "float real;\n";
		srcStdStr += "float imag; };\n";
		srcStdStr += "typedef struct ComplexStruct SComplex;\n";
		srcStdStr += "__kernel void addconst_complex(__global SComplex * restrict a, SComplex val, __global SComplex * restrict c) {\n";
		srcStdStr += "    size_t index =  get_global_id(0);\n";
		srcStdStr += "    c[index].real = a[index].real + val.real;\n";
		srcStdStr += "    c[index].imag = a[index].imag + val.imag;\n";
		srcStdStr += "}\n";
		fnName = "addconst_complex";
		break;
		}
	break;
	}

	GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());
	}


    /*
     * Our virtual destructor.
     */
    clMathConst_impl::~clMathConst_impl()
    {
    	cleanup();
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
            gr_vector_void_star &output_items)
    {
    	if (kernel == NULL) {
    		return 0;
    	}
        const gr_complex *in = (const gr_complex *)input_items[0];
        gr_complex *out = (gr_complex *)output_items[0];
/*
        const float *in = (const float *) input_items[0];
        float *out = (float *) output_items[0];

    	for (int i=0;i<noutput_items;i++) {
    		// out[i]=in[i] * value;
    		out[i]=in[i];
    	}

    	consume_each(noutput_items);
    	return noutput_items;
*/
		// const SComplex *in = (const SComplex *) &input_items[0];
		// SComplex *out = (SComplex *) &output_items[0];

        // Create buffer for A and copy host contents
        cl::Buffer aBuffer = cl::Buffer(
            *context,
            CL_MEM_READ_ONLY | optimalBufferType,
			noutput_items * dataSize,
            (void *) input_items[0]);

        // Create buffer for that uses the host ptr C
        cl::Buffer cBuffer = cl::Buffer(
            *context,
            CL_MEM_WRITE_ONLY | optimalBufferType,
			noutput_items * dataSize,
            (void *) output_items[0]);

        // Set kernel args
        kernel->setArg(0, aBuffer);

        kernel->setArg(1, value);
        kernel->setArg(2, cBuffer);

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

    	if (kernel == NULL) {
    		return 0;
    	}

        // Create buffer for A and copy host contents
        cl::Buffer aBuffer = cl::Buffer(
            *context,
            CL_MEM_READ_ONLY | optimalBufferType,
			noutput_items * dataSize,
            (void *) input_items[0]);

        // Create buffer for that uses the host ptr C
        cl::Buffer cBuffer = cl::Buffer(
            *context,
            CL_MEM_WRITE_ONLY | optimalBufferType,
			noutput_items * dataSize,
            (void *) output_items[0]);

        // Set kernel args
        kernel->setArg(0, aBuffer);

        kernel->setArg(1, value);
        kernel->setArg(2, cBuffer);

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

