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

#ifndef INCLUDED_CLENABLED_CLXCORRELATE_H
#define INCLUDED_CLENABLED_CLXCORRELATE_H

#include <clenabled/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace clenabled {

    /*!
     * \brief This block will perform cross-correlation of 1 or more signals against a reference signal.
     * \ingroup clenabled
     *
     */
    class CLENABLED_API clXCorrelate : virtual public gr::sync_block
    {
     public:
      typedef std::shared_ptr<clXCorrelate> sptr;

      /*!
       * \brief Make a cross-correlate block.
       * \param openCLPlatformType Platform type code.
       * \param devSelector device selector
       * \param platformId Platform ID
       * \param devId Device Id
       * \param setDebug Output debug info
       * \param num_inputs Number of signals sent to the block.  Signal[0] will serve as the reference signal.
       * \param signal_length Number of samples to treat as a block to search for the best correlation.
       * \param data_type Float (1) or Complex (2)
       * \param data_size Provide the sizeof(data type)
       * \param max_search_index Limit the number of offsets performed to find the best correlation.
       * \param decim_frames Only process every decim_frames frame.
       * \param async Flag indicating if a second thread should be used for processing.
       */
      static sptr make(int openCLPlatformType,int devSelector,int platformId, int devId, bool setDebug,
    		  	  	   int num_inputs, int signal_length, int data_type, int data_size, int max_search_index, int decim_frames, bool async=false);
    };

  } // namespace clenabled
} // namespace gr

#endif /* INCLUDED_CLENABLED_CLXCORRELATE_H */

