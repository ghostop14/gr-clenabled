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
#include <gnuradio/fxpt.h>
#include "clSignalSource_impl.h"

namespace gr {
  namespace clenabled {

    clSignalSource::sptr
    clSignalSource::make(int idataType, int openCLPlatformType, int devSelector,int platformId, int devId, float samp_rate,int waveform, float freq, float amplitude,int setDebug)
    {
        int dsize=sizeof(float);

        switch(idataType) {
        case DTYPE_FLOAT:
      	  dsize=sizeof(float);
        break;
        case DTYPE_INT:
      	  dsize=sizeof(int);
        break;
        case DTYPE_COMPLEX:
      	  dsize=sizeof(gr_complex);
        break;
        }

      if (setDebug == 1)
		  return gnuradio::get_initial_sptr
			(new clSignalSource_impl(idataType, dsize, openCLPlatformType, devSelector, platformId, devId, samp_rate, waveform, freq, amplitude, true));
      else
		  return gnuradio::get_initial_sptr
			(new clSignalSource_impl(idataType, dsize, openCLPlatformType, devSelector, platformId, devId, samp_rate, waveform, freq, amplitude, false));

    }

    /*
     * The private constructor
     */
    clSignalSource_impl::clSignalSource_impl(int idataType, int iDataSize, int openCLPlatformType, int devSelector,int platformId, int devId, float samp_rate,int waveform, float freq, float amplitude,bool setDebug)
      : gr::block("clSignalSource",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, iDataSize)),
			  GRCLBase(idataType, iDataSize,openCLPlatformType,devSelector,platformId,devId,setDebug),
			  d_phase(0), d_phase_inc(0),d_sampling_freq((double)samp_rate), d_waveform(waveform),
		      d_frequency(freq),d_ampl((double)amplitude),d_angle_pos(0.0),d_angle_rate_inc(0.0)
    {
    	set_frequency(freq);

    	int imaxItems=gr::block::max_noutput_items();
    	if (imaxItems==0)
    		imaxItems=8192;

    	setBufferLength(imaxItems);

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

    void clSignalSource_impl::setBufferLength(int numItems) {
    	if (cBuffer)
    		delete cBuffer;

        cBuffer = new cl::Buffer(
            *context,
            CL_MEM_READ_WRITE,
			numItems * dataSize);

        buildKernel(numItems);

        GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str());

        curBufferSize=numItems;
    }

    void clSignalSource_impl::buildKernel(int numItems) {
        switch(dataType) {
        case DTYPE_FLOAT:
        	// Float data type
        	fnName = "sig_float";

    		srcStdStr = "__kernel void sig_float(const double phase, const double phase_inc,const float ampl, __global float * restrict c) {\n";
    		srcStdStr += "    size_t index =  get_global_id(0);\n";
    		srcStdStr += "    float dval =  (float)(phase+(phase_inc*(double)index));\n";

    		switch (d_waveform) {
    		case SIGSOURCE_COS:
                srcStdStr += "    c[index] = (float)(cos(dval) * ampl);\n";
    		break;
    		case SIGSOURCE_SIN:
                srcStdStr += "    c[index] = (float)(sin(dval) * ampl);\n";
    		break;
    		}
        	srcStdStr += "}\n";
        break;


        case DTYPE_INT:
        	fnName = "sig_int";

    		srcStdStr = "__kernel void sig_int(const double phase, const double phase_inc,const double ampl, __global int * restrict c) {\n";
    		srcStdStr += "    size_t index =  get_global_id(0);\n";
    		srcStdStr += "    float dval =  (float)(phase+(phase_inc*(double)index));\n";
    		switch (d_waveform) {
    		case SIGSOURCE_COS:
                srcStdStr += "    c[index] = (int)(cos(dval) * ampl);\n";
    		break;
    		case SIGSOURCE_SIN:
                srcStdStr += "    c[index] = (int)(sin(dval) * ampl);\n";
    		break;
    		}
        	srcStdStr += "}\n";
        break;

        case DTYPE_COMPLEX:
        	srcStdStr = "struct ComplexStruct {\n";
        	srcStdStr += "float real;\n";
        	srcStdStr += "float imag; };\n";
        	srcStdStr += "typedef struct ComplexStruct SComplex;\n";

        	fnName = "sig_complex";

    		srcStdStr += "__kernel void sig_complex(const double phase, const double phase_inc, const double ampl, __global SComplex * restrict c) {\n";
    		srcStdStr += "    size_t index =  get_global_id(0);\n";
    		srcStdStr += "    float dval =  (float)(phase+(phase_inc*(double)index));\n";
            srcStdStr += "    c[index].real = (float)(cos(dval) * ampl);\n";
            srcStdStr += "    c[index].imag = (float)(sin(dval) * ampl);\n";
        	srcStdStr += "}\n";
        break;
        }
    }

    bool clSignalSource_impl::stop() {
    	return true;
    }
    /*
     * Our virtual destructor.
     */
    clSignalSource_impl::~clSignalSource_impl()
    {
    	stop();
    }

    void
    clSignalSource_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    void
	clSignalSource_impl::set_frequency(double frequency)
    {
        // angle_rate is in radians / step
      d_frequency = frequency;
      d_phase_inc = gr::fxpt::float_to_fixed(d_angle_rate_inc);

      d_angle_rate_inc=2.0 * M_PI * d_frequency / d_sampling_freq;
    }

    void clSignalSource_impl::step() {
        d_phase += d_phase_inc;
        d_angle_pos += d_angle_rate_inc;
        while (d_angle_pos > (2*M_PI))
        	d_angle_pos -= 2.0 * M_PI;
    }

    int clSignalSource_impl::testOpenCL(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items) {
    	return processOpenCL(noutput_items,ninput_items,input_items, output_items);
    }

    int clSignalSource_impl::testCPU(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    {
    	gr_complex *output=(gr_complex *)output_items[0];

    	// For complex, sin and cos is generated.  For float it's really sin or cos

        for(int i = 0; i < noutput_items; i++) {
          output[i] = gr_complex(gr::fxpt::cos(d_phase) * d_ampl, gr::fxpt::sin(d_phase) * d_ampl);
          step();
        }
    	return noutput_items;
    }

    int clSignalSource_impl::processOpenCL(int noutput_items,
            gr_vector_int &ninput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items)
    {

		if (kernel == NULL) {
			return 0;
		}

    	if (noutput_items > curBufferSize) {
    		setBufferLength(noutput_items);
    	}
		// Do the work

		// Set kernel args
		kernel->setArg(0, d_angle_pos);
		kernel->setArg(1, d_angle_rate_inc);
		kernel->setArg(2, d_ampl);
		kernel->setArg(3, *cBuffer);

		cl::NDRange localWGSize=cl::NullRange;

		if (contextType!=CL_DEVICE_TYPE_CPU) {
			if (noutput_items % preferredWorkGroupSizeMultiple == 0) {
				// for some reason problems start to happen when we're no longer using constant memory
				localWGSize=cl::NDRange(preferredWorkGroupSizeMultiple);
			}
		}

		// Do the work
		queue->enqueueNDRangeKernel(
			*kernel,
			cl::NullRange,
			cl::NDRange(noutput_items),
			localWGSize);

		queue->enqueueReadBuffer(*cBuffer,CL_TRUE,0,noutput_items*dataSize,(void *)output_items[0]);

		// In the CPU code this is done as part of each loop iteration.
		// Since the kernel just uses a multiplier, we'll block increment for the next time
		// here.

//		d_phase = d_phase + (d_phase_inc * noutput_items);
		d_angle_pos = d_angle_pos + (d_angle_rate_inc * noutput_items);

		// keep the number from growing to out-of-bounds since S(n)=S(n+m*(2*M_PI))  [where m is an integer - m cycles ahead]
		while (d_angle_pos > (2*M_PI))
        	d_angle_pos -= 2.0 * M_PI;

		return noutput_items;
    }

    int
    clSignalSource_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        int retVal = processOpenCL(noutput_items,ninput_items,input_items,output_items);
        // int retVal = testCPU(noutput_items,ninput_items,input_items,output_items);
        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace clenabled */
} /* namespace gr */

