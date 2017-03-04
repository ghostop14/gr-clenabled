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

#ifndef INCLUDED_CLENABLED_CLFILTER_IMPL_H
#define INCLUDED_CLENABLED_CLFILTER_IMPL_H

#include <clenabled/clFilter.h>
#include "fft_filter.h"
#include "GRCLBase.h"
#include <clFFT.h>

namespace gr {
  namespace clenabled {

    class clFilter_impl : public clFilter, public fft_filter_ccf, public GRCLBase
    {
     private:
        bool d_updated;


        std::vector<float> d_new_taps;


        float *transformedTaps_float=NULL;
        std::vector<float>  d_tail_float;	    // state carried between blocks for overlap-add

        std::string kernelCode;
        std::string kernelCodeWithConst;  // only used if size of filter array is less than constant memory max space.

		cl::Buffer *aBuffer=NULL;
		cl::Buffer *bBuffer=NULL;
		cl::Buffer *cBuffer=NULL;

		// clFFT
		bool hasInitialized_clFFT = false;

		clfftPlanHandle planHandle;
		clfftDim dim = CLFFT_1D;

        void setFilterVariables(int noutput_items);
        int prevTaps=0;
        int prevInputLength=0;  // numinputs

        // Precalculated values
        int inputLengthBytes;
        int  resultLengthPoints;
        // int resultLengthAlignedWithTaps;
        int paddedBufferLengthBytes;
        int filterLengthBytes;

		int paddingLength;
		int paddingBytes;

		char *zeroBuff=NULL;
		char *tmpFFTBuff=NULL;
        void *ifftBuff=NULL;

		int curBufferSize=0;
        void setBufferLength(int numItems);

		void * paddedInputPtr = NULL;
		void * paddedResultPtr = NULL;
		float * filterPtr = NULL;

    	virtual int filterCPU(int noutput_items,
                gr_vector_const_void_star &input_items,
                gr_vector_void_star &output_items);
    	virtual int filterCPU2(int noutput_items,
                gr_vector_const_void_star &input_items,
                gr_vector_void_star &output_items);
    	virtual int filterGPU(int ninput_items,
                gr_vector_const_void_star &input_items,
                gr_vector_void_star &output_items);
    	virtual int filterGPUTimeDomain(int ninput_items,
                gr_vector_const_void_star &input_items,
                gr_vector_void_star &output_items);
    	virtual int filterGPUFrequencyDomain(int ninput_items,
                gr_vector_const_void_star &input_items,
                gr_vector_void_star &output_items);
     public:
    	clFilter_impl(int openclPlatform, int decimation,
              const std::vector<float> &taps,
              int nthreads=1, bool setDebug=false);
      virtual ~clFilter_impl();

      int testCPU(int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items);
      int testOpenCL(int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items);

      int freqDomainSampleBlockSize() { return d_nsamples; };
      int getCurrentBufferSize() { return curBufferSize; };

      void TestNotifyNewFilter(int noutput_items);
      virtual void set_taps2(const std::vector<float> &taps);

      // overrides for fft_filter_ccf implementation to support complex and floats
      virtual int set_taps(const std::vector<float> &taps);
      virtual void compute_sizes(int ntaps);


      virtual void set_nthreads(int n);
      virtual std::vector<float> taps() const;

      int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace clenabled
} // namespace gr

#endif /* INCLUDED_CLENABLED_CLFILTER_IMPL_H */

