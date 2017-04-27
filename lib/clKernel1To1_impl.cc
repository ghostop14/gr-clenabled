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
#include "clKernel1To1_impl.h"
#include <fstream>

namespace gr {
  namespace clenabled {

    clKernel1To1::sptr
    clKernel1To1::make(int idataType, int openCLPlatformType, int devSelector,int platformId, int devId, const char *kernelFnName, const char *filename, int setDebug)
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

      if (setDebug == 1)
		  return gnuradio::get_initial_sptr
			(new clKernel1To1_impl(idataType, dsize, openCLPlatformType, devSelector, platformId, devId, kernelFnName, filename, true));
      else
		  return gnuradio::get_initial_sptr
			(new clKernel1To1_impl(idataType, dsize, openCLPlatformType, devSelector, platformId, devId, kernelFnName, filename, false));

    }

    /*
     * The private constructor
     */
    clKernel1To1_impl::clKernel1To1_impl(int idataType, int iDataSize, int openCLPlatformType, int devSelector,int platformId, int devId,
    		const char *kernelFnName, const char *filename, bool setDebug)
      : gr::sync_block("clKernel1To1",
              gr::io_signature::make(1,1, iDataSize),
              gr::io_signature::make(1,1, iDataSize)),
			  GRCLBase(idataType, iDataSize,openCLPlatformType,devSelector,platformId,devId,setDebug)
    {
		std::string fileline;

		srcStdStr = "";
		fnName = kernelFnName;
		std::ifstream infile;

		try {
			infile.open (filename);
		}
		catch (...) {
			std::cout << "ERROR opening kernel file: " << filename << std::endl;
			exit(1);
		}

		std::stringstream buffer;
		buffer << infile.rdbuf();
		srcStdStr = buffer.str();

		if (debugMode) {
			std::cout << "Function name: " << kernelFnName << std::endl;
		}

		infile.close();

    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

        setBufferLength(imaxItems);

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

    void clKernel1To1_impl::setBufferLength(int numItems) {
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

        GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());

        curBufferSize=numItems;
    }

    bool clKernel1To1_impl::stop() {
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


    /*
     * Our virtual destructor.
     */
    clKernel1To1_impl::~clKernel1To1_impl()
    {
    	stop();
    }

    int clKernel1To1_impl::processOpenCL(int noutput_items,
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

        queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputSize,input_items[0]);

		// Do the work

		// Set kernel args
		kernel->setArg(0, *aBuffer);
		kernel->setArg(1, *cBuffer);

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

		queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,inputSize,(void *)output_items[0]);

		return noutput_items;
    }

    int
    clKernel1To1_impl::work (int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
    	if (debugMode && CLPRINT_NITEMS)
    		std::cout << "clKernel1To1 noutput_items: " << noutput_items << std::endl;

        int retVal = processOpenCL(noutput_items,d_ninput_items,input_items,output_items);

        // Tell runtime system how many output items we produced.
        return retVal;
    }

  } /* namespace clenabled */
} /* namespace gr */

