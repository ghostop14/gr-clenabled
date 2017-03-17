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
    clFFT::make(int fftSize, int clFFTDir,const std::vector<float> &window, int idataType, int openCLPlatformType,int devSelector,int platformId, int devId,int setDebug)
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
			(new clFFT_impl(fftSize, clFFTDir,window,idataType,dsize,openCLPlatformType,devSelector,platformId,devId,true));
      }
      else {
          return gnuradio::get_initial_sptr
            (new clFFT_impl(fftSize, clFFTDir,window,idataType,dsize,openCLPlatformType,devSelector,platformId,devId,false));
      }
    }

    /*
     * The private constructor
     */
    clFFT_impl::clFFT_impl(int fftSize, int clFFTDir,const std::vector<float> &window,int idataType, int dSize, int openCLPlatformType,int devSelector,int platformId, int devId,bool setDebug)
      : gr::sync_block("clFFT",
              gr::io_signature::make(1, 1, fftSize*dSize),
              gr::io_signature::make(1, 1, fftSize*dSize)),
	  	  	  GRCLBase(DTYPE_COMPLEX, sizeof(gr_complex),openCLPlatformType,devSelector,platformId,devId,setDebug),
			  d_fft_size(fftSize), d_forward(true), d_shift(false),d_window(window)
    {
        d_fft = new fft_complex(d_fft_size, d_forward, 1);

    	// Move type to enum var
    	if (clFFTDir == CLFFT_FORWARD)
    		fftDir = CLFFT_FORWARD;
    	else
    		fftDir = CLFFT_BACKWARD;

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

        // using vectors we don't want to change the output multiple since 1 item will be an fft worth of data.
        //    	set_output_multiple(fftSize);

        /* Bake the plan. */
        err = clfftBakePlan(planHandle, 1, &(*queue)(), NULL, NULL);

    	setBufferLength(fftSize);
}

    void clFFT_impl::setBufferLength(int numItems) {
    	if (aBuffer)
    		delete aBuffer;

    	if (cBuffer)
    		delete cBuffer;

        curBufferSize=numItems;
    	inputSize = curBufferSize*dataSize; // curBufferSize will be fftSize

    	if (windowBuffer) {
        	if (dataType==DTYPE_COMPLEX) {
        		delete[] (gr_complex *)windowBuffer;
        	}
        	else {
        		delete[] (float *)windowBuffer;
        	}

    	}

    	if (dataType==DTYPE_COMPLEX) {
    		windowBuffer = (void *)new gr_complex[curBufferSize];
    	}
    	else {
    		windowBuffer = (void* )new float[curBufferSize];
    	}


    	aBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_ONLY,  // this has to be read/write of the clFFT enqueue transform call crashes.
			inputSize);

        cBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_WRITE,
			inputSize);
    }


    /*
     * Our virtual destructor.
     */
    clFFT_impl::~clFFT_impl()
    {
    	if (curBufferSize > 0)
    		stop();
    }

    bool clFFT_impl::stop() {
    	std::cout << "Calling Stop." << std::endl;

    	curBufferSize = 0;
        /* Release the plan. */
    	int err;


    	if (windowBuffer) {
        	if (dataType==DTYPE_COMPLEX) {
        		delete[] (gr_complex *)windowBuffer;
        	}
        	else {
        		delete[] (float *)windowBuffer;
        	}

        	windowBuffer = NULL;
    	}

        err = clfftDestroyPlan( &planHandle );
       /* Release clFFT library. */
        clfftTeardown( );

    	if (aBuffer) {
    		delete aBuffer;
    		aBuffer = NULL;
    	}

    	if (cBuffer) {
    		delete cBuffer;
    		cBuffer = NULL;
    	}

    	delete d_fft;
    	d_fft = NULL;

    	return GRCLBase::stop();
    }

    void
    clFFT_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int clFFT_impl::testCPU(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    	{
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

        int count = 0;

        while(count < noutput_items) {
			// copy input into optimally aligned buffer
            if(d_window.size()) {
    			  gr_complex *dst = d_fft->get_inbuf();
    			  if(!d_forward && d_shift) {
    				unsigned int offset = (!d_forward && d_shift)?(d_fft_size/2):0;
    				int fft_m_offset = d_fft_size - offset;
    				volk_32fc_32f_multiply_32fc(&dst[fft_m_offset], in, &d_window[0], offset);
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
    				memcpy(&dst[d_fft_size - len], &in[count], sizeof(gr_complex)*len);
    			  }
    			  else {
    				memcpy(d_fft->get_inbuf(), &in[count], inputSize);
    			  }
            }  // if-else d_window.size();

			// compute the fft
			d_fft->execute();

			// copy result to our output
	        if(d_forward && d_shift) {  // apply a fft shift on the data
	          unsigned int len = (unsigned int)(ceil(d_fft_size/2.0));
	          memcpy(&out[0], &d_fft->get_outbuf()[len], sizeof(gr_complex)*(d_fft_size - len));
	          memcpy(&out[d_fft_size - len], &d_fft->get_outbuf()[0], sizeof(gr_complex)*len);
	        }
	        else {
				memcpy (out, d_fft->get_outbuf (), inputSize);
	        }

			in  += d_fft_size;
			out += d_fft_size;
			count += d_fft_size;
        }

        return noutput_items;
    	}
 /*
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

        while(count++ < noutput_items) {
        // copy input into optimally aligned buffer
        if(d_window.size()) {
			  gr_complex *dst = d_fft->get_inbuf();
			  if(!d_forward && d_shift) {
				unsigned int offset = (!d_forward && d_shift)?(d_fft_size/2):0;
				int fft_m_offset = d_fft_size - offset;
				volk_32fc_32f_multiply_32fc(&dst[fft_m_offset], &in[count], &d_window[0], offset);
				volk_32fc_32f_multiply_32fc(&dst[0], &in[count+offset], &d_window[offset], d_fft_size-offset);
			  }
			  else {
					volk_32fc_32f_multiply_32fc(&dst[0], &in[count], &d_window[0], d_fft_size);
			  }
        }
        else {
			  if(!d_forward && d_shift) {  // apply an ifft shift on the data
				gr_complex *dst = d_fft->get_inbuf();
				unsigned int len = (unsigned int)(floor(d_fft_size/2.0)); // half length of complex array
				memcpy(&dst[0], &in[len], sizeof(gr_complex)*(d_fft_size - len));
				memcpy(&dst[d_fft_size - len], &in[count], sizeof(gr_complex)*len);
			  }
			  else {
				memcpy(d_fft->get_inbuf(), &in[count], input_data_size);
			  }
        }  // if-else d_window.size();

        // compute the fft
        d_fft->execute();

        // copy result to our output
        if(d_forward && d_shift) {  // apply a fft shift on the data
          unsigned int len = (unsigned int)(ceil(d_fft_size/2.0));
          memcpy(&out[0], &d_fft->get_outbuf()[len], sizeof(gr_complex)*(d_fft_size - len));
          memcpy(&out[d_fft_size - len], &d_fft->get_outbuf()[0], sizeof(gr_complex)*len);
        }
        else {
           memcpy ((void *)&out[count], d_fft->get_outbuf (), output_data_size);
        }

//        in  += d_fft_size;
//        out += d_fft_size;
        count += d_fft_size;
        }

        return noutput_items;
    }
*/

    int clFFT_impl::testOpenCL(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return processOpenCL(noutput_items,input_items, output_items);
    }

    int clFFT_impl::processOpenCL(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    {
    	// using input vectors guarantees that noutput_items will be a multiple of fftSize
    	// Need to cycle through.

		// only taking FFT size items

        int count = 0;
        int i;
        int err;
        const gr_complex *in_complex = (const gr_complex *) input_items[0];
        gr_complex *out_complex = (gr_complex *) output_items[0];
        const float *in_float = (const float *) input_items[0];
        float *out_float = (float *) output_items[0];

        while(count < noutput_items) {
            // Apply the window if there is one, otherwise just copy to the buffer.
        	// This is the same logic as in the native FFT class, rather than making 2 GPU calls,
        	// These single multiply ops are faster on the CPU.
            if(d_window.size()) {
            	if (dataType==DTYPE_COMPLEX) {
            		volk_32fc_32f_multiply_32fc((gr_complex *)windowBuffer, &in_complex[count], &d_window[0], d_fft_size);
            	}
            	else {
            		float *outBuff;
            		const float *inBuff;
            		inBuff=&in_float[count];
            		outBuff=(float *)windowBuffer;

            		for (i=0;i<d_fft_size;i++) {
            			outBuff[i] = inBuff[i] * d_window[i];
            		}
            	}
                queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputSize,(void *)windowBuffer);
            }
            else {
            	// No window, just copy the data
            	if (dataType==DTYPE_COMPLEX) {
                    queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputSize,(void *)&in_complex[count]);
            	}
            	else {
                    queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputSize,(void *)&in_float[count]);
            	}
            }

            // Execute the plan.
           	err = clfftEnqueueTransform(planHandle, fftDir, 1, &((*queue)()), 0, NULL, NULL, &((*aBuffer)()), &((*cBuffer)()), NULL);
            // Wait for calculations to be finished.
            err = clFinish((*queue)());
            // Fetch results of calculations.
        	if (dataType==DTYPE_COMPLEX) {
            	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,inputSize,(void *)&out_complex[count]);
        	}
        	else {
            	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,inputSize,(void *)&out_float[count]);
        	}

        	count += curBufferSize; // this will be fftsize*batchsize which for now is 1, so curBufferSize = fftsize
        }

      // Tell runtime system how many output items we produced.
      return noutput_items; // will always return FFTSize*2 items.
    }


    int
    clFFT_impl::work(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    {
      // for vectors coming in, noutput_items will be the number of vectors.
      // So for the calculation we need to multiply # of vectors * fft_size to get the # of data points

      int inputSize = noutput_items * d_fft_size;

      int retVal = processOpenCL(inputSize,input_items,output_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace clenabled */
} /* namespace gr */

