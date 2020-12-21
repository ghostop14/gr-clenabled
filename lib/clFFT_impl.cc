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
#include "clFFT_impl.h"
#include "window.h"
#include "fft.h"
#include <volk/volk.h>

namespace gr {
namespace clenabled {

clFFT::sptr
clFFT::make(int fftSize, int clFFTDir,const std::vector<float> &window, int idataType, int openCLPlatformType,int devSelector,
		int platformId, int devId,int setDebug, int num_streams, bool shift)
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

	if (setDebug == 1) {
		return gnuradio::get_initial_sptr
				(new clFFT_impl(fftSize, clFFTDir,window,idataType,dsize,openCLPlatformType,devSelector,platformId,devId,true, num_streams, shift));
	}
	else {
		return gnuradio::get_initial_sptr
				(new clFFT_impl(fftSize, clFFTDir,window,idataType,dsize,openCLPlatformType,devSelector,platformId,devId,false, num_streams, shift));
	}
}

/*
 * The private constructor
 */
clFFT_impl::clFFT_impl(int fftSize, int clFFTDir,const std::vector<float> &window,int idataType, int dSize, int openCLPlatformType,int devSelector,int platformId,
		int devId,bool setDebug, int num_streams, bool shift)
: gr::sync_block("clFFT",
		gr::io_signature::make(1, num_streams, fftSize*dSize),
		// Output will always be complex
		gr::io_signature::make(1, num_streams, fftSize*sizeof(gr_complex))),
		GRCLBase(idataType, dSize,openCLPlatformType,devSelector,platformId,devId,setDebug),
		d_fft_size(fftSize), d_forward(true), d_shift(shift),d_window(window), d_num_streams(num_streams)
{
    if (!(window.empty() || window.size() == d_fft_size)) {
        throw std::runtime_error("OpenCL FFT: window not the same length as fft_size\n");
    }

    dataSize = dSize;

	vlen_2 = d_fft_size/2;
	data_size_2 = vlen_2 * dSize;

	// Move type to enum var
	if (clFFTDir == CLFFT_FORWARD) {
		fftDir = CLFFT_FORWARD;
	}
	else {
		fftDir = CLFFT_BACKWARD;
	}

	/* Create a default plan for a complex FFT. */
	size_t clLengths[1];
	clLengths[0]=(size_t)d_fft_size;

	int err;

	/* Setup clFFT. */
	clfftSetupData fftSetup;
	err = clfftInitSetupData(&fftSetup);
	err = clfftSetup(&fftSetup);

	err = clfftCreateDefaultPlan(&planHandle, (*context)(), dim, clLengths);

	/* Set plan parameters. */
	// There's no trig in the transforms and there are precomputed "twiddles" but they're done with
	// double precision.  SINGLE/DOUBLE here really just refers to the data type for the math.
	err = clfftSetPlanPrecision(planHandle, CLFFT_SINGLE);

	if (dataType==DTYPE_COMPLEX) {
		err = clfftSetLayout(planHandle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);
	}
	else {
		// This matches the GR FFT block.  Output is always complex.
		err = clfftSetLayout(planHandle, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);
	}

	if (err != CL_SUCCESS) {
		std::cout << "Error setting FFT in/out data types.  setLayout return value: " << err << std::endl;
	}

	clfftSetPlanScale(planHandle, CLFFT_FORWARD, 1.0f); // 1.0 is the default but don't assume.
	clfftSetPlanScale(planHandle, CLFFT_BACKWARD, 1.0f); // By default the backward scale is set to 1/N so you have to set it here.

	//err = clfftSetResultLocation(planHandle, CLFFT_INPLACE);  // In-place puts data back in source queue.  Not what we want.
	err = clfftSetResultLocation(planHandle, CLFFT_OUTOFPLACE);

	/* Bake the plan. */
	err = clfftBakePlan(planHandle, 1, &(*queue)(), NULL, NULL);

	// Now let's set up for batch processing of blocks
	maxBatchSize = 1;

	setBufferLength(d_fft_size);

	// If we have a Window, we only need to copy it to the buffer once.
	// Copy our window to cBuffer
    if (window.size() == d_fft_size) {
    	// Window will always be float.  So can't use data_size here.
    	queue->enqueueWriteBuffer(*ocl_windowBuffer,CL_FALSE,0,d_fft_size*sizeof(float),(void *)&d_window[0]);
    }

    // Used for some of the CPU tests.
	d_fft = new fft_complex(d_fft_size, d_forward, 1);

	// We'll need this if we have to swap the FFT
	if ((d_shift && (dataType == DTYPE_COMPLEX)) || (dataType == DTYPE_FLOAT) ) {
		tmp_buffer = new gr_complex[d_fft_size];
	}

	buildMultiplyFloatKernel();
}

void clFFT_impl::setBufferLength(int numItems) {
	if (aBuffer)
		delete aBuffer;

	if (cBuffer)
		delete cBuffer;

	if (ocl_windowBuffer)
		delete ocl_windowBuffer;

	curBufferSize=numItems;
	fft_times_data_size = curBufferSize*dataSize; // curBufferSize will be fftSize
	fft_times_data_times_batch = fft_times_data_size * maxBatchSize;
	fft_times_data_size_out = d_fft_size * sizeof(gr_complex);

	if (windowBuffer) {
		if (dataType==DTYPE_COMPLEX) {
			delete[] (gr_complex *)windowBuffer;
		}
		else {
			delete[] (float *)windowBuffer;
		}

	}

	if (dataType==DTYPE_COMPLEX) {
		windowBuffer = (void *)new gr_complex[curBufferSize];
	}
	else {
		windowBuffer = (void* )new float[curBufferSize];
	}


	aBuffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,  // this has to be read/write of the clFFT enqueue transform call crashes.
			fft_times_data_times_batch);

	ocl_windowBuffer = new cl::Buffer(
			*context,
			CL_MEM_READ_ONLY,  // this has to be read/write of the clFFT enqueue transform call crashes.
			fft_times_data_times_batch);

	cBuffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,
			fft_times_data_size_out*maxBatchSize);
}

void clFFT_impl::buildMultiplyFloatKernel() {
	// This kernel is a little different from c = a*b in an effort
	// to re-use memory.  This will do: a = a * c

	// Now we set up our OpenCL kernel
	std::string srcStdStr="";
	std::string fnName = "MultiplyFloat";

	if (dataType==DTYPE_COMPLEX) {
		srcStdStr += "struct ComplexStruct {\n";
		srcStdStr += "float real;\n";
		srcStdStr += "float imag; };\n";
		srcStdStr += "typedef struct ComplexStruct SComplex;\n";

		srcStdStr += "__kernel void MultiplyFloat(__global SComplex * restrict a, __global float * restrict c) {\n";
		srcStdStr += "    size_t index =  get_global_id(0);\n";

		srcStdStr += "    float c_val = c[index];\n";
		srcStdStr += "    a[index].real *= c_val;\n";
		srcStdStr += "    a[index].imag *= c_val;\n";
	}
	else {
		srcStdStr += "__kernel void MultiplyFloat(__global float * restrict a, __global float * restrict c) {\n";
		srcStdStr += "    size_t index =  get_global_id(0);\n";
		srcStdStr += "	  a[index] *= c[index];\n";
	}

	srcStdStr += "}\n";

	try {
		// Create and program from source
		if (multiply_program) {
			delete multiply_program;
			multiply_program = NULL;
		}
		if (multiply_sources) {
			delete multiply_sources;
			multiply_sources = NULL;
		}
		multiply_sources=new cl::Program::Sources(1, std::make_pair(srcStdStr.c_str(), 0));
		multiply_program = new cl::Program(*context, *multiply_sources);

		// Build program
		multiply_program->build(devices);

		multiply_kernel=new cl::Kernel(*multiply_program, (const char *)fnName.c_str());
	}
	catch(cl::Error& e) {
		std::cout << "OpenCL Error compiling kernel for " << fnName << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what() << std::endl;
		std::cout << srcStdStr << std::endl;
		exit(0);
	}

	try {
		preferredWorkGroupSizeMultiple = multiply_kernel->getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(devices[0]);
	}
	catch(cl::Error& e) {
		std::cout << "Error getting kernel preferred work group size multiple" << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what()<< std::endl;
	}

	try {
		maxWorkGroupSize = multiply_kernel->getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0]);
	}
	catch(cl::Error& e) {
		std::cout << "Error getting kernel preferred work group size multiple" << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what()<< std::endl;
	}
}

/*
 * Our virtual destructor.
 */
clFFT_impl::~clFFT_impl()
{
	if (curBufferSize > 0)
		stop();
}

bool clFFT_impl::stop() {
	curBufferSize = 0;
	/* Release the plan. */
	int err;


	if (tmp_buffer) {
		delete[] tmp_buffer;
		tmp_buffer = NULL;
	}

	if (windowBuffer) {
		if (dataType==DTYPE_COMPLEX) {
			delete[] (gr_complex *)windowBuffer;
		}
		else {
			delete[] (float *)windowBuffer;
		}

		windowBuffer = NULL;
	}

	err = clfftDestroyPlan( &planHandle );
	/* Release clFFT library. */
	clfftTeardown( );

	if (aBuffer) {
		delete aBuffer;
		aBuffer = NULL;
	}

	if (ocl_windowBuffer) {
		delete ocl_windowBuffer;
		ocl_windowBuffer = NULL;
	}

	if (cBuffer) {
		delete cBuffer;
		cBuffer = NULL;
	}

	if (d_fft) {
		delete d_fft;
		d_fft = NULL;
	}

	// Additional Kernels
	// Multiply
	try {
		if (multiply_kernel != NULL) {
			delete multiply_kernel;
			multiply_kernel=NULL;
		}
	}
	catch (...) {
		multiply_kernel=NULL;
		std::cout<<"multiply kernel delete error." << std::endl;
	}

	try {
		if (multiply_program != NULL) {
			delete multiply_program;
			multiply_program = NULL;
		}
	}
	catch(...) {
		multiply_program = NULL;
		std::cout<<"ccmag program delete error." << std::endl;
	}

	return GRCLBase::stop();
}

void
clFFT_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
{
	/* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
}

void clFFT_impl::FFTValidationTest(bool fwdXForm) {
	gr_complex input_items[d_fft_size];
	gr_complex output_items_gnuradio[d_fft_size];
	gr_complex output_items_opencl[d_fft_size];

	int i;

	// Create some data
	float frequency_signal = 10;
	float frequency_sampling = d_fft_size*frequency_signal;
	float curPhase = 0.0;
	float signal_ang_rate = 2*M_PI*frequency_signal / frequency_sampling;

	for (i=0;i<d_fft_size;i++) {
		input_items[i] = gr_complex(sin(curPhase+(signal_ang_rate*i)),cos(curPhase+(signal_ang_rate*i)));
		output_items_gnuradio[i] = gr_complex(0.0,0.0);
		output_items_opencl[i] = gr_complex(0.0,0.0);
	}


	const gr_complex *in = (const gr_complex *) &input_items[0];
	gr_complex *out = (gr_complex *) &output_items_gnuradio[0];

	std::cout << "Running native transform..." << std::endl;
	// Run an FFT with GNURadio code
	memcpy(d_fft->get_inbuf(), in, d_fft_size*sizeof(gr_complex));
	d_fft->execute();
	memcpy (out, d_fft->get_outbuf (), d_fft_size*sizeof(gr_complex));

	// Run an FFT with OpenCL code
	out = (gr_complex *) &output_items_opencl[0];

	// Note: inputSize is set in setBufferLength().  inputSize = fft_size * sizeof(data type)
	queue->enqueueWriteBuffer(*aBuffer,CL_FALSE,0,d_fft_size*sizeof(gr_complex),(void *)&in[0]);

	std::cout << "Running OpenCL transform..." << std::endl;

	int err;
	err = clfftEnqueueTransform(planHandle, fftDir, 1, &((*queue)()), 0, NULL, NULL, &((*aBuffer)()), &((*cBuffer)()), NULL);
	if( err != CL_SUCCESS ){
		std::cout << "OpenCL FFT Error enqueuing transform.  Error code " << err << std::endl;
		exit(1);
	}
	if( err != CL_SUCCESS ){
		std::cout << "OpenCL FFT Error finishing transform.  Error code " << err << std::endl;
		exit(1);
	}
	// Fetch results of calculations.
	queue->enqueueReadBuffer(*cBuffer,CL_FALSE,0,d_fft_size*sizeof(gr_complex),(void *)&out[0]);
	queue->finish();

	// Compare results

	float o_r_gnuradio,o_i_gnuradio,o_r_opencl,o_i_opencl;
	float diff_real,diff_imag;

	std::cout << "Comparing transform results..." << std::endl;
	bool resmatched=true;
	int numMismatched = 0;
	float real_ratio, imag_ratio;

	for(i=0;i<d_fft_size;i++) {
		o_r_gnuradio = round(output_items_gnuradio[i].real(),12);
		o_i_gnuradio = round(output_items_gnuradio[i].imag(),12);
		o_r_opencl = round(output_items_opencl[i].real(),12);
		o_r_opencl = round(output_items_opencl[i].imag(),12);

		diff_real = fabs(o_r_gnuradio - o_r_opencl);
		diff_imag = fabs(o_i_gnuradio - o_i_opencl);
		real_ratio = fabs(o_r_gnuradio / o_r_opencl);
		imag_ratio = fabs(o_i_gnuradio / o_i_opencl);

		if (diff_real > 0.0 || diff_imag > 0.0) {
			resmatched = false;
			std::cout << std::fixed << std::setw(12)
			    				<< std::setprecision(12) << "Output varied for sample " << i << ": real diff=" << diff_real <<
								/* " ratio: " << real_ratio << */ " imag diff=" << diff_imag << /* " imag ratio: " << imag_ratio << */ std::endl;
			numMismatched++;
		}
	}

	std::cout << "Last Sample: " << std::endl;
	std::cout << "GNURadio: real = " << std::fixed << std::setw(12) << std::setprecision(12) <<
			output_items_gnuradio[d_fft_size-1].real() << " imag: " << output_items_gnuradio[d_fft_size-1].imag() << std::endl;
	std::cout << "OpenCL: real = "<< std::fixed << std::setw(12) << std::setprecision(12) <<
			output_items_opencl[d_fft_size-1].real() << " imag: " << output_items_opencl[d_fft_size-1].imag() << std::endl;

	if (resmatched) {
		std::cout << "Results matched." << std::endl;
	}
	else {
		std::cout << numMismatched << " out of " << d_fft_size << " didn't match." << std::endl;

	}
}

float clFFT_impl::round(float input, int precision) {
	float num = input*pow(10.0,(float)precision);
	long long part = (long long)num;

	return ((float)part)/pow(10.0,(float)precision);
}

int clFFT_impl::testCPU(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	const gr_complex *in = (const gr_complex *) input_items[0];
	gr_complex *out = (gr_complex *) output_items[0];

	int count = 0;

	while(count < noutput_items) {
		// copy input into optimally aligned buffer
		if(d_window.size()) {
			gr_complex *dst = d_fft->get_inbuf();
			if(!d_forward && d_shift) {
				unsigned int offset = (!d_forward && d_shift)?(d_fft_size/2):0;
				int fft_m_offset = d_fft_size - offset;
				volk_32fc_32f_multiply_32fc(&dst[fft_m_offset], in, &d_window[0], offset);
				volk_32fc_32f_multiply_32fc(&dst[0], &in[offset], &d_window[offset], d_fft_size-offset);
			}
			else {
				volk_32fc_32f_multiply_32fc(&dst[0], in, &d_window[0], d_fft_size);
			}
		}
		else {
			if(!d_forward && d_shift) {  // apply an ifft shift on the data
				gr_complex *dst = d_fft->get_inbuf();
				unsigned int len = (unsigned int)(floor(d_fft_size/2.0)); // half length of complex array
				memcpy(&dst[0], &in[len], sizeof(gr_complex)*(d_fft_size - len));
				memcpy(&dst[d_fft_size - len], &in[count], sizeof(gr_complex)*len);
			}
			else {
				memcpy(d_fft->get_inbuf(), &in[count], fft_times_data_size);
			}
		}  // if-else d_window.size();

		// compute the fft
		d_fft->execute();

		// copy result to our output
		if(d_forward && d_shift) {  // apply a fft shift on the data
			unsigned int len = (unsigned int)(ceil(d_fft_size/2.0));
			memcpy(&out[0], &d_fft->get_outbuf()[len], sizeof(gr_complex)*(d_fft_size - len));
			memcpy(&out[d_fft_size - len], &d_fft->get_outbuf()[0], sizeof(gr_complex)*len);
		}
		else {
			memcpy (out, d_fft->get_outbuf (), fft_times_data_size);
		}

		in  += d_fft_size;
		out += d_fft_size;
		count += d_fft_size;
	}

	return noutput_items;
}

int clFFT_impl::testOpenCL(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items) {
	return processOpenCL(noutput_items,input_items, output_items);
}

int clFFT_impl::processOpenCL(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	int i;
	int err;

	// Protect context from switching
	gr::thread::scoped_lock guard(d_mutex);

	// Loop through streams
	for (int curStream=0;curStream<d_num_streams;curStream++) {
		const gr_complex *in_complex = (const gr_complex *) input_items[curStream];
		const float *in_float = (const float *) input_items[curStream];
		gr_complex *out_complex = (gr_complex *) output_items[curStream];

		// FFT is stream->vector.  So noutput_items are individual sample counts.  Moving += fft size.
		for (int count=0;count<noutput_items; count+= d_fft_size) {
			if (dataType==DTYPE_COMPLEX) {
				if ((fftDir==CLFFT_FORWARD) || (!d_shift)) {
					queue->enqueueWriteBuffer(*aBuffer,CL_FALSE,0,fft_times_data_size,(void *)&in_complex[count]);
				}
				else {
					// This case is backward shift
					// Shift has to happen here on the buffer load for reverse.
					queue->enqueueWriteBuffer(*aBuffer,CL_FALSE,0,data_size_2,(void *)&in_complex[count+vlen_2]);
					queue->enqueueWriteBuffer(*aBuffer,CL_FALSE,data_size_2,data_size_2,(void *)&in_complex[count]);
				}
			}
			else {
				if ((fftDir==CLFFT_FORWARD) || (!d_shift)) {
					queue->enqueueWriteBuffer(*aBuffer,CL_FALSE,0,fft_times_data_size,(void *)&in_float[count]);
				}
				else {
					// This case is backward shift
					// Shift has to happen here on the buffer load for reverse.
					queue->enqueueWriteBuffer(*aBuffer,CL_FALSE,0,data_size_2,(void *)&in_float[count+vlen_2]);
					queue->enqueueWriteBuffer(*aBuffer,CL_FALSE,data_size_2,data_size_2,(void *)&in_float[count]);
				}
			}

			if(d_window.size()) {
				// Perform the in-place aBuffer multiply

				// Set kernel args
				multiply_kernel->setArg(0, *aBuffer);
				multiply_kernel->setArg(1, *ocl_windowBuffer);

				// Note: data gets stored back in aBuffer to keep the rest of the logic the same whether or not there's a window.
				queue->enqueueNDRangeKernel(
						*multiply_kernel,
						cl::NullRange,
						cl::NDRange(d_fft_size),
						cl::NullRange);
			}

			// Execute the plan using aBuffer as the source, and storing in cBuffer
			err = clfftEnqueueTransform(planHandle, fftDir, 1, &((*queue)()), 0, NULL, NULL, &((*aBuffer)()), &((*cBuffer)()), NULL);

			// Retrieve the data to the correct output location.
			queue->enqueueReadBuffer(*cBuffer,CL_FALSE,0,fft_times_data_size_out,(void *)&out_complex[count]);
		}
	}

	queue->finish();

	// Shift will only apply to complex forward.
	// For reverse shift, it has to be done before the data is loaded.
	if ((fftDir==CLFFT_FORWARD) &&  d_shift && (dataType == DTYPE_COMPLEX)) {
		for (int cur_signal=0;cur_signal<d_num_streams;cur_signal++) {
			gr_complex *out = (gr_complex *) output_items[cur_signal];

			for (int i=0;i<noutput_items;i+=d_fft_size) {
				// shift fft
				memcpy(tmp_buffer, out, fft_times_data_size);
				memcpy(out, &tmp_buffer[vlen_2], data_size_2);
				memcpy(&out[vlen_2], &tmp_buffer[0], data_size_2);

				out += d_fft_size;
			}
		}
	}
	else if ( (fftDir==CLFFT_FORWARD) && (dataType == DTYPE_FLOAT) ){
		// The resulting transform will be hermitian.  So you have to copy it and take the conjugate
		// to recover the second half
		for (int cur_signal=0;cur_signal<d_num_streams;cur_signal++) {
			gr_complex *out = (gr_complex *) output_items[cur_signal];

			for (int i=0;i<noutput_items;i+=d_fft_size) {
				// Hermitian only outputs half the output since the other half
				// Is the complex conjugate.  It's up to the program to fill in the
				// missing half.
				// And it's reversed.
				// So let's conjugate into the tmp buffer, then
				// reverse it into the final output.
				volk_32fc_conjugate_32fc(tmp_buffer,&out[0],vlen_2);

				for (int j=0;j<vlen_2;j++) {
					out[vlen_2+j] = tmp_buffer[vlen_2-j];
				}

				out += d_fft_size;
			}
		}
	}

	// Tell runtime system how many output items we produced.
	return noutput_items; // will always return FFTSize*2 items.
}


int
clFFT_impl::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	// for vectors coming in, noutput_items will be the number of vectors.
	// So for the calculation we need to multiply # of vectors * fft_size to get the # of data points

	int inputSize = noutput_items * d_fft_size;

	if (debugMode && CLPRINT_NITEMS)
		std::cout << "clFFT_impl inputSize: " << inputSize << std::endl;

	int retVal = processOpenCL(inputSize,input_items,output_items);

	// Tell runtime system how many output items we produced.
	return noutput_items;
}

} /* namespace clenabled */
} /* namespace gr */

