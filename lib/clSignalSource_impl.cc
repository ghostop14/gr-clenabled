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

#define CL_TWO_PI 6.28318530717958647692
#define CL_MINUS_TWO_PI -6.28318530717958647692

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
      : gr::sync_block("clSignalSource",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, iDataSize)),
			  GRCLBase(idataType, iDataSize,openCLPlatformType,devSelector,platformId,devId,setDebug),
			  d_phase(0), d_phase_inc(0),d_sampling_freq((double)samp_rate), d_waveform(waveform),
		      d_frequency(freq),d_float_ampl(amplitude),d_float_angle_pos(0.0),d_float_angle_rate_inc(0.0),
			  d_double_ampl((double)amplitude),d_double_angle_pos(0.0),d_double_angle_rate_inc(0.0)
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

    	if (!hasDoublePrecisionSupport) {
    		std::cout << "OpenCL Signal Source Warning: Your selected OpenCL platform doesn't support double precision math.  The resulting output from this block is going to contain potentially impactful 'noise' (plot it on a frequency plot versus native block for comparison)." << std::endl;
    	}

        switch(dataType) {
        case DTYPE_FLOAT:
        	// Float data type
        	fnName = "sig_float";

        	srcStdStr = "";
        	//srcStdStr = "#define CL_TWO_PI 6.28318530717958647692\n";

        	if (hasDoublePrecisionSupport) {
        		srcStdStr += "__kernel void sig_float(const double phase, const double phase_inc,const double ampl, __global float * restrict c) {\n";
        		srcStdStr += "    int index =  get_global_id(0);\n";
        		srcStdStr += "    double dval = phase+(phase_inc*(double)index);\n";
        		//srcStdStr += "		dval = dval / CL_TWO_PI - (float)((int)(dval / CL_TWO_PI));\n";
        		//srcStdStr += "		dval = dval * CL_TWO_PI;\n";

        		switch (d_waveform) {
        		case SIGSOURCE_COS:
                    srcStdStr += "    c[index] = (float)(cos(dval) * ampl);\n";
        		break;
        		case SIGSOURCE_SIN:
                    srcStdStr += "    c[index] = (float)(sin(dval) * ampl);\n";
        		break;
        		}
            	srcStdStr += "}\n";
        	}
        	else {
        		srcStdStr += "__kernel void sig_float(const float phase, const float phase_inc,const float ampl, __global float * restrict c) {\n";
        		srcStdStr += "    int index =  get_global_id(0);\n";
        		srcStdStr += "    float dval = phase+(phase_inc*(float)index);\n";
        		//srcStdStr += "		dval = dval / CL_TWO_PI - (float)((int)(dval / CL_TWO_PI));\n";
        		//srcStdStr += "		dval = dval * CL_TWO_PI;\n";

        		switch (d_waveform) {
        		case SIGSOURCE_COS:
                    srcStdStr += "    c[index] = (float)(cos(dval) * ampl);\n";
        		break;
        		case SIGSOURCE_SIN:
                    srcStdStr += "    c[index] = (float)(sin(dval) * ampl);\n";
        		break;
        		}
            	srcStdStr += "}\n";
        	}
        break;


        case DTYPE_INT:
        	fnName = "sig_int";

        	srcStdStr = "";
        	//srcStdStr = "#define CL_TWO_PI 6.28318530717958647692\n";

    		srcStdStr += "__kernel void sig_int(const float phase, const float phase_inc,const float ampl, __global int * restrict c) {\n";
    		srcStdStr += "    int index =  get_global_id(0);\n";
    		srcStdStr += "    float dval = phase+(phase_inc*(float)index);\n";
    		//srcStdStr += "		dval = dval / CL_TWO_PI - (float)((int)(dval / CL_TWO_PI));\n";
    		//srcStdStr += "		dval = dval * CL_TWO_PI;\n";
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
        	if (hasDoublePrecisionSupport) {
            	srcStdStr = "";
            	//srcStdStr = "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n";
            	srcStdStr += "struct ComplexStruct {\n";
            	srcStdStr += "float real;\n";
            	srcStdStr += "float imag; \n};\n";
            	srcStdStr += "typedef struct ComplexStruct SComplex;\n";

            	fnName = "sig_complex";

            	srcStdStr += "#define CL_TWO_PI 6.28318530717958647692\n";

        		srcStdStr += "__kernel void sig_complex(const double phase, const double phase_inc, const double ampl, __global SComplex * restrict c) {\n";
        		srcStdStr += "		int index =  get_global_id(0);\n";
        		// Step
        		srcStdStr += "		double dval = phase+(phase_inc*(double)index);\n";
        		// Bound rollover
        		//srcStdStr += "		dval = dval / CL_TWO_PI - (float)((int)(dval / CL_TWO_PI));\n";
        		//srcStdStr += "		dval = dval * CL_TWO_PI;\n";

                srcStdStr += "		c[index].real = (float)(cos(dval) * ampl);\n";
                srcStdStr += "		c[index].imag = (float)(sin(dval) * ampl);\n";
            	srcStdStr += "}\n";
        	}
        	else {
            	srcStdStr = "";
            	//srcStdStr = "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n";
            	srcStdStr += "struct ComplexStruct {\n";
            	srcStdStr += "float real;\n";
            	srcStdStr += "float imag; \n};\n";
            	srcStdStr += "typedef struct ComplexStruct SComplex;\n";

            	fnName = "sig_complex";

            	srcStdStr += "#define CL_TWO_PI 6.28318530717958647692\n";

        		srcStdStr += "__kernel void sig_complex(const float phase, const float phase_inc, const float ampl, __global SComplex * restrict c) {\n";
        		srcStdStr += "		int index =  get_global_id(0);\n";
        		// Step
        		srcStdStr += "		float dval = phase+(phase_inc*(float)index);\n";
        		// Bound rollover
        		//srcStdStr += "		dval = dval / CL_TWO_PI - (float)((int)(dval / CL_TWO_PI));\n";
        		//srcStdStr += "		dval = dval * CL_TWO_PI;\n";

                srcStdStr += "		c[index].real = cos(dval) * ampl;\n";
                srcStdStr += "		c[index].imag = sin(dval) * ampl;\n";
            	srcStdStr += "}\n";
        	}
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
	clSignalSource_impl::set_frequency(double frequency)
    {
        // angle_rate is in radians / step
      d_frequency = frequency;

      // Set up our new params
      d_float_angle_rate_inc=(float)(CL_TWO_PI * d_frequency / d_sampling_freq);
      d_double_angle_rate_inc=CL_TWO_PI * d_frequency / d_sampling_freq;

      d_phase_inc = gr::fxpt::float_to_fixed(d_float_angle_rate_inc);
    }

    float clSignalSource_impl::getAnglePos() {
    	if (hasDoublePrecisionSupport) {
        	return (float)d_double_angle_pos;
    	}
    	else {
        	return d_float_angle_pos;
    	}
    }
    float clSignalSource_impl::getAngleRate() {
    	if (hasDoublePrecisionSupport) {
        	return (float)d_double_angle_rate_inc;
    	}
    	else {
        	return d_float_angle_rate_inc;
    	}
    }

    void clSignalSource_impl::step() {
        d_phase += d_phase_inc;

        if (hasDoublePrecisionSupport) {
            d_double_angle_pos += d_double_angle_rate_inc;

            // don't speed this up, this is for work test comparisons.
            while (d_double_angle_pos > CL_TWO_PI)
            	d_double_angle_pos -= CL_TWO_PI;

            while (d_double_angle_pos < -CL_TWO_PI)
            	d_double_angle_pos += CL_TWO_PI;
        }
        else {
            d_float_angle_pos += d_float_angle_rate_inc;

            // don't speed this up, this is for work test comparisons.
            while (d_float_angle_pos > CL_TWO_PI)
            	d_float_angle_pos -= CL_TWO_PI;

            while (d_float_angle_pos < -CL_TWO_PI)
            	d_float_angle_pos += CL_TWO_PI;
        }
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
          output[i] = gr_complex(gr::fxpt::cos(d_phase) * d_float_ampl, gr::fxpt::sin(d_phase) * d_float_ampl);
          //output[i] = gr_complex(cos(d_angle_pos) * d_ampl, sin(d_angle_pos) * d_ampl);
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


    	// Protect context from switching
        gr::thread::scoped_lock guard(d_mutex);

    	// Do the work

		// Set kernel args
        if (hasDoublePrecisionSupport) {
    		kernel->setArg(0, d_double_angle_pos);
    		kernel->setArg(1, d_double_angle_rate_inc);
    		kernel->setArg(2, d_double_ampl);
    		kernel->setArg(3, *cBuffer);
        }
        else {
    		kernel->setArg(0, d_float_angle_pos);
    		kernel->setArg(1, d_float_angle_rate_inc);
    		kernel->setArg(2, d_float_ampl);
    		kernel->setArg(3, *cBuffer);
        }

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
        if (hasDoublePrecisionSupport) {
    		d_double_angle_pos = d_double_angle_pos + (d_double_angle_rate_inc * (float)noutput_items);

    		// keep the number from growing to out-of-bounds since S(n)=S(n+m*(2*M_PI))  [where m is an integer - m cycles ahead]
    		// Bound rollover
    		//srcStdStr += "		dval = dval / CL_TWO_PI - (float)((int)(dval / CL_TWO_PI));\n";
    		//srcStdStr += "		dval = dval * CL_TWO_PI;\n";


    		// bound rollover
    		if ((d_double_angle_pos > CL_TWO_PI) || (d_double_angle_pos < CL_MINUS_TWO_PI)) {
    			d_double_angle_pos = d_double_angle_pos / CL_TWO_PI - (double)((int)(d_double_angle_pos / CL_TWO_PI));
    			d_double_angle_pos = d_double_angle_pos * CL_TWO_PI;
    		}
    		/*
    		if (d_angle_pos > CL_TWO_PI) {
    			// std::cout << "d_angle_pos > CL_TWO_PI" << std::endl;

    			while (d_angle_pos > CL_TWO_PI) {
    				d_angle_pos -= CL_TWO_PI;
    			}
    		}
    		else if (d_angle_pos < CL_MINUS_TWO_PI) {
    			// std::cout << "d_angle_pos < CL_MINUS_TWO_PI" << std::endl;
    			while (d_angle_pos < CL_MINUS_TWO_PI) {
    				d_angle_pos += CL_TWO_PI;
    			}
    		}
        	*/
        }
        else {
    		d_float_angle_pos = d_float_angle_pos + (d_float_angle_rate_inc * (float)noutput_items);

    		// keep the number from growing to out-of-bounds since S(n)=S(n+m*(2*M_PI))  [where m is an integer - m cycles ahead]
    		// Bound rollover
    		//srcStdStr += "		dval = dval / CL_TWO_PI - (float)((int)(dval / CL_TWO_PI));\n";
    		//srcStdStr += "		dval = dval * CL_TWO_PI;\n";


    		// bound rollover
    		if ((d_float_angle_pos > CL_TWO_PI) || (d_float_angle_pos < CL_MINUS_TWO_PI)) {
    			d_float_angle_pos = d_float_angle_pos / CL_TWO_PI - (float)((int)(d_float_angle_pos / CL_TWO_PI));
    			d_float_angle_pos = d_float_angle_pos * CL_TWO_PI;
    		}
    		/*
    		if (d_angle_pos > CL_TWO_PI) {
    			// std::cout << "d_angle_pos > CL_TWO_PI" << std::endl;

    			while (d_angle_pos > CL_TWO_PI) {
    				d_angle_pos -= CL_TWO_PI;
    			}
    		}
    		else if (d_angle_pos < CL_MINUS_TWO_PI) {
    			// std::cout << "d_angle_pos < CL_MINUS_TWO_PI" << std::endl;
    			while (d_angle_pos < CL_MINUS_TWO_PI) {
    				d_angle_pos += CL_TWO_PI;
    			}
    		}
        	*/
        }

		return noutput_items;
    }

    int
    clSignalSource_impl::work (int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
    	if (debugMode && CLPRINT_NITEMS)
    		std::cout << "clSignalSource noutput_items: " << noutput_items << std::endl;

        int retVal = processOpenCL(noutput_items,d_ninput_items,input_items,output_items);
        //int retVal = testCPU(noutput_items,d_ninput_items,input_items,output_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace clenabled */
} /* namespace gr */

