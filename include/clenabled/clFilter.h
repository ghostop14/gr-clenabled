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


#ifndef INCLUDED_CLENABLED_CLFILTER_H
#define INCLUDED_CLENABLED_CLFILTER_H

#include <clenabled/api.h>
// #include <gnuradio/block.h>
#include <gnuradio/sync_decimator.h>

namespace gr {
  namespace clenabled {

	const bool DEFAULT_USE_TIME_DOMAIN_SETTING=false;  // frequency domain is still faster.

    /*!
     * \brief <+description of block+>
     * \ingroup clenabled
     *
     */
    class CLENABLED_API clFilter : virtual public gr::sync_decimator // gr::block
    {
     public:
      typedef boost::shared_ptr<clFilter> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of clenabled::clFilter.
       *
       * To avoid accidental use of raw pointers, clenabled::clFilter's
       * constructor is in a private implementation
       * class. clenabled::clFilter::make is the public interface for
       * creating new instances.
       */
      static sptr make(int openclPlatform, int devSelector,int platformId, int devId, int decimation,
              const std::vector<float> &taps,int nthreads=1,int setDebug=0,bool use_time = DEFAULT_USE_TIME_DOMAIN_SETTING);

  	virtual void set_taps2(const std::vector<float> &taps)=0;
    virtual std::vector<float> taps() const = 0;

  	/*!
  	 * \brief Set number of threads to use.
  	 */
  	virtual void set_nthreads(int n)=0;

  	virtual ~clFilter() { }
    };

  } // namespace clenabled
} // namespace gr

#endif /* INCLUDED_CLENABLED_CLFILTER_H */

