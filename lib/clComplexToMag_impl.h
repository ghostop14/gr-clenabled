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

#ifndef INCLUDED_CLENABLED_CLCOMPLEXTOMAG_IMPL_H
#define INCLUDED_CLENABLED_CLCOMPLEXTOMAG_IMPL_H

#include <clenabled/clComplexToMag.h>
#include "GRCLBase.h"

namespace gr {
  namespace clenabled {

    class clComplexToMag_impl : public clComplexToMag, public GRCLBase
    {
     private:
      // Nothing to declare in this block.
		cl::Buffer *aBuffer=NULL;
		cl::Buffer *cBuffer=NULL;
		int curBufferSize=0;

		gr_vector_int d_ninput_items;  // backward compatibility item moving from block to sync_block.

		void buildKernel(int numItems);

     public:
      clComplexToMag_impl(int openCLPlatformType,int devSelector,int platformId, int devId,bool setDebug=false);
      virtual ~clComplexToMag_impl();
      virtual bool stop();

      int testCPU(int noutput_items,
              gr_vector_int &ninput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items);

      int processOpenCL(int noutput_items,
              gr_vector_int &ninput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items);

      void setBufferLength(int numItems);

      int testOpenCL(int noutput_items,
              gr_vector_int &ninput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items);

      int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace clenabled
} // namespace gr

#endif /* INCLUDED_CLENABLED_CLCOMPLEXTOMAG_IMPL_H */

