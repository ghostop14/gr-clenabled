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

#ifndef INCLUDED_CLENABLED_CLXCORRELATE_FFT_VCF_H
#define INCLUDED_CLENABLED_CLXCORRELATE_FFT_VCF_H

#include <clenabled/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace clenabled {

    /*!
     * \brief <+description of block+>
     * \ingroup clenabled
     *
     */
    class CLENABLED_API clxcorrelate_fft_vcf : virtual public gr::sync_block
    {
     public:
      typedef std::shared_ptr<clxcorrelate_fft_vcf> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of clenabled::clxcorrelate_fft_vcf.
       *
       * To avoid accidental use of raw pointers, clenabled::clxcorrelate_fft_vcf's
       * constructor is in a private implementation
       * class. clenabled::clxcorrelate_fft_vcf::make is the public interface for
       * creating new instances.
       */
      // Default input type is FFT (1), time-series is (2).  Time-series will trigger an FFT first internally.
      static sptr make(int fftSize, int num_inputs, int openCLPlatformType,int devSelector,int platformId, int devId, int input_type=1);
    };

  } // namespace clenabled
} // namespace gr

#endif /* INCLUDED_CLENABLED_CLXCORRELATE_FFT_VCF_H */

