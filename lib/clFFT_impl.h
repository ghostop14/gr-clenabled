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

#ifndef INCLUDED_CLENABLED_CLFFT_IMPL_H
#define INCLUDED_CLENABLED_CLFFT_IMPL_H

#include <clenabled/clFFT.h>
#include "GRCLBase.h"
#include <clFFT.h>
#include "fft.h"

namespace gr {
  namespace clenabled {

    class clFFT_impl : public clFFT, public GRCLBase
    {
    	// Good example of using clFFT:
    	// https://dournac.org/info/fft_gpu

     private:

        gr::clenabled::fft_complex          *d_fft;
        unsigned int          d_fft_size;
        std::vector<float>    d_window;
        bool                  d_forward;
        bool                  d_shift;

		cl::Buffer *aBuffer=NULL;
		cl::Buffer *cBuffer=NULL;
		int curBufferSize=0;
		int fft_times_data_size;
		int fft_times_data_times_batch;
		int maxBatchSize;
		void *windowBuffer=NULL;

		// clFFT
		clfftPlanHandle planHandle;
		clfftDim dim = CLFFT_1D;
		clfftDirection fftDir = CLFFT_FORWARD;  // options are CLFFT_FORWARD (-1) or CLFFT_BACKWARD (1)  [see clFFT.h]

		void setBufferLength(int numItems);

		float round(float input, int precision);

     public:
		int FFTValidationTest(bool fwdXForm);

    int testCPU(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items);

    int processOpenCL(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items);

    int testOpenCL(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items);

      clFFT_impl(int fftSize, int clFFTDir,const std::vector<float> &window,int idataType, int dSize, int openCLPlatformType,int devSelector,int platformId, int devId,bool setDebug=false);
      virtual ~clFFT_impl();

      virtual bool stop();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      virtual int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace clenabled
} // namespace gr

#endif /* INCLUDED_CLENABLED_CLFFT_IMPL_H */

