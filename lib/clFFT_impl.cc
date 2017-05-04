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
        // THere's no trig in the transforms and there are precomputed "twiddles" but they're done with
        // double precision.  SINGLE/DOUBLE here really just refers to the data type for the math.
        err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE);

        if (dataType==DTYPE_COMPLEX) {
            err = clfftSetLayout(planHandle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
        }
        else {
            err = clfftSetLayout(planHandle, CLFFT_REAL, CLFFT_REAL);
        }

    	clfftSetPlanScale(planHandle, CLFFT_FORWARD, 1.0f); // 1.0 is the default but don't assume.
    	clfftSetPlanScale(planHandle, CLFFT_BACKWARD, 1.0f); // By default the backward scale is set to 1/N so you have to set it here.

        //err = clfftSetResultLocation(planHandle, CLFFT_INPLACE);  // In-place puts data back in source queue.  Not what we want.
        err = clfftSetResultLocation(planHandle, CLFFT_OUTOFPLACE);

        // using vectors we don't want to change the output multiple since 1 item will be an fft worth of data.
        //    	set_output_multiple(fftSize);

        /* Bake the plan. */
        err = clfftBakePlan(planHandle, 1, &(*queue)(), NULL, NULL);

        // Now let's set up for batch processing of blocks
        maxBatchSize = 1;
    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

    	maxBatchSize = (int)ceil((float)imaxItems / d_fft_size);

    	setBufferLength(fftSize);
}

    void clFFT_impl::setBufferLength(int numItems) {
    	if (aBuffer)
    		delete aBuffer;

    	if (cBuffer)
    		delete cBuffer;

        curBufferSize=numItems;
        fft_times_data_size = curBufferSize*dataSize; // curBufferSize will be fftSize
        fft_times_data_times_batch = fft_times_data_size * maxBatchSize;

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
			fft_times_data_times_batch);

        cBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_WRITE,
			fft_times_data_times_batch);
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

    int clFFT_impl::FFTValidationTest(bool fwdXForm) {
    	gr_complex input_items[d_fft_size];
    	gr_complex output_items_gnuradio[d_fft_size];
    	gr_complex output_items_opencl[d_fft_size];

    	int i;

    	// Create some data
    	float frequency_signal = 10;
    	float frequency_sampling = d_fft_size*frequency_signal;
    	float curPhase = 0.0;
    	float signal_ang_rate = 2*M_PI*frequency_signal / frequency_sampling;

    	for (i=0;i<d_fft_size;i++) {
			input_items[i] = gr_complex(sin(curPhase+(signal_ang_rate*i)),cos(curPhase+(signal_ang_rate*i)));
			output_items_gnuradio[i] = gr_complex(0.0,0.0);
			output_items_opencl[i] = gr_complex(0.0,0.0);
    	}


        const gr_complex *in = (const gr_complex *) &input_items[0];
        gr_complex *out = (gr_complex *) &output_items_gnuradio[0];

		std::cout << "Running native transform..." << std::endl;
        // Run an FFT with GNURadio code
		memcpy(d_fft->get_inbuf(), in, d_fft_size*sizeof(gr_complex));
		d_fft->execute();
		memcpy (out, d_fft->get_outbuf (), d_fft_size*sizeof(gr_complex));

        // Run an FFT with OpenCL code
		out = (gr_complex *) &output_items_opencl[0];

    	// Note: inputSize is set in setBufferLength().  inputSize = fft_size * sizeof(data type)
        queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,d_fft_size*sizeof(gr_complex),(void *)&in[0]);

		std::cout << "Running OpenCL transform..." << std::endl;

        int err;
    	err = clfftEnqueueTransform(planHandle, fftDir, 1, &((*queue)()), 0, NULL, NULL, &((*aBuffer)()), &((*cBuffer)()), NULL);
        if( err != CL_SUCCESS ){
        	std::cout << "OpenCL FFT Error enqueuing transform.  Error code " << err << std::endl;
        	exit(1);
        }
        // Wait for calculations to be finished.
        err = clFinish((*queue)());
        if( err != CL_SUCCESS ){
        	std::cout << "OpenCL FFT Error finishing transform.  Error code " << err << std::endl;
        	exit(1);
        }
        // Fetch results of calculations.
    	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,d_fft_size*sizeof(gr_complex),(void *)&out[0]);

        // Compare results

		float o_r_gnuradio,o_i_gnuradio,o_r_opencl,o_i_opencl;
		float diff_real,diff_imag;

		std::cout << "Comparing transform results..." << std::endl;
		bool resmatched=true;
		int numMismatched = 0;
		float real_ratio, imag_ratio;

		for(i=0;i<d_fft_size;i++) {
			o_r_gnuradio = round(output_items_gnuradio[i].real(),12);
			o_i_gnuradio = round(output_items_gnuradio[i].imag(),12);
			o_r_opencl = round(output_items_opencl[i].real(),12);
			o_r_opencl = round(output_items_opencl[i].imag(),12);

			diff_real = fabs(o_r_gnuradio - o_r_opencl);
			diff_imag = fabs(o_i_gnuradio - o_i_opencl);
			real_ratio = fabs(o_r_gnuradio / o_r_opencl);
			imag_ratio = fabs(o_i_gnuradio / o_i_opencl);

			if (diff_real > 0.0 || diff_imag > 0.0) {
				resmatched = false;
				std::cout << std::fixed << std::setw(12)
			    << std::setprecision(12) << "Output varied for sample " << i << ": real diff=" << diff_real <<
					/* " ratio: " << real_ratio << */ " imag diff=" << diff_imag << /* " imag ratio: " << imag_ratio << */ std::endl;
				numMismatched++;
			}
		}

		std::cout << "Last Sample: " << std::endl;
		std::cout << "GNURadio: real = " << std::fixed << std::setw(12) << std::setprecision(12) <<
				output_items_gnuradio[d_fft_size-1].real() << " imag: " << output_items_gnuradio[d_fft_size-1].imag() << std::endl;
		std::cout << "OpenCL: real = "<< std::fixed << std::setw(12) << std::setprecision(12) <<
				output_items_opencl[d_fft_size-1].real() << " imag: " << output_items_opencl[d_fft_size-1].imag() << std::endl;

		if (resmatched) {
			std::cout << "Results matched." << std::endl;
		}
		else {
			std::cout << numMismatched << " out of " << d_fft_size << " didn't match." << std::endl;

		}
    }

    float clFFT_impl::round(float input, int precision) {
    	float num = input*pow(10.0,(float)precision);
    	long long part = (long long)num;

    	return ((float)part)/pow(10.0,(float)precision);
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
    				memcpy(d_fft->get_inbuf(), &in[count], fft_times_data_size);
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
				memcpy (out, d_fft->get_outbuf (), fft_times_data_size);
	        }

			in  += d_fft_size;
			out += d_fft_size;
			count += d_fft_size;
        }

        return noutput_items;
    	}

    int clFFT_impl::testOpenCL(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return processOpenCL(noutput_items,input_items, output_items);
    }

/*
    int clFFT_impl::processOpenCL(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    {
    	// Protect context from switching
        gr::thread::scoped_lock guard(d_mutex);

    	// using input vectors guarantees that noutput_items will be a multiple of fftSize
    	// Need to cycle through.

        int count = 0;
        int i;
        int err;
        const gr_complex *in_complex = (const gr_complex *) input_items[0];
        gr_complex *out_complex = (gr_complex *) output_items[0];
        const float *in_float = (const float *) input_items[0];
        float *out_float = (float *) output_items[0];

        if (ceil(noutput_items/d_fft_size) > maxBatchSize) {
        	// shouldn't happen but we'll need to increase the buffers
        	setBufferLength(noutput_items);
        }

        int localBatchSize = noutput_items/d_fft_size;  // noutput_items should be a multiple of fft_size
        int localdatasize = fft_times_data_size * localBatchSize;

        if(d_window.size() > 0) {
        	// We have to go through the window in blocks before we can run the transform
        	int buffIndex = 0;

            while(count < noutput_items) {
                // Apply the window if there is one, otherwise just copy to the buffer.
            	// This is the same logic as in the native FFT class, rather than making 2 GPU calls,
            	// These single multiply ops are faster on the CPU.
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

            	// Queue the windowed blocks into the buffer
                queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,buffIndex,fft_times_data_size,(void *)windowBuffer);
                buffIndex += d_fft_size;

            	count += d_fft_size;
            }

        }
        else {
        	// just run the transform
        	// No window, just copy the data
        	if (dataType==DTYPE_COMPLEX) {
                queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,localdatasize,(void *)&in_complex[count]);
        	}
        	else {
                queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,localdatasize,(void *)&in_float[count]);
        	}
        }

        // Execute the plan.

        // https://community.amd.com/thread/160016  backward transform
    	err = clfftEnqueueTransform(planHandle, fftDir, 1, &((*queue)()), 0, NULL, NULL, &((*aBuffer)()), &((*cBuffer)()), NULL);
        // Wait for calculations to be finished.
        err = clFinish((*queue)());

        // Fetch results of calculations.
    	if (dataType==DTYPE_COMPLEX) {
        	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,localdatasize,(void *)&out_complex[count]);
    	}
    	else {
        	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,localdatasize,(void *)&out_float[count]);
    	}

      // Tell runtime system how many output items we produced.
      return noutput_items; // will always return FFTSize*2 items.
    }
*/
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


    	// Protect context from switching
        gr::thread::scoped_lock guard(d_mutex);

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
                queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,fft_times_data_size,(void *)windowBuffer);
            }
            else {
            	// No window, just copy the data
            	if (dataType==DTYPE_COMPLEX) {
                    queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,fft_times_data_size,(void *)&in_complex[count]);
            	}
            	else {
                    queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,fft_times_data_size,(void *)&in_float[count]);
            	}
            }

            // Execute the plan.

            // https://community.amd.com/thread/160016  backward transform
        	err = clfftEnqueueTransform(planHandle, fftDir, 1, &((*queue)()), 0, NULL, NULL, &((*aBuffer)()), &((*cBuffer)()), NULL);
            // Wait for calculations to be finished.
            err = clFinish((*queue)());
            // Fetch results of calculations.
        	if (dataType==DTYPE_COMPLEX) {
            	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,fft_times_data_size,(void *)&out_complex[count]);
        	}
        	else {
            	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,fft_times_data_size,(void *)&out_float[count]);
        	}

        	count += d_fft_size;
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

  	if (debugMode && CLPRINT_NITEMS)
  		std::cout << "clFFT_impl inputSize: " << inputSize << std::endl;

      int retVal = processOpenCL(inputSize,input_items,output_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace clenabled */
} /* namespace gr */

