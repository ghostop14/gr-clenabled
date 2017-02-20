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
#include "clFFT_impl.h"
#include "window.h"
#include "fft.h"
#include <volk/volk.h>

namespace gr {
  namespace clenabled {

    clFFT::sptr
    clFFT::make(int fftSize, int clFFTDir,int idataType, int openCLPlatformType,int setDebug)
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
        (new clFFT_impl(fftSize, clFFTDir,dsize,openCLPlatformType,setDebug));
    }

    /*
     * The private constructor
     */
    clFFT_impl::clFFT_impl(int fftSize, int clFFTDir,int idataType, int dSize, int openCLPlatformType,int setDebug)
      : gr::block("clFFT",
              gr::io_signature::make(1, 1, dSize),
              gr::io_signature::make(1, 1, dSize)),
	  	  	  GRCLBase(DTYPE_COMPLEX, sizeof(gr_complex),openCLPlatformType),
			  d_fft_size(fftSize), d_forward(true), d_shift(true)
    {
        d_fft = new fft_complex(d_fft_size, d_forward, 1);
        d_window=gr::clenabled::window::blackmanharris(fftSize);

    	if (setDebug == 1)
    		debugMode = true;
    	else
    		debugMode = false;


    	// Move type to enum var
    	if (clFFTDir == CLFFT_FORWARD)
    		fftDir = CLFFT_FORWARD;
    	else
    		fftDir = CLFFT_BACKWARD;

    	setBufferLength(fftSize);

    	/* Create a default plan for a complex FFT. */
        size_t clLengths[1];
        clLengths[0]=(size_t)fftSize;

        int err;

        /* Setup clFFT. */
        clfftSetupData fftSetup;
        err = clfftInitSetupData(&fftSetup);
        err = clfftSetup(&fftSetup);

        err = clfftCreateDefaultPlan(&planHandle, (*context)(), dim, clLengths);

        /* Set plan parameters. */
        err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE);

        if (dataType==DTYPE_COMPLEX) {
            err = clfftSetLayout(planHandle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
        }
        else {
            err = clfftSetLayout(planHandle, CLFFT_REAL, CLFFT_REAL);
        }
        //err = clfftSetResultLocation(planHandle, CLFFT_INPLACE);  // In-place puts data back in source queue.  Not what we want.
        err = clfftSetResultLocation(planHandle, CLFFT_OUTOFPLACE);

        //fftQueue = clCreateCommandQueueWithProperties((*context)(), devices[0](), 0, &err);

        /* Bake the plan. */
        err = clfftBakePlan(planHandle, 1, &(*queue)(), NULL, NULL);
    }

    void clFFT_impl::setBufferLength(int numItems) {
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

        curBufferSize=numItems;
    }


    /*
     * Our virtual destructor.
     */
    clFFT_impl::~clFFT_impl()
    {
    	if (aBuffer)
    		delete aBuffer;

    	if (cBuffer)
    		delete cBuffer;
        /* Release the plan. */
    	int err;

        err = clfftDestroyPlan( &planHandle );
       /* Release clFFT library. */
        clfftTeardown( );

        delete d_fft;
    }

    void
    clFFT_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int clFFT_impl::testCPU(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    	{
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        unsigned int input_data_size = input_signature()->sizeof_stream_item (0);
        unsigned int output_data_size = output_signature()->sizeof_stream_item (0);

        int count = 0;

        // int cmax = (int)((float)noutput_items / (float)d_fft_size);

        while(count++ < noutput_items) {
        // copy input into optimally aligned buffer
        if(d_window.size()) {
          gr_complex *dst = d_fft->get_inbuf();
          if(!d_forward && d_shift) {
            unsigned int offset = (!d_forward && d_shift)?(d_fft_size/2):0;
            int fft_m_offset = d_fft_size - offset;
            volk_32fc_32f_multiply_32fc(&dst[fft_m_offset], &in[0], &d_window[0], offset);
            volk_32fc_32f_multiply_32fc(&dst[0], &in[offset], &d_window[offset], d_fft_size-offset);
          }
          else {
            volk_32fc_32f_multiply_32fc(&dst[0], in, &d_window[0], d_fft_size);
          }
        }
        else {
          if(!d_forward && d_shift) {  // apply an ifft shift on the data
            gr_complex *dst = d_fft->get_inbuf();
            unsigned int len = (unsigned int)(floor(d_fft_size/2.0)); // half length of complex array
            memcpy(&dst[0], &in[len], sizeof(gr_complex)*(d_fft_size - len));
            memcpy(&dst[d_fft_size - len], &in[0], sizeof(gr_complex)*len);
          }
          else {
            memcpy(d_fft->get_inbuf(), in, input_data_size);
          }
        }

        // compute the fft
        d_fft->execute();

        // copy result to our output
        if(d_forward && d_shift) {  // apply a fft shift on the data
          unsigned int len = (unsigned int)(ceil(d_fft_size/2.0));
          memcpy(&out[0], &d_fft->get_outbuf()[len], sizeof(gr_complex)*(d_fft_size - len));
          memcpy(&out[d_fft_size - len], &d_fft->get_outbuf()[0], sizeof(gr_complex)*len);
        }
        else {
          memcpy (out, d_fft->get_outbuf (), output_data_size);
        }

        in  += d_fft_size;
        out += d_fft_size;
        }

        return noutput_items;
    }

    int clFFT_impl::testOpenCL(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return processOpenCL(noutput_items,ninput_items,input_items, output_items);
    }

    int clFFT_impl::processOpenCL(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    {
		// have to be careful with the # of points in each iteration.
		// The FFT was set up for FFTSize items, so curBufferSize = FFTSize*2.
		// So we're only going to consume that many at a clip.  If we don't have enough, return 0.
		if (noutput_items < curBufferSize)
			return 0;

		// only taking FFT size items
    	int inputSize = curBufferSize*dataSize;
        queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputSize,input_items[0]);

		// Do the work

        int err;
        /* Execute the plan. */
       	err = clfftEnqueueTransform(planHandle, fftDir, 1, &(*queue)(), 0, NULL, NULL, &(*aBuffer)(), &(*cBuffer)(), NULL);

        /* Wait for calculations to be finished. */
        err = clFinish((*queue)());

        // and outputting FFTSize samples

        /* Fetch results of calculations. */
    	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,inputSize,(void *)output_items[0]);

      // Tell runtime system how many output items we produced.
      return curBufferSize; // will always return FFTSize*2 items.
    }


    int
    clFFT_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      int retVal = processOpenCL(noutput_items,ninput_items,input_items,output_items);

      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return retVal;
    }

  } /* namespace clenabled */
} /* namespace gr */

