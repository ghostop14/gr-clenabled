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
#include "clCostasLoop_impl.h"
#include <gnuradio/sincos.h>
#include <gnuradio/expj.h>
#include <gnuradio/math.h>

#define CL_TWO_PI 6.28318530717958647692
#define CL_MINUS_TWO_PI -6.28318530717958647692

namespace gr {
  namespace clenabled {

    clCostasLoop::sptr
    clCostasLoop::make(int openCLPlatformType, int devSelector,int platformId, int devId, float loop_bw, int order, int setDebug)
    {
    	if (setDebug == 1) {
  	      return gnuradio::get_initial_sptr
  	        (new clCostasLoop_impl(openCLPlatformType,devSelector,platformId,devId,loop_bw,order,true));
    	}
    	else {
  	      return gnuradio::get_initial_sptr
  	        (new clCostasLoop_impl(openCLPlatformType,devSelector,platformId,devId,loop_bw,order,false));
    	}

    }

    /*
     * The private constructor
     */
    clCostasLoop_impl::clCostasLoop_impl(int openCLPlatformType, int devSelector,int platformId, int devId, float loop_bw, int order, bool setDebug)
      : gr::sync_block("clCostasLoop",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
			  // Last param here says use an out-of-order queue.  This is to facilitate task-parallel vs data-parallel processing
			  GRCLBase(DTYPE_COMPLEX, sizeof(gr_complex),openCLPlatformType,devSelector,platformId,devId,setDebug,true),
			  blocks::control_loop(loop_bw, 1.0, -1.0),
			  d_order(order), d_float_error(0), d_float_noise(1.0), d_double_error(0), d_double_noise(1.0), d_phase_detector(NULL),
			  d_loopbw(loop_bw)
    {
    	d_double_freq = d_freq;  // d_freq is in control_loop as a float

        // Set up the phase detector to use based on the constellation order
        switch(d_order) {
        	case 2:
            	d_phase_detector = &clCostasLoop_impl::phase_detector_2;
            break;

        	case 4:
            	d_phase_detector = &clCostasLoop_impl::phase_detector_4;
            break;
/*
        	case 8:
            	d_phase_detector = &clCostasLoop_impl::phase_detector_8;
            break;
*/
        	default:
//        		throw std::invalid_argument("order must be 2, 4, or 8");
        		throw std::invalid_argument("order must be 2 or 4");
        	break;
        }

        int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

    	setBufferLength(imaxItems);

        // And finally optimize the data we get based on the preferred workgroup size.
        // Note: We can't do this until the kernel is compiled and since it's in the block class
        // it has to be done here.
        // Note: for CPU's adjusting the workgroup size away from 1 seems to decrease performance.
        // For GPU's setting it to the preferred size seems to have the best performance.
    	try {
    		if (contextType != CL_DEVICE_TYPE_CPU) {
    			gr::block::set_output_multiple(preferredWorkGroupSizeMultiple);
    		}
    		else {
    			// Keep the IO somewhat aligned
    			gr::block::set_output_multiple(32);
    		}
    	}
        catch (...) {

        }

    }

    void clCostasLoop_impl::buildKernel(int numItems) {
    	maxConstItems = (int)((float)maxConstMemSize / ((float)dataSize));
    	bool useConst;

    	if (numItems > maxConstItems)
    		useConst = false;
    	else
    		useConst = true;

		if (debugMode) {
			if (useConst)
				std::cout << "OpenCL INFO: Costas Loop building kernel with __constant params..." << std::endl;
			else
				std::cout << "OpenCL INFO: Costas Loop - too many items for constant memory.  Building kernel with __global params..." << std::endl;
		}

		// Now we set up our OpenCL kernel
		srcStdStr = "";
		fnName = "";

		fnName = "costasloop";

		// d_alpha, d_beta are calculated by the control_loop constructor.
		// d_max_freq and d_min_freq are fed in as 1.0 and -1.0 respectively for the costas loop constructor.
		srcStdStr = "#define d_alpha " + std::to_string(d_alpha) + "\n";
		srcStdStr += "#define d_beta " + std::to_string(d_beta) + "\n";

		srcStdStr += "#define d_max_freq 1.0\n";
		srcStdStr += "#define d_min_freq -1.0\n";

		srcStdStr += "#define CL_TWO_PI 6.28318530717958647692\n";
		srcStdStr += "#define CL_MINUS_TWO_PI -6.28318530717958647692\n";

		srcStdStr += "struct ComplexStruct {\n";
		srcStdStr += "float real;\n";
		srcStdStr += "float imag; };\n";
		srcStdStr += "typedef struct ComplexStruct SComplex;\n";

		if (useConst)
			srcStdStr += "__kernel void costasloop(__constant SComplex * iptr, const int noutput_items, __global SComplex * restrict optr,\n";
		else
			srcStdStr += "__kernel void costasloop(__global SComplex * restrict iptr, const int noutput_items, __global SComplex * restrict optr,\n";

    	if (hasDoublePrecisionSupport) {
    		srcStdStr += "							__global double * restrict d_phase, __global double * restrict d_error,__global double * restrict d_freq) {\n";
    		srcStdStr += "	double i_r,i_i,n_r,n_i,o_r,o_i;\n";
    		srcStdStr += "	double x1,x2;\n";
    		srcStdStr += "	int i;\n";
    		srcStdStr += "\n";
    		srcStdStr += "  double l_phase=*d_phase;\n";
    		srcStdStr += "  double l_error=*d_error;\n";
    		srcStdStr += "  double l_freq=*d_freq;\n";

    		srcStdStr += "	for(i = 0; i < noutput_items; i++) {\n";
    		srcStdStr += "	  n_i = sin(-l_phase);\n";
    		srcStdStr += "	  n_r = cos(-l_phase);\n";
    		srcStdStr += "\n";
    		srcStdStr += "	  //optr[i] = iptr[i] * nco_out;\n";
    		srcStdStr += "	  i_r = iptr[i].real;\n";
    		srcStdStr += "	  i_i = iptr[i].imag;\n";

    		if (hasDoubleFMASupport) {
        		srcStdStr += "	  o_r = fma((double)iptr[i].real,n_r,-((double)iptr[i].imag)*n_i);\n";
        		srcStdStr += "	  o_i = fma((double)iptr[i].real,n_i,(double)iptr[i].imag*n_r);\n";
    		}
    		else {
        		srcStdStr += "	  o_r = (i_r * n_r) - (i_i*n_i);\n";
        		srcStdStr += "	  o_i = (i_r * n_i) + (i_i * n_r);\n";
    		}
    		srcStdStr += "	  optr[i].real = o_r;\n";
    		srcStdStr += "	  optr[i].imag = o_i;\n";
    		srcStdStr += "\n";

    		if (d_order == 2) {
    			//d_error = (*this.*d_phase_detector)(optr[i]);\n";
    			// 2nd order in-place\n";
    			srcStdStr += "	  l_error = o_r*o_i;\n";
    		}
    		else {
    	          // 4th order in-place
    	          // d_error = (optr[i].real()>0 ? 1.0 : -1.0) * optr[i].imag() - (optr[i].imag()>0 ? 1.0 : -1.0) * optr[i].real();
    			srcStdStr += "	  l_error = (o_r>0 ? 1.0 : -1.0) * o_i - (o_i>0 ? 1.0 : -1.0) * o_r;\n";

    		}

    		srcStdStr += "\n";
    		srcStdStr += "	  // d_error = gr::branchless_clip(d_error, 1.0);\n";
    		srcStdStr += "	  l_error = 0.5 * (fabs(l_error+1) - fabs(l_error-1));\n";
    		srcStdStr += "\n";
    		srcStdStr += "	  //advance_loop(d_error);\n";

    		if (hasDoubleFMASupport) {
        		srcStdStr += "	  l_freq = fma(d_beta,l_error,l_freq);\n";
        		srcStdStr += "	  l_phase = l_phase + fma(d_alpha,l_error,l_freq);\n";
    		}
    		else {
        		srcStdStr += "	  l_freq = l_freq + d_beta * l_error;\n";
        		srcStdStr += "	  l_phase = l_phase + l_freq + d_alpha * l_error;\n";
    		}

    		srcStdStr += "\n";
    		srcStdStr += "	  //phase_wrap();\n";

    		srcStdStr += "if ((l_phase > CL_TWO_PI) || (l_phase < CL_MINUS_TWO_PI)) {\n";
    		srcStdStr += "	l_phase = l_phase / CL_TWO_PI - (float)((int)(l_phase / CL_TWO_PI));\n";
    		srcStdStr += "	l_phase = l_phase * CL_TWO_PI;\n";
    		srcStdStr += "}\n";

    		srcStdStr += "\n";
    		srcStdStr += "	  //frequency_limit();\n";
    		srcStdStr += "	  if(l_freq > d_max_freq)\n";
    		srcStdStr += "		l_freq = d_max_freq;\n";
    		srcStdStr += "	  else if(l_freq < d_min_freq)\n";
    		srcStdStr += "		l_freq = d_min_freq;\n";
    		srcStdStr += "	}\n";
    		srcStdStr += "\n";
    		srcStdStr += "	*d_phase = l_phase;\n";
    		srcStdStr += "	*d_freq = l_freq;\n";
    		srcStdStr += "	*d_error = l_error;\n";

    		srcStdStr += "}\n";
    	}
    	else {
    		srcStdStr += "							__global float * restrict d_phase, __global float * restrict d_error,__global float * restrict d_freq) {\n";
    		srcStdStr += "	float i_r,i_i,n_r,n_i,o_r,o_i;\n";
    		srcStdStr += "	float x1,x2;\n";
    		srcStdStr += "	int i;\n";
    		srcStdStr += "\n";
    		srcStdStr += "  float l_phase=*d_phase;\n";
    		srcStdStr += "  float l_error=*d_error;\n";
    		srcStdStr += "  float l_freq=*d_freq;\n";

    		srcStdStr += "	for(i = 0; i < noutput_items; i++) {\n";
    		srcStdStr += "	  n_i = sin(-l_phase);\n";
    		srcStdStr += "	  n_r = cos(-l_phase);\n";
    		srcStdStr += "\n";
    		srcStdStr += "	  //optr[i] = iptr[i] * nco_out;\n";
    		srcStdStr += "	  i_r = iptr[i].real;\n";
    		srcStdStr += "	  i_i = iptr[i].imag;\n";

    		if (hasDoubleFMASupport) {
        		srcStdStr += "	  o_r = fma((double)iptr[i].real,n_r,-((double)iptr[i].imag)*n_i);\n";
        		srcStdStr += "	  o_i = fma((double)iptr[i].real,n_i,(double)iptr[i].imag*n_r);\n";
    		}
    		else {
        		srcStdStr += "	  o_r = (i_r * n_r) - (i_i*n_i);\n";
        		srcStdStr += "	  o_i = (i_r * n_i) + (i_i * n_r);\n";
    		}
    		srcStdStr += "	  optr[i].real = o_r;\n";
    		srcStdStr += "	  optr[i].imag = o_i;\n";
    		srcStdStr += "\n";

    		if (d_order == 2) {
    			//d_error = (*this.*d_phase_detector)(optr[i]);\n";
    			// 2nd order in-place\n";
    			srcStdStr += "	  l_error = o_r*o_i;\n";
    		}
    		else {
    	          // 4th order in-place
    	          // d_error = (optr[i].real()>0 ? 1.0 : -1.0) * optr[i].imag() - (optr[i].imag()>0 ? 1.0 : -1.0) * optr[i].real();
    			srcStdStr += "	  l_error = (o_r>0 ? 1.0 : -1.0) * o_i - (o_i>0 ? 1.0 : -1.0) * o_r;\n";

    		}

    		srcStdStr += "\n";
    		srcStdStr += "	  // d_error = gr::branchless_clip(d_error, 1.0);\n";
    		srcStdStr += "	  l_error = 0.5 * (fabs(l_error+1) - fabs(l_error-1));\n";
    		srcStdStr += "\n";
    		srcStdStr += "	  //advance_loop(d_error);\n";

    		if (hasDoubleFMASupport) {
        		srcStdStr += "	  l_freq = fma(d_beta,l_error,l_freq);\n";
        		srcStdStr += "	  l_phase = l_phase + fma(d_alpha,l_error,l_freq);\n";
    		}
    		else {
        		srcStdStr += "	  l_freq = l_freq + d_beta * l_error;\n";
        		srcStdStr += "	  l_phase = l_phase + l_freq + d_alpha * l_error;\n";
    		}

    		srcStdStr += "\n";
    		srcStdStr += "	  //phase_wrap();\n";

    		srcStdStr += "if ((l_phase > CL_TWO_PI) || (l_phase < CL_MINUS_TWO_PI)) {\n";
    		srcStdStr += "	l_phase = l_phase / CL_TWO_PI - (float)((int)(l_phase / CL_TWO_PI));\n";
    		srcStdStr += "	l_phase = l_phase * CL_TWO_PI;\n";
    		srcStdStr += "}\n";

    		srcStdStr += "\n";
    		srcStdStr += "	  //frequency_limit();\n";
    		srcStdStr += "	  if(l_freq > d_max_freq)\n";
    		srcStdStr += "		l_freq = d_max_freq;\n";
    		srcStdStr += "	  else if(l_freq < d_min_freq)\n";
    		srcStdStr += "		l_freq = d_min_freq;\n";
    		srcStdStr += "	}\n";
    		srcStdStr += "\n";
    		srcStdStr += "	*d_phase = l_phase;\n";
    		srcStdStr += "	*d_freq = l_freq;\n";
    		srcStdStr += "	*d_error = l_error;\n";

    		srcStdStr += "}\n";
    	}

    }


    void clCostasLoop_impl::setBufferLength(int numItems) {
    	if (aBuffer)
    		delete aBuffer;

    	if (cBuffer)
    		delete cBuffer;

    	aBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_ONLY,
			numItems * dataSize);

        cBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_WRITE,
			numItems * dataSize);

		if (buff_phase)
			delete buff_phase;

		if (buff_error)
			delete buff_error;

		if (buff_freq)
			delete buff_freq;

		int param_buff_size;
    	if (hasDoublePrecisionSupport) {
    		param_buff_size = sizeof(double);
    	}
    	else {
    		param_buff_size = sizeof(float);
    	}

		buff_phase = new cl::Buffer(
            *context,
            CL_MEM_READ_WRITE,
			param_buff_size);

		buff_error = new cl::Buffer(
            *context,
            CL_MEM_READ_WRITE,
			param_buff_size);

		buff_freq = new cl::Buffer(
            *context,
            CL_MEM_READ_WRITE,
			param_buff_size);

		buildKernel(numItems);
    	GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());

        curBufferSize = numItems;
    }

    /*
     * Our virtual destructor.
     */
    clCostasLoop_impl::~clCostasLoop_impl()
    {
    	if (curBufferSize > 0)
    		stop();
    }

    bool clCostasLoop_impl::stop() {
    	curBufferSize = 0;

    	if (aBuffer) {
    		delete aBuffer;
    		aBuffer = NULL;
    	}

    	if (cBuffer) {
    		delete cBuffer;
    		cBuffer = NULL;
    	}

		if (buff_phase) {
			delete buff_phase;
			buff_phase = NULL;
		}

		if (buff_error) {
			delete buff_error;
			buff_error = NULL;
		}

		if (buff_freq) {
			delete buff_freq;
			buff_freq = NULL;
		}

    	return GRCLBase::stop();
    }

    float clCostasLoop_impl::phase_detector_2(gr_complex sample) const
    {
      return (sample.real()*sample.imag());
    }

    float clCostasLoop_impl::phase_detector_4(gr_complex sample) const
    {
      return ((sample.real()>0 ? 1.0 : -1.0) * sample.imag() -
	      (sample.imag()>0 ? 1.0 : -1.0) * sample.real());
    }

    float clCostasLoop_impl::phase_detector_8(gr_complex sample) const
    {
      /* This technique splits the 8PSK constellation into 2 squashed
	 QPSK constellations, one when I is larger than Q and one
	 where Q is larger than I. The error is then calculated
	 proportionally to these squashed constellations by the const
	 K = sqrt(2)-1.

	 The signal magnitude must be > 1 or K will incorrectly bias
	 the error value.

	 Ref: Z. Huang, Z. Yi, M. Zhang, K. Wang, "8PSK demodulation for
	 new generation DVB-S2", IEEE Proc. Int. Conf. Communications,
	 Circuits and Systems, Vol. 2, pp. 1447 - 1450, 2004.
      */

      float K = (sqrt(2.0) - 1);
      if(fabsf(sample.real()) >= fabsf(sample.imag())) {
	return ((sample.real()>0 ? 1.0 : -1.0) * sample.imag() -
		(sample.imag()>0 ? 1.0 : -1.0) * sample.real() * K);
      }
      else {
	return ((sample.real()>0 ? 1.0 : -1.0) * sample.imag() * K -
		(sample.imag()>0 ? 1.0 : -1.0) * sample.real());
      }
    }

    int clCostasLoop_impl::testCPU(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    	{
        const gr_complex *iptr = (gr_complex *) input_items[0];
        gr_complex *optr = (gr_complex *) output_items[0];
        float *foptr = (float *) output_items[1];

        bool write_foptr = output_items.size() >= 2;

        gr_complex nco_out;
        float i_r,i_i,n_r,n_i;

        std::vector<tag_t> tags;
        /*
        get_tags_in_range(tags, 0, nitems_read(0),
                          nitems_read(0)+noutput_items,
                          pmt::intern("phase_est"));
  		*/
        if(write_foptr) {
          for(int i = 0; i < noutput_items; i++) {
            if(tags.size() > 0) {
              if(tags[0].offset-nitems_read(0) == (size_t)i) {
                d_phase = (float)pmt::to_double(tags[0].value);
                tags.erase(tags.begin());
              }
            }
            nco_out = gr_expj(-d_phase);

            optr[i] = iptr[i] * nco_out;

            d_float_error = phase_detector_2(optr[i]);
            d_float_error = gr::branchless_clip(d_float_error, 1.0);

            advance_loop(d_float_error);
            phase_wrap();
            frequency_limit();

            foptr[i] = d_freq;
          }
        }
        else {
          for(int i = 0; i < noutput_items; i++) {
            if(tags.size() > 0) {
              if(tags[0].offset-nitems_read(0) == (size_t)i) {
                d_phase = (float)pmt::to_double(tags[0].value);
                tags.erase(tags.begin());
              }
            }
            // gr_expj does a sine/cosine
            // EXPENSIVE LINE
            nco_out = gr_expj(-d_phase);

            optr[i] = iptr[i] * nco_out;

            // EXPENSIVE LINE
            d_float_error = (*this.*d_phase_detector)(optr[i]);
            d_float_error = gr::branchless_clip(d_float_error, 1.0);

            advance_loop(d_float_error);
            phase_wrap();
            frequency_limit();
          }
        }

        return noutput_items;
    }


    int clCostasLoop_impl::testOpenCL(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return processOpenCL(noutput_items,input_items, output_items);
    }

    int clCostasLoop_impl::processOpenCL(int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    {
    	if (kernel == NULL) {
    		return 0;
    	}

    	if (noutput_items > curBufferSize) {
    		setBufferLength(noutput_items);
    	}

    	int inputSize = noutput_items*dataSize;
    	int sizeFloat = sizeof(float);
    	int sizeDouble = sizeof(double);

    	// Not worried about multiple queueing for task-parallel so don't worry about the mutex here.

    	// Protect context from switching
        // gr::thread::scoped_lock guard(d_mutex);

        // Set kernel args
        queue->enqueueWriteBuffer(*aBuffer,CL_TRUE,0,inputSize,input_items[0]);
        kernel->setArg(0, *aBuffer);
        kernel->setArg(1, noutput_items);
        kernel->setArg(2, *cBuffer);

    	if (hasDoublePrecisionSupport) {
            queue->enqueueWriteBuffer(*buff_phase,CL_TRUE,0,sizeDouble,(void *)&d_double_phase);
            queue->enqueueWriteBuffer(*buff_error,CL_TRUE,0,sizeDouble,(void *)&d_double_error);
            queue->enqueueWriteBuffer(*buff_freq,CL_TRUE,0,sizeDouble,(void *)&d_double_freq);

            kernel->setArg(3, *buff_phase);
            kernel->setArg(4, *buff_error);
            kernel->setArg(5, *buff_freq);
    	}
    	else {
            queue->enqueueWriteBuffer(*buff_phase,CL_TRUE,0,sizeFloat,(void *)&d_phase);
            queue->enqueueWriteBuffer(*buff_error,CL_TRUE,0,sizeFloat,(void *)&d_float_error);
            queue->enqueueWriteBuffer(*buff_freq,CL_TRUE,0,sizeFloat,(void *)&d_freq);

            kernel->setArg(3, *buff_phase);
            kernel->setArg(4, *buff_error);
            kernel->setArg(5, *buff_freq);

    	}

        cl::NDRange localWGSize = cl::NDRange(1);

        // Do the work
        queue->enqueueNDRangeKernel(
            *kernel,
            cl::NullRange,
			cl::NDRange(1),
			localWGSize);

    	queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,inputSize,(void *)output_items[0]);
    	if (hasDoublePrecisionSupport) {
        	queue->enqueueReadBuffer(*buff_phase,CL_TRUE,0,sizeDouble,(void *)&d_double_phase);
        	queue->enqueueReadBuffer(*buff_error,CL_TRUE,0,sizeDouble,(void *)(void *)&d_double_error);
        	queue->enqueueReadBuffer(*buff_freq,CL_TRUE,0,sizeDouble,(void *)(void *)&d_double_freq);
    	}
    	else {
        	queue->enqueueReadBuffer(*buff_phase,CL_TRUE,0,sizeFloat,(void *)&d_phase);
        	queue->enqueueReadBuffer(*buff_error,CL_TRUE,0,sizeFloat,(void *)(void *)&d_float_error);
        	queue->enqueueReadBuffer(*buff_freq,CL_TRUE,0,sizeFloat,(void *)(void *)&d_freq);
    	}

      return noutput_items;
    }


    int
    clCostasLoop_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
    	if (debugMode && CLPRINT_NITEMS)
    		std::cout << "clCostasLoop noutput_items: " << noutput_items << std::endl;

        int retVal = processOpenCL(noutput_items,input_items,output_items);
        // int retVal = testCPU(noutput_items,input_items,output_items);

        // Tell runtime system how many output items we produced.
        return retVal;
    }

  } /* namespace clenabled */
} /* namespace gr */

