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

#ifndef INCLUDED_CLENABLED_CLCOSTASLOOP_IMPL_H
#define INCLUDED_CLENABLED_CLCOSTASLOOP_IMPL_H

#include <clenabled/clCostasLoop.h>
#include "GRCLBase.h"

namespace gr {
  namespace clenabled {

    class clCostasLoop_impl : public clCostasLoop, public GRCLBase
    {
     private:
        std::string srcStdStr;
        std::string fnName = "";

		cl::Buffer *aBuffer=NULL;
		cl::Buffer *cBuffer=NULL;

		cl::Buffer *buff_phase=NULL;
		cl::Buffer *buff_error=NULL;
		cl::Buffer *buff_freq=NULL;

		int curBufferSize=0;

        float d_loopbw;
        int d_order;

        float d_float_error;
        float d_float_noise;

        double d_double_phase;
        double d_double_error;
        double d_double_noise;
        double d_double_freq;

        float (clCostasLoop_impl::*d_phase_detector)(gr_complex sample) const;

		void buildKernel(int numItems);

	    float phase_detector_2(gr_complex sample) const;
	    float phase_detector_4(gr_complex sample) const;
	    float phase_detector_8(gr_complex sample) const;

     public:
      clCostasLoop_impl(int openCLPlatformType, int devSelector,int platformId, int devId, float loop_bw, int order, bool setDebug);
      ~clCostasLoop_impl();

      void setBufferLength(int numItems);
      virtual bool stop();

      int testCPU(int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items);

      int testOpenCL(int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items);

      int processOpenCL(int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items);

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace clenabled
} // namespace gr

#endif /* INCLUDED_CLENABLED_CLCOSTASLOOP_IMPL_H */

