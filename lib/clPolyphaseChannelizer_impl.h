/* -*- c++ -*- */
/*
 * Copyright 2020 Aaron Giles and Dan Banks.
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

#ifndef INCLUDED_CLENABLED_CLPOLYPHASECHANNELIZER_IMPL_H
#define INCLUDED_CLENABLED_CLPOLYPHASECHANNELIZER_IMPL_H

#include <clenabled/clPolyphaseChannelizer.h>
#include "GRCLBase.h"
#include <clFFT.h>

namespace gr {
  namespace clenabled {

    class clPolyphaseChannelizer_impl : public clPolyphaseChannelizer, public GRCLBase
    {
     private:
        int d_buf_items;
        int d_num_channels;
        int d_ninputs_per_iter; // number of input samples to produce one output on each channel

        std::vector<float> d_taps;
        std::vector<int> d_ch_map;
        std::string d_programCode;

        cl::Program *d_program;
        cl::Kernel *d_kernel;
        cl::Kernel *d_kernel_chmap;

        cl::Buffer *d_in_clmem;
        cl::Buffer *d_filt_clmem;
        cl::Buffer *d_taps_clmem;
        cl::Buffer *d_fft_clmem;
        cl::Buffer *d_chmap_clmem;
        cl::Buffer *d_mapout_clmem;

        clfftPlanHandle d_plan_handle;

        void init_opencl();
        void init_clfft();
        void buildProgram();

     public:
      clPolyphaseChannelizer_impl(int openCLPlatformType, int devSelector, int platformId, int devId,
                                  const std::vector<float> &taps, int buf_items, int num_channels, int ninputs_per_iter, const std::vector<int> &ch_map, bool setDebug=false);
      ~clPolyphaseChannelizer_impl();

      virtual bool stop();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace clenabled
} // namespace gr

#endif /* INCLUDED_CLENABLED_CLPOLYPHASECHANNELIZER_IMPL_H */

