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
#include "clSignalSource_impl.h"

namespace gr {
  namespace clenabled {

    clSignalSource::sptr
    clSignalSource::make(int idataType, int openCLPlatformType, int devSelector,int platformId, int devId, float samp_rate,int waveform, float freq, float amplitude,int setDebug)
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
			(new clSignalSource_impl(idataType, dsize, openCLPlatformType, devSelector, platformId, devId, samp_rate, waveform, freq, amplitude, true));
      else
		  return gnuradio::get_initial_sptr
			(new clSignalSource_impl(idataType, dsize, openCLPlatformType, devSelector, platformId, devId, samp_rate, waveform, freq, amplitude, false));

    }

    /*
     * The private constructor
     */
    clSignalSource_impl::clSignalSource_impl(int idataType, int iDataSize, int openCLPlatformType, int devSelector,int platformId, int devId, float samp_rate,int waveform, float freq, float amplitude,bool setDebug)
      : gr::block("clSignalSource",
              gr::io_signature::make(1, 1, sizeof(iDataSize)),
              gr::io_signature::make(1, 1, sizeof(iDataSize))),
			  GRCLBase(idataType, iDataSize,openCLPlatformType,devSelector,platformId,devId,setDebug)
    {
    	setBufferLength(8192);
    }

    void clSignalSource_impl::setBufferLength(int numItems) {
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

    bool clSignalSource_impl::stop() {
    	return true;
    }
    /*
     * Our virtual destructor.
     */
    clSignalSource_impl::~clSignalSource_impl()
    {
    	stop();
    }

    void
    clSignalSource_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int clSignalSource_impl::processOpenCL(int noutput_items,
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

		queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,inputSize,(void *)output_items[0]);

		return noutput_items;
    }

    int
    clSignalSource_impl::general_work (int noutput_items,
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

