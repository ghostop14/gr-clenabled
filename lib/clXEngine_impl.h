/* -*- c++ -*- */
/*
 * Copyright 2020,2021 ghostop14.
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

#ifndef INCLUDED_CLENABLED_CLXENGINE_IMPL_H
#define INCLUDED_CLENABLED_CLXENGINE_IMPL_H

#include <clenabled/clXEngine.h>
#include <clenabled/GRCLBase.h>
#include <boost/thread/thread.hpp>

#define CLXCORR_TRIANGULAR_ORDER 1
#define CLXCORR_FULL_MATRIX 2

namespace gr {
namespace clenabled {

struct XComplexStruct {
	float real = 0.0f;
	float imag = 0.0f;
};
typedef struct XComplexStruct XComplex;

class clXEngine_impl : public clXEngine, public GRCLBase
{
	// Tracking pointers
	gr_complex *complex_input = NULL;
	char *char_input = NULL;

	gr_complex *output_matrix = NULL;
	gr_complex *thread_complex_input = NULL;
	char *thread_char_input = NULL;
	gr_complex *thread_output_matrix = NULL;
	int current_write_buffer;

	// Actual Memory
#define INPUT_PINNED

#ifdef INPUT_PINNED
	cl::Buffer *pinned_input1=NULL;
	cl::Buffer *pinned_input2=NULL;
#endif
	gr_complex *complex_input1 = NULL;
	gr_complex *complex_input2 = NULL;
	gr_complex *output_matrix1 = NULL;
	gr_complex *output_matrix2 = NULL;
	char *char_input1 = NULL;
	char *char_input2 = NULL;
	int d_npol;
	int d_num_inputs;
	int d_output_format;
	int d_first_channel;
	int d_num_channels;
	int d_num_baselines;
	int d_integration_time;
	int d_pipeline_integration;
	int d_pipeline_integration_counter;
	int integration_tracker;
	int frame_size;
	int input_size;
	int num_chan_x2;
	size_t matrix_flat_length;
	long output_size;

	bool d_synchronized;
	bool d_use_internal_synchronizer;
	uint64_t *tag_list;

	int d_data_type;
	int d_data_size;

	bool d_output_file;
	std::string d_file_base;
	bool d_wrote_json = false;
	std::string filename;
	int d_rollover_size_mb;
	size_t rollover_size_bytes;
	int current_rollover_index=1;
	bool d_rollover_files;
    size_t d_bytesWritten = 0;
    FILE *d_fp = NULL;
    boost::mutex d_fpmutex;

    boost::mutex d_thread_active_lock;

	virtual bool open();
	virtual void close();
	virtual void write_json(long seq_num);

	size_t frame_size_times_integration;
	size_t frame_size_times_integration_unpack;
	size_t frame_size_times_integration_bytes;
	int channels_times_baselines;

	std::vector<std::string> d_antenna_list;
	std::string str_antenna_list;

	long d_sync_timestamp;
	long current_timestamp;
	std::string d_object_name;
	double d_starting_chan_center_freq;
	double d_channel_width;
	bool d_disable_output;

	// For async mode, threading:
	boost::thread *proc_thread=NULL;
	bool threadRunning=false;
	bool stop_thread = false;
	bool thread_is_processing=false;
	bool thread_process_data=false;

	// OpenCL variables
    cl::NDRange localWGSize=cl::NullRange;
    cl::NDRange maxCalcWGSize=cl::NullRange;

	cl::Buffer *char_matrix_buffer=NULL;
	cl::Buffer *input_matrix_buffer=NULL;
	cl::Buffer *cross_correlation_buffer=NULL;

	// char to complex kernel
    cl::Program::Sources *char_to_cc_sources=NULL;
    cl::Program *char_to_cc_program=NULL;
    cl::Kernel *char_to_cc_kernel=NULL;

	// zero accumulator
    cl_float2 f_zero = {0.0f,0.0f};

	virtual void runThread();

	void buildKernel();
	void buildKernel_float4();
	void buildCharToComplexKernel();

	inline void xcorrelate(XComplex *cross_correlation)  {
		// Execute the conversion kernel
		kernel->setArg(0, *input_matrix_buffer);
		kernel->setArg(1, *cross_correlation_buffer);

		queue->enqueueNDRangeKernel(
				*kernel,
				cl::NullRange,
				cl::NDRange(channels_times_baselines),
				localWGSize);
	};

public:
	clXEngine_impl(int openCLPlatformType,int devSelector,int platformId, int devId, bool setDebug, int data_type, int data_size, int polarization, int num_inputs,
  		  int output_format, int first_channel, int num_channels, int integration, std::vector<std::string> antenna_list,
			  bool output_file=false, std::string file_base="", int rollover_size_mb=0, bool internal_synchronizer=false,
			  long sync_timestamp=0, std::string object_name="", double starting_chan_center_freq=0.0, double channel_width=0.0,
			  bool disable_output=false, int pipeline_integration=0);
	~clXEngine_impl();


	virtual bool start();
	virtual bool stop();

    void forecast (int noutput_items, gr_vector_int &ninput_items_required);

	long get_input_buffer_size() { return d_num_inputs * d_num_channels * d_npol * d_integration_time; };
	long get_output_buffer_size() { return matrix_flat_length; };

	void xcorrelate(XComplex *input_matrix, XComplex *cross_correlation) {
		queue->enqueueWriteBuffer(*input_matrix_buffer,CL_FALSE,0,frame_size_times_integration_bytes,input_matrix);
		xcorrelate(cross_correlation);
    };

	void xcorrelate(char *input_matrix, XComplex *cross_correlation) {
		// Have to convert first.
		// For 4-bit, frame_size_times_integration_bytes = frame_size_times_integration
		// For 8-bit, frame_size_times_integration_bytes = frame_size_times_integration * 2
		queue->enqueueWriteBuffer(*char_matrix_buffer,CL_FALSE,0,frame_size_times_integration_bytes,input_matrix);

		// Execute the conversion kernel
		char_to_cc_kernel->setArg(0, *char_matrix_buffer);
		char_to_cc_kernel->setArg(1, *input_matrix_buffer);

		queue->enqueueNDRangeKernel(
				*char_to_cc_kernel,
				cl::NullRange,
				cl::NDRange(frame_size_times_integration_unpack),
				localWGSize);

		xcorrelate(cross_correlation);
    };

	int work_processor(
			int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items, bool liveWork
	);

	int work_test(
			int noutput_items,
			gr_vector_const_void_star &input_items,
			gr_vector_void_star &output_items
	);

    int general_work(int noutput_items,
         gr_vector_int &ninput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
};

} // namespace clenabled
} // namespace gr

#endif /* INCLUDED_CLENABLED_CLXENGINE_IMPL_H */

