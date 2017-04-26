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


#ifndef INCLUDED_CLENABLED_CLCOSTASLOOP_H
#define INCLUDED_CLENABLED_CLCOSTASLOOP_H

#include <clenabled/api.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/blocks/control_loop.h>

namespace gr {
  namespace clenabled {

    /*!
     * \brief <+description of block+>
     * \ingroup clenabled
     *
     */
    class CLENABLED_API clCostasLoop : virtual public gr::sync_block,
	  	  	  	  	  	  	  	  	  virtual public gr::blocks::control_loop

    {
     public:
      typedef boost::shared_ptr<clCostasLoop> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of clenabled::clCostasLoop.
       *
       * To avoid accidental use of raw pointers, clenabled::clCostasLoop's
       * constructor is in a private implementation
       * class. clenabled::clCostasLoop::make is the public interface for
       * creating new instances.
       */
      static sptr make(int openCLPlatformType, int devSelector,int platformId, int devId, float loop_bw, int order, int setDebug=0);

    };

  } // namespace clenabled
} // namespace gr

#endif /* INCLUDED_CLENABLED_CLCOSTASLOOP_H */

