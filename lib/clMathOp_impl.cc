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
          (new clMathOp_impl(idataType,dsize,openCLPlatformType,OCLDEVICESELECTOR_FIRST,0,0,operatorType,true));
  	}
  	else {
        return gnuradio::get_initial_sptr
          (new clMathOp_impl(idataType,dsize,openCLPlatformType,OCLDEVICESELECTOR_FIRST,0,0,operatorType,false));
  	}
}

    clMathOp::sptr
    clMathOp::make(int idataType, int openCLPlatformType, int devSelector,int platformId, int devId,int operatorType,int setDebug)
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
          (new clMathOp_impl(idataType,dsize,openCLPlatformType,devSelector,platformId,devId,operatorType,true));
  	}
  	else {
        return gnuradio::get_initial_sptr
          (new clMathOp_impl(idataType,dsize,openCLPlatformType,devSelector,platformId,devId,operatorType,false));
  	}
}

    /*
     * The private constructor
     */
    clMathOp_impl::clMathOp_impl(int idataType, size_t dsize,int openCLPlatformType, int devSelector,int platformId, int devId,int operatorType, bool setDebug)
      : gr::sync_block("clMathOp",
              gr::io_signature::make(2, 2, dsize),
              gr::io_signature::make(1, 1, dsize)),
			  GRCLBase(idataType, dsize,openCLPlatformType,devSelector,platformId,devId,setDebug)
    {
    	// Now we set up our OpenCL kernel

        numParams = 2;
        numConstParams = 2;
        d_operatorType = operatorType;
        curBufferSize = 0;


        // for this module we have to do this calculation after setBufferLength
        // because some streams have 1 param and others have 2.
    	maxConstItems = (int)((float)maxConstMemSize / ((float)dataSize*numConstParams));

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

    void clMathOp_impl::buildKernel(int numItems) {
    	bool useConst;

    	if (numItems > maxConstItems)
    		useConst = false;
    	else
    		useConst = true;

		if (debugMode) {
			if (useConst)
				std::cout << "OpenCL INFO: Math Op building kernel with __constant params..." << std::endl;
			else
				std::cout << "OpenCL INFO: Math Op - too many items for constant memory.  Building kernel with __global params..." << std::endl;
		}

        switch(dataType) {
        case DTYPE_FLOAT:
        	// Float data type
        	fnName = "op_float";

        	if (useConst)
        		srcStdStr = "__kernel void op_float(__constant float * a, __constant float * b, __global float * restrict c) {\n";
        	else
        		srcStdStr = "__kernel void op_float(__global float * restrict a, __global float * restrict b, __global float * restrict c) {\n";

        	if (d_operatorType != MATHOP_EMPTY)
        		srcStdStr += "    size_t index =  get_global_id(0);\n";

        	switch (d_operatorType) {
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

        	}
        	srcStdStr += "}\n";
        break;


        case DTYPE_INT:
        	fnName = "op_int";

        	if (useConst)
        		srcStdStr = "__kernel void op_int(__constant int * a, __constant * b, __global int * restrict c) {\n";
        	else
        		srcStdStr = "__kernel void op_int(__global int * restrict a, __global * restrict b, __global int * restrict c) {\n";

        	if (d_operatorType != MATHOP_EMPTY)
        		srcStdStr += "    size_t index =  get_global_id(0);\n";

        	switch (d_operatorType) {
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

        	if (useConst)
        		srcStdStr += "__kernel void op_complex(__constant SComplex * a, __constant SComplex * b, __global SComplex * restrict c) {\n";
        	else
        		srcStdStr += "__kernel void op_complex(__global SComplex * restrict a, __global SComplex * restrict b, __global SComplex * restrict c) {\n";

        	if (d_operatorType != MATHOP_EMPTY)
        		srcStdStr += "    size_t index =  get_global_id(0);\n";
        	switch (d_operatorType) {
        	case MATHOP_MULTIPLY:
            	srcStdStr += "    float a_r=a[index].real;\n";
            	srcStdStr += "    float a_i=a[index].imag;\n";
            	srcStdStr += "    float b_r=b[index].real;\n";
            	srcStdStr += "    float b_i=b[index].imag;\n";
            	srcStdStr += "    c[index].real = (a_r * b_r) - (a_i*b_i);\n";
            	srcStdStr += "    c[index].imag = (a_r * b_i) + (a_i * b_r);\n";
        	break;
        	case MATHOP_ADD:
            	srcStdStr += "    c[index].real = a[index].real + b[index].real;\n";
            	srcStdStr += "    c[index].imag = a[index].imag + b[index].imag;\n";
        	break;
        	case MATHOP_SUBTRACT:
            	srcStdStr += "    c[index].real = a[index].real - b[index].real;\n";
            	srcStdStr += "    c[index].imag = a[index].imag - b[index].imag;\n";
        	break;

        	case MATHOP_MULTIPLY_CONJUGATE:
                numParams = 2;
            	fnName = "op_complex";

            	srcStdStr = "struct ComplexStruct {\n";
            	srcStdStr += "float real;\n";
            	srcStdStr += "float imag; };\n";
            	srcStdStr += "typedef struct ComplexStruct SComplex;\n";

            	if (useConst)
            		srcStdStr += "__kernel void op_complex(__constant SComplex * a, __constant SComplex * b, __global SComplex * restrict c) {\n";
            	else
            		srcStdStr += "__kernel void op_complex(__global SComplex * restrict a, __global SComplex * restrict b, __global SComplex * restrict c) {\n";

            	srcStdStr += "    size_t index =  get_global_id(0);\n";
            	srcStdStr += "    float a_r=a[index].real;\n";
            	srcStdStr += "    float a_i=a[index].imag;\n";
            	srcStdStr += "    float b_r=b[index].real;\n";
            	srcStdStr += "    float b_i=-1.0 * b[index].imag;\n";
            	srcStdStr += "    c[index].real = (a_r * b_r) - (a_i*b_i);\n";
            	srcStdStr += "    c[index].imag = (a_r * b_i) + (a_i * b_r);\n";
            	numConstParams = 2;
        	break;
        	}
        	srcStdStr += "}\n";
        break;
        }
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

        int imaxItems = numItems;

        if (d_operatorType == MATHOP_LOG || d_operatorType == MATHOP_LOG10 ||
        		d_operatorType == MATHOP_COMPLEX_CONJUGATE || d_operatorType == MATHOP_MULTIPLY_CONJUGATE )
        	numConstParams = 1;
        else
        	numConstParams = 2;

        if (curBufferSize==0) {
        	// when curBufferSize==0 we're in the constructor so do this.
        	// If we call it multiple times it'll keep halfing the output buffer
            try {
            	set_max_noutput_items(maxConstItems/numConstParams);
            }
            catch(...) {

            }
        }

    	buildKernel(numItems);
        GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());

        curBufferSize=numItems;
    }

    bool clMathOp_impl::stop() {
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

    /*
     * Our virtual destructor.
     */
    clMathOp_impl::~clMathOp_impl()
    {
    	if (curBufferSize > 0)
    		stop();
    }

    int clMathOp_impl::testLog10(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    	{

        const float *in = (const float *) input_items[0];
        float *out = (float *) output_items[0];

		for (int i=0;i<noutput_items;i++) {
			out[i] = n * log10(std::max(in[i], (float) 1e-18)) + k;
		}

    	return noutput_items;
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

    	// Protect context from switching
        gr::thread::scoped_lock guard(d_mutex);

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
	clMathOp_impl::work (int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
    	if (debugMode && CLPRINT_NITEMS)
    		std::cout << "clMathOp noutput_items: " << noutput_items << std::endl;

      int retVal = processOpenCL(noutput_items,d_ninput_items,input_items,output_items);

      // Tell runtime system how many output items we produced.
      return retVal;
    }

  } /* namespace clenabled */
} /* namespace gr */

