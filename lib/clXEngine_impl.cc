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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "clXEngine_impl.h"
#include <volk/volk.h>

// Some carry-forward tricks from file_sink_base.cc
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

// win32 (mingw/msvc) specific
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifdef O_BINARY
#define	OUR_O_BINARY O_BINARY
#else
#define	OUR_O_BINARY 0
#endif

// should be handled via configure
#ifdef O_LARGEFILE
#define	OUR_O_LARGEFILE	O_LARGEFILE
#else
#define	OUR_O_LARGEFILE 0
#endif

namespace gr {
namespace clenabled {

clXEngine::sptr
clXEngine::make(int openCLPlatformType,int devSelector,int platformId, int devId, bool setDebug, int data_type, int polarization, int num_inputs,
		  int output_format, int first_channel, int num_channels, int integration, std::vector<std::string> antenna_list,
		  bool output_file, std::string file_base, int rollover_size_mb, bool internal_synchronizer,
		  long sync_timestamp, std::string object_name, double starting_chan_center_freq, double channel_width, bool disable_output, int pipeline_integration)
{
	int data_size = 1;

	switch (data_type) {
	case DTYPE_COMPLEX:
		data_size = sizeof(gr_complex);
		break;
	case DTYPE_BYTE:
		data_size = sizeof(char)*2;  // Need 2 bytes to make up the complex input (will still be complex, just char, not float)
		break;
	case DTYPE_PACKEDXY:
		data_size = sizeof(char);  // Need 2 bytes to make up the packed xy input
		break;
	}

	return gnuradio::get_initial_sptr
			(new clXEngine_impl(openCLPlatformType, devSelector, platformId, devId, setDebug, data_type, data_size,polarization, num_inputs,
					output_format, first_channel, num_channels, integration, antenna_list, output_file, file_base, rollover_size_mb,
					internal_synchronizer,sync_timestamp, object_name, starting_chan_center_freq, channel_width, disable_output,pipeline_integration));
}

/*
 * The private constructor
 */
clXEngine_impl::clXEngine_impl(int openCLPlatformType,int devSelector,int platformId, int devId, bool setDebug, int data_type, int data_size, int polarization, int num_inputs,
		  int output_format, int first_channel, int num_channels, int integration, std::vector<std::string> antenna_list,
			  bool output_file, std::string file_base, int rollover_size_mb, bool internal_synchronizer,
			  long sync_timestamp, std::string object_name, double starting_chan_center_freq, double channel_width, bool disable_output,int pipeline_integration)
: gr::block("clXEngine",
		gr::io_signature::make(2, num_inputs*(data_type==DTYPE_PACKEDXY?1:polarization), num_channels*(data_type==DTYPE_PACKEDXY?2:data_size)),
		gr::io_signature::make(0, 0, 0)),
		GRCLBase(data_type, data_size,openCLPlatformType,devSelector,platformId,devId,setDebug),
		d_npol(polarization), d_num_inputs(num_inputs), d_output_format(output_format),d_first_channel(first_channel),
		d_num_channels(num_channels), d_integration_time(integration), d_pipeline_integration(pipeline_integration), d_pipeline_integration_counter(0),
		integration_tracker(0),d_data_type(data_type), d_data_size(data_size),
		d_output_file(output_file), d_file_base(file_base), d_rollover_size_mb(rollover_size_mb),
		d_use_internal_synchronizer(internal_synchronizer),
		d_antenna_list(antenna_list),
		d_sync_timestamp(sync_timestamp),
		current_timestamp(0),
		d_object_name(object_name),
		d_starting_chan_center_freq(starting_chan_center_freq),
		d_channel_width(channel_width),
		d_disable_output(disable_output)

{
	if (num_inputs < 2) {
		GR_LOG_ERROR(d_logger, "Please specify at least 2 inputs to correlate.");
		throw std::out_of_range ("Please specify at least 2 inputs to correlate.");
	}

	if (internal_synchronizer) {
		if ((integration % 16) > 0) {
			GR_LOG_ERROR(d_logger, "For the ATA synchronizer, the number of integration frames should be a multiple of 16 to align with blocks coming from the SNAP.");
			throw std::out_of_range ("ATA xengine: The number of integration frames should be a multiple of 16 to align with blocks coming from the SNAP.");
		}
	}

	if (d_disable_output)
		d_output_file = false;

	if (d_output_file) {
		if ((d_rollover_size_mb) > 0) {
			d_rollover_files = true;
			d_bytesWritten = 0;
			rollover_size_bytes = d_rollover_size_mb * 1000000;
		}
		else {
			d_rollover_files = false;
		}

		bool retval = open();

		if (!retval) {
			std::string errmsg = "[X-Engine] can't open file: ";
			errmsg += filename;

			GR_LOG_ERROR(d_logger,errmsg);

			throw std::runtime_error (errmsg);
		}
	}

	// GR's YML doesn't allow a list of strings as a _vector type.  So we're
	// Working the system a bit.  If no antennas are defined, the list could be [''].
	// So since we're supposed to have more than 1 antenna anyway we can use that here
	// to check.
	if (d_antenna_list.size() > 1) {
		int num_ants = d_antenna_list.size();

		str_antenna_list = "[";

		int i=0;

		for (auto ant=d_antenna_list.begin(); ant!=d_antenna_list.end(); ++ant) {
		    str_antenna_list += "\"" + *ant + "\"";

		    if (i < (num_ants-1)) {
			    str_antenna_list += ",";
		    }

		    i++;
		}


		str_antenna_list += "]";
	}
	else {
		str_antenna_list = "[]";
	}

	d_synchronized = false;
	tag_list = new uint64_t[d_num_inputs];

	// Override just in case pol doesn't come through right.

	if (d_data_type == DTYPE_PACKEDXY) {
		d_npol = 2;
	}

	// See "Accelerating Radio Astronomy Cross-Correlation with Graphics Processing Units" by M. A. Clark
	// and xGPU on github for reference documentation and reference implementation.

	d_num_baselines = (d_num_inputs+1)*d_num_inputs / 2;

	// Input size is the size of one sampling vector.  Basically the channel's data
	input_size = d_num_channels * d_data_size;

	num_chan_x2 = d_num_channels * 2;
	frame_size = d_num_channels * d_num_inputs * d_npol;
	frame_size_times_integration = frame_size * d_integration_time;
	if (d_data_type == DTYPE_PACKEDXY) {
		frame_size_times_integration_unpack = frame_size_times_integration / 2; // This is used for PACKEDXY kernel char->complex kernel call
	}
	else {
		frame_size_times_integration_unpack = frame_size_times_integration;
	}

	frame_size_times_integration_bytes = frame_size_times_integration * d_data_size;

	channels_times_baselines = d_num_channels*d_num_baselines;

	current_write_buffer = 1;

	if (d_output_format == CLXCORR_TRIANGULAR_ORDER) {
		// This is only the lower triangular matrix size (including the autocorrelation diagonal
		matrix_flat_length = d_num_channels * d_num_baselines * d_npol * d_npol;
	}
	else {
		// This is the full matrix
		matrix_flat_length = d_num_channels * (d_num_inputs*d_num_inputs*d_npol*d_npol);
	}

	if (d_data_type == DTYPE_PACKEDXY) {
		buildKernel_float4();
	}
	else {
		buildKernel();
	}
	buildCharToComplexKernel();


	int input_matrix_type;

	unsigned long gpu_memory_allocated = 0;

	// Want to precalc in case these throw an error:
	if (d_data_type != DTYPE_COMPLEX) {
		// char input matrix
		gpu_memory_allocated += frame_size_times_integration * d_data_size;
	}
	// input_matrix_buffer
	gpu_memory_allocated += frame_size_times_integration * sizeof(gr_complex);
	// output correlation buffer
	gpu_memory_allocated += matrix_flat_length * sizeof(gr_complex);

	std::stringstream msg_stream;
	msg_stream << "X-Engine Startup Parameters:" << std::endl;
	msg_stream << "Total GPU memory requested: " << gpu_memory_allocated / 1e6 << " MB (" << gpu_memory_allocated << " bytes)" << std::endl;

	if (d_data_type == DTYPE_COMPLEX) {
		msg_stream << "GPU Input Buffer Size (bytes): " << frame_size_times_integration * sizeof(gr_complex) << std::endl;
	}
	else {
		msg_stream << "GPU Input Buffer Size (bytes): " << frame_size_times_integration * d_data_size << std::endl;
	}

	msg_stream << "Integrated output frame size (bytes): " << matrix_flat_length*sizeof(gr_complex) << std::endl <<
				  "Number of Antennas: " << num_inputs << std::endl <<
				  "Starting Channel: " << d_first_channel << std::endl <<
				  "Number of Channels: " << d_num_channels << std::endl <<
				  "Number of Polarizations: " << d_npol << std::endl <<
				  "Number of Baselines (including autocorrelations): " << d_num_baselines << std::endl <<
				  "Number of GPU Integration Frames: " << d_integration_time << std::endl <<
				  "Number of Additional Frame Pipeline Integration: " << d_pipeline_integration << std::endl;


	GR_LOG_INFO(d_logger,msg_stream.str());

	if (d_data_type == DTYPE_COMPLEX) {
		input_matrix_type = CL_MEM_READ_ONLY;
		char_matrix_buffer = NULL; // Don't need it in this case
	}
	else {
		char_matrix_buffer = new cl::Buffer(
				*context,
				CL_MEM_READ_ONLY,
				frame_size_times_integration * d_data_size);

		input_matrix_type = CL_MEM_READ_WRITE;
	}

	// This will always be complex, regardless of the block input type.
	// For byte, we use another kernel to convert into this buffer prior to
	// running the cross correlation.

	input_matrix_buffer = new cl::Buffer(
			*context,
			input_matrix_type,
			frame_size_times_integration * sizeof(gr_complex));

	// Output will always be complex
	output_size = matrix_flat_length * sizeof(gr_complex);

	cross_correlation_buffer = new cl::Buffer(
			*context,
			CL_MEM_READ_WRITE,
			output_size);

	if (d_pipeline_integration > 1) {
		// Now zero out memory for the first cycle
		queue->enqueueFillBuffer(*cross_correlation_buffer,f_zero,0,output_size);
	}

	message_port_register_out(pmt::mp("xcorr"));
	message_port_register_out(pmt::mp("sync"));

	if (d_use_internal_synchronizer) {
		set_tag_propagation_policy(TPP_DONT);
		// The SNAP outputs packets in 16-time step blocks.  So let's take advantage of that here.
		set_output_multiple(16);
	}
}

bool clXEngine_impl::start() {

	/*
	 * Optimal workgroupsize seems to be 256 for 256 channels.
	 * Max workgroup size is 1024.
	*/
	/* Note: This code I don't think is generally optimal.  So it's here for reference
	 * But commented out.  For some reason maxworkgroupsize is coming back as 256 when clinfo reports 1024.
	std::cout << "DEBUG: max workgroup size: " << maxWorkGroupSize << std::endl;
	if (d_num_channels <= maxWorkGroupSize) {
		localWGSize = cl::NDRange(d_num_channels);
	}
	*/

	// 2 buffers will allow us to process one in a worker thread while the other
	// is being loaded.
	size_t mem_alignment = volk_get_alignment();

	if (d_data_type == DTYPE_COMPLEX) {
		// complex_input1 = new gr_complex[frame_size_times_integration];
		// complex_input2 = new gr_complex[frame_size_times_integration];
#ifdef INPUT_PINNED
		pinned_input1 = new cl::Buffer(
				*context,
				CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
				frame_size_times_integration * sizeof(gr_complex));
		pinned_input2 = new cl::Buffer(
				*context,
				CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
				frame_size_times_integration * sizeof(gr_complex));

		complex_input1 = (gr_complex *)queue->enqueueMapBuffer(*pinned_input1, CL_TRUE, CL_MAP_WRITE, 0,
													frame_size_times_integration * sizeof(gr_complex));
		complex_input2 = (gr_complex *)queue->enqueueMapBuffer(*pinned_input2, CL_TRUE, CL_MAP_WRITE, 0,
													frame_size_times_integration * sizeof(gr_complex));
#else
		complex_input1 = (gr_complex *)volk_malloc(frame_size_times_integration*sizeof(gr_complex), mem_alignment);
		complex_input2 = (gr_complex *)volk_malloc(frame_size_times_integration*sizeof(gr_complex), mem_alignment);
#endif
		complex_input = complex_input1;
		thread_complex_input = complex_input;
	}
	else {
		// char_input1 = new char[frame_size_times_integration_bytes];
		// char_input2 = new char[frame_size_times_integration_bytes];
#ifdef INPUT_PINNED
		pinned_input1 = new cl::Buffer(
				*context,
				CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
				frame_size_times_integration);
		pinned_input2 = new cl::Buffer(
				*context,
				CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
				frame_size_times_integration);

		char_input1 = (char *)queue->enqueueMapBuffer(*pinned_input1, CL_TRUE, CL_MAP_WRITE, 0,
													frame_size_times_integration);
		char_input2 = (char *)queue->enqueueMapBuffer(*pinned_input2, CL_TRUE, CL_MAP_WRITE, 0,
													frame_size_times_integration);
#else
		char_input1 = (char *)volk_malloc(frame_size_times_integration_bytes, mem_alignment);
		char_input2 = (char *)volk_malloc(frame_size_times_integration_bytes, mem_alignment);
#endif
		char_input = char_input1;
		thread_char_input = char_input;
	}

	output_matrix1 = (gr_complex *)volk_malloc(matrix_flat_length*sizeof(gr_complex), mem_alignment);
	output_matrix2 = (gr_complex *)volk_malloc(matrix_flat_length*sizeof(gr_complex), mem_alignment);
	// output_matrix1 = new gr_complex[matrix_flat_length];
	// output_matrix2 = new gr_complex[matrix_flat_length];
	output_matrix = output_matrix1;
	thread_output_matrix = output_matrix;

	d_pipeline_integration_counter = 1;

	proc_thread = new boost::thread(boost::bind(&clXEngine_impl::runThread, this));
	return true;
}

void
clXEngine_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
{
	for (int i=0;i< ninput_items_required.size();i++) {
		ninput_items_required[i] = noutput_items;
	}
}


bool clXEngine_impl::open()
{
	gr::thread::scoped_lock guard(d_fpmutex);	// hold mutex for duration of this function
	// we use the open system call to get access to the O_LARGEFILE flag.
	d_wrote_json = false;

	int fd;
	int flags;
	flags = O_WRONLY|O_CREAT|O_TRUNC|OUR_O_LARGEFILE|OUR_O_BINARY;

	filename = d_file_base;

	if (d_rollover_files) {
		std::string newStr = std::to_string(current_rollover_index++);

		while (newStr.length() < 3)
			newStr = "0" + newStr;

		filename += "_" + newStr;
	}

	if((fd = ::open(filename.c_str(), flags, 0664)) < 0){
		GR_LOG_ERROR(d_logger,"Error in initial 0664 open");
		return false;
	}

	if(d_fp) {		// if we've already got a new one open, close it
		fclose(d_fp);
		d_fp = NULL;
	}

	if((d_fp = fdopen (fd, "wb")) == NULL) {
		::close(fd);        // don't leak file descriptor if fdopen fails.
		GR_LOG_ERROR(d_logger,"open-for-write returned NULL");
		return false;
	}

	// Finalize setup
	bool fileOpen = d_fp != 0;

	d_bytesWritten = 0;

	return fileOpen;
}

void clXEngine_impl::write_json(long seq_num) {
	   FILE * pFile;

	   std::string json_file = filename;
	   json_file += ".json";

	   try {
		   pFile = fopen (json_file.c_str(),"w");
	   }
	   catch(...) {
		   GR_LOG_ERROR(d_logger,"Error writing data json descriptor file " + json_file);
		   return;
	   }

	   long integ_frames;
	   if (d_pipeline_integration < 2) {
		   integ_frames = (long)d_integration_time;
	   }
	   else {
		   integ_frames = (long)d_integration_time * (long)d_pipeline_integration;
	   }

	   fprintf(pFile,"{\n\"sync_timestamp\":%ld,\n\"first_seq_num\":%ld,\n\"object_name\":\"%s\",\n\"num_baselines\":%d,\n\"first_channel\":%d,\n\"first_channel_center_freq\":%f,\n\"channels\":%d,\n\"channel_width\":%f,\n\"polarizations\":%d,\n\"antennas\":%d,\n\"antenna_names\":%s,\n\"ntime\":%ld,\n\"samples_per_block\":%ld,\n\"bytes_per_block\":%ld,\n\"data_type\":\"cf32_le\",\n\"data_format\": \"triangular order\"\n}\n",
			   d_sync_timestamp, seq_num, d_object_name.c_str(), d_num_baselines, d_first_channel, d_starting_chan_center_freq, d_num_channels, d_channel_width, d_npol, d_num_inputs, str_antenna_list.c_str(), integ_frames, matrix_flat_length, matrix_flat_length*sizeof(gr_complex));
	   fclose (pFile);

	   d_wrote_json = true;
}

void clXEngine_impl::close() {
	gr::thread::scoped_lock guard(d_fpmutex);	// hold mutex for duration of this function
	if (d_fp) {
		fclose(d_fp);
		d_fp = NULL;
	}
}

bool clXEngine_impl::stop() {
	if (proc_thread) {
		stop_thread = true;

		while (threadRunning)
			usleep(10);

		delete proc_thread;
		proc_thread = NULL;
	}

	close();

#ifdef INPUT_PINNED
	if (pinned_input1) {
		GR_LOG_INFO(d_logger,"Releasing GPU mapped memory objects");
		if (complex_input1) {
			queue->enqueueUnmapMemObject(*pinned_input1,complex_input1);
		}
		else {
			queue->enqueueUnmapMemObject(*pinned_input1,char_input1);
		}

		delete pinned_input1;
		pinned_input1 = NULL;
	}
	if (pinned_input2) {
		if (complex_input1) {
			queue->enqueueUnmapMemObject(*pinned_input2,complex_input2);
		}
		else {
			queue->enqueueUnmapMemObject(*pinned_input2,char_input2);
		}
		delete pinned_input2;
		pinned_input2 = NULL;
	}
#else
	if (complex_input1) {
		// delete[] complex_input1;
		volk_free(complex_input1);
		complex_input1 = NULL;
	}

	if (complex_input2) {
		// delete[] complex_input2;
		volk_free(complex_input2);
		complex_input2 = NULL;
	}

	if (char_input1) {
		// delete[] char_input1;
		volk_free(char_input1);
		char_input1 = NULL;
	}

	if (char_input2) {
		// delete[] char_input2;
		volk_free(char_input2);
		char_input2 = NULL;
	}

#endif

	if (output_matrix1) {
		//delete[] output_matrix1;
		volk_free(output_matrix1);
		output_matrix1 = NULL;
	}

	if (output_matrix2) {
		// delete[] output_matrix2;
		volk_free(output_matrix2);
		output_matrix2 = NULL;
	}

	if (char_matrix_buffer) {
		delete char_matrix_buffer;
		char_matrix_buffer = NULL;
	}

	if (input_matrix_buffer) {
		delete input_matrix_buffer;
		input_matrix_buffer = NULL;
	}

	if (cross_correlation_buffer) {
		delete cross_correlation_buffer;
		cross_correlation_buffer = NULL;
	}

	// Additional Kernels
	// Char to Complex
	try {
		if (char_to_cc_kernel != NULL) {
			delete char_to_cc_kernel;
			char_to_cc_kernel=NULL;
		}
	}
	catch (...) {
		char_to_cc_kernel=NULL;
		std::cout<<"ccmag kernel delete error." << std::endl;
	}

	try {
		if (char_to_cc_program != NULL) {
			delete char_to_cc_program;
			char_to_cc_program = NULL;
		}
	}
	catch(...) {
		char_to_cc_program = NULL;
		std::cout<<"ccmag program delete error." << std::endl;
	}

	if (tag_list) {
		delete[] tag_list;
		tag_list = NULL;
	}

	return true;
}

/*
 * Our virtual destructor.
 */
clXEngine_impl::~clXEngine_impl()
{
	bool ret_val = stop();
}

void clXEngine_impl::buildKernel_float4() {
	std::string srcStdStr="";
	std::string fnName = "XCorrelate";

	// Use #defines to not have to pass them in as params since they won't change
	// This will save time on unchanging param transfers at runtime.
	srcStdStr += "#define d_num_channels " + std::to_string(d_num_channels) + "\n";
	srcStdStr += "#define d_num_baselines " + std::to_string(d_num_baselines) + "\n";
	srcStdStr += "#define d_integration_time " + std::to_string(d_integration_time/2) + "\n";
	srcStdStr += "#define d_num_inputs " + std::to_string(d_num_inputs) + "\n";
	srcStdStr += "#define ant_times_time " + std::to_string(d_num_inputs*d_integration_time/2) + "\n";
	srcStdStr += "#define d_npol " + std::to_string(d_npol) + "\n";
	// frame_size = inputs * channels * pol
	srcStdStr += "#define d_frame_size " + std::to_string(frame_size) + "\n";
	int frame_over_2 = frame_size / 2;
	srcStdStr += "#define d_frame_size_2 " + std::to_string(frame_over_2) + "\n";
	srcStdStr += "\n"; // Just for legibility

	srcStdStr += "__attribute__((always_inline)) inline void cxmac(float2* accum, float2* z0, float2* z1) {\n";
	if (hasDoubleFMASupport || hasSingleFMASupport) {
		srcStdStr += "	(*accum).x += fma((*z0).x,(*z1).x,((*z0).y * (*z1).y));\n";
		srcStdStr += "	(*accum).y += fma((*z0).y, (*z1).x, (-(*z0).x * (*z1).y));\n";
	}
	else {
		srcStdStr += "	(*accum).x += (*z0).x * (*z1).x + (*z0).y * (*z1).y;\n";
		srcStdStr += "	(*accum).y += (*z0).y * (*z1).x - (*z0).x * (*z1).y;\n";
	}
	srcStdStr += "}\n\n";

	// srcStdStr += "__kernel void XCorrelate(__global XComplex * restrict input_matrix, __global XComplex * restrict cross_correlation) {\n";
	srcStdStr += "__kernel void XCorrelate(__global float8 * restrict input_matrix, __global float8 * restrict cross_correlation) {\n";
	srcStdStr += "size_t i =  get_global_id(0);\n";

	srcStdStr += "int f = i/d_num_baselines;\n";
	srcStdStr += "int k = i - f*d_num_baselines;\n";
	if (hasDoubleFMASupport || hasSingleFMASupport) {
		srcStdStr += "int station1 = -0.5 + sqrt(fma(2.0,k,0.25));\n";
	}
	else {
		srcStdStr += "int station1 = -0.5 + sqrt(0.25 + 2*k);\n";
	}
	srcStdStr += "int station2 = k - ((station1+1)*station1)/2;\n";
	srcStdStr += "float2 sumXX = (float2)(0.0f,0.0f);\n";
	srcStdStr += "float2 sumXY = (float2)(0.0f,0.0f);\n";
	srcStdStr += "float2 sumYX = (float2)(0.0f,0.0f);\n";
	srcStdStr += "float2 sumYY = (float2)(0.0f,0.0f);\n";
	srcStdStr += "float2 inputRowX, inputRowY, inputColX, inputColY;\n";

	srcStdStr += "  size_t index1_base = f*ant_times_time + station1 * d_integration_time;\n";
	srcStdStr += "  size_t index2_base = f*ant_times_time + station2 * d_integration_time;\n";

	srcStdStr += "for(int t=0; t<d_integration_time; t++) {\n";
	// srcStdStr += "  int index1 = t*d_frame_size_2 + station1 * d_num_channels + f;\n";
	// srcStdStr += "  int index2 = t*d_frame_size_2 + station2 * d_num_channels + f;\n";
	srcStdStr += "  size_t index1 = index1_base + t;\n";
	srcStdStr += "  size_t index2 = index2_base + t;\n";

	srcStdStr += "	float8 inputRowXY_f8 = input_matrix[index1];\n";
	srcStdStr += "	inputRowX = inputRowXY_f8.s01;\n";
	srcStdStr += "	inputRowY = inputRowXY_f8.s23;\n";

	srcStdStr += "	float8 inputColXY_f8 = input_matrix[index2];\n";
	srcStdStr += "	float4 inputColXY_f4 = inputColXY_f8.s0123;\n";
	srcStdStr += "	inputColX = inputColXY_f8.s01;\n";
	srcStdStr += "	inputColY = inputColXY_f8.s23;\n";

	srcStdStr += "	cxmac(&sumXX, &inputRowX, &inputColX);\n";
	srcStdStr += "	cxmac(&sumXY, &inputRowX, &inputColY);\n";
	srcStdStr += "	cxmac(&sumYX, &inputRowY, &inputColX);\n";
	srcStdStr += "	cxmac(&sumYY, &inputRowY, &inputColY);\n";

	srcStdStr += "	inputRowX = inputRowXY_f8.s45;\n";
	srcStdStr += "	inputRowY = inputRowXY_f8.s67;\n";

	srcStdStr += "	inputColX = inputColXY_f8.s45;\n";
	srcStdStr += "	inputColY = inputColXY_f8.s67;\n";

	srcStdStr += "	cxmac(&sumXX, &inputRowX, &inputColX);\n";
	srcStdStr += "	cxmac(&sumXY, &inputRowX, &inputColY);\n";
	srcStdStr += "	cxmac(&sumYX, &inputRowY, &inputColX);\n";
	srcStdStr += "	cxmac(&sumYY, &inputRowY, &inputColY);\n";
	srcStdStr += "}\n"; // End for loop


	srcStdStr += "float8 outputvec=(float8)(sumXX,sumXY,sumYX,sumYY);\n";
	// srcStdStr += "cross_correlation[i] += outputvec;\n";

	if (d_pipeline_integration > 1) {
		srcStdStr += "cross_correlation[i] += outputvec;\n";
	}
	else {
		srcStdStr += "cross_correlation[i] = outputvec;\n";
	}

	srcStdStr += "}\n"; // End function

	if (debugMode) {
		GR_LOG_INFO(d_logger,"Using Correlate Kernel:");
		GR_LOG_INFO(d_logger,srcStdStr.c_str());
	}
	GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str(),true,"-cl-fast-relaxed-math");
}

void clXEngine_impl::buildKernel() {
	std::string srcStdStr="";
	std::string fnName = "XCorrelate";

	// Use #defines to not have to pass them in as params since they won't change
	// This will save time on unchanging param transfers at runtime.
	srcStdStr += "#define d_num_baselines " + std::to_string(d_num_baselines) + "\n";
	srcStdStr += "#define d_integration_time " + std::to_string(d_integration_time) + "\n";
	srcStdStr += "#define d_num_channels " + std::to_string(d_num_channels) + "\n";
	srcStdStr += "#define d_num_inputs " + std::to_string(d_num_inputs) + "\n";
	srcStdStr += "#define d_npol " + std::to_string(d_npol) + "\n";
	// frame_size = inputs * channels * pol
	srcStdStr += "#define d_frame_size " + std::to_string(frame_size) + "\n";
	srcStdStr += "\n"; // Just for legibility

	srcStdStr += "struct ComplexStruct {\n";
	srcStdStr += "float real;\n";
	srcStdStr += "float imag; };\n";
	srcStdStr += "typedef struct ComplexStruct XComplex;\n";

	srcStdStr += "__attribute__((always_inline)) inline void cxmac(XComplex* accum, XComplex* z0, XComplex* z1) {\n";
	if (hasDoubleFMASupport || hasSingleFMASupport) {
		srcStdStr += "	accum->real += fma(z0->real,z1->real,(z0->imag * z1->imag));\n";
		srcStdStr += "	accum->imag += fma(z0->imag, z1->real, (-z0->real * z1->imag));\n";
	}
	else {
		srcStdStr += "	accum->real += z0->real * z1->real + z0->imag * z1->imag;\n";
		srcStdStr += "	accum->imag += z0->imag * z1->real - z0->real * z1->imag;\n";
	}
	srcStdStr += "}\n\n";

	srcStdStr += "__kernel void XCorrelate(__global XComplex * restrict input_matrix, __global XComplex * restrict cross_correlation) {\n";
	srcStdStr += "size_t i =  get_global_id(0);\n";

	srcStdStr += "int f = i/d_num_baselines;\n";
	srcStdStr += "int k = i - f*d_num_baselines;\n";
	if (hasDoubleFMASupport || hasSingleFMASupport) {
		srcStdStr += "int station1 = -0.5 + sqrt(fma(2.0,k,0.25));\n";
	}
	else {
		srcStdStr += "int station1 = -0.5 + sqrt(0.25 + 2*k);\n";
	}
	srcStdStr += "int station2 = k - ((station1+1)*station1)/2;\n";
	srcStdStr += "XComplex sumXX; sumXX.real = 0.0; sumXX.imag = 0.0;\n";
	srcStdStr += "XComplex sumXY; sumXY.real = 0.0; sumXY.imag = 0.0;\n";
	srcStdStr += "XComplex sumYX; sumYX.real = 0.0; sumYX.imag = 0.0;\n";
	srcStdStr += "XComplex sumYY; sumYY.real = 0.0; sumYY.imag = 0.0;\n";
	srcStdStr += "XComplex inputRowX, inputRowY, inputColX, inputColY;\n";

	srcStdStr += "for(int t=0; t<d_integration_time; t++){\n";
	// Had to adapt index lookups.  xGPU was expecting [t][freq][station][pol],
	// But we have [t][station][freq][pol]
	// So the index1/2 calcs are different than xGPU's
	/*
	srcStdStr += "  int index_base = (t*d_num_channels + f)*d_num_inputs;\n";
	srcStdStr += "  int index1 = (index_base + station1)*d_npol;\n";
	srcStdStr += "  int index2 = (index_base + station2)*d_npol;\n";
	*/
	srcStdStr += "  int index1 = t*d_frame_size + (station1 * d_num_channels + f) * d_npol;\n";
	srcStdStr += "  int index2 = t*d_frame_size + (station2 * d_num_channels + f) * d_npol;\n";

	srcStdStr += "	inputRowX = input_matrix[index1];\n";
	srcStdStr += "	inputColX = input_matrix[index2];\n";

	srcStdStr += "	cxmac(&sumXX, &inputRowX, &inputColX);\n";
	// Doing the If's at compile time saves the kernels from having to synchronize on the if's
	// so they can just run.
	if (d_npol > 1) {
		srcStdStr += "	inputRowY = input_matrix[index1 + 1];\n";
		srcStdStr += "	inputColY = input_matrix[index2 + 1];\n";

		srcStdStr += "	cxmac(&sumXY, &inputRowX, &inputColY);\n";
		srcStdStr += "	cxmac(&sumYX, &inputRowY, &inputColX);\n";
		srcStdStr += "	cxmac(&sumYY, &inputRowY, &inputColY);\n";
	}
	srcStdStr += "}\n"; // End for loop

	if (d_pipeline_integration > 1) {
		if (d_npol == 1) {
			srcStdStr += "cross_correlation[i    ] += sumXX;\n";
		}
		else {
			srcStdStr += "int four_i = 4*i;\n";
			srcStdStr += "cross_correlation[four_i    ] += sumXX;\n";
			srcStdStr += "cross_correlation[four_i + 1] += sumXY;\n";
			srcStdStr += "cross_correlation[four_i + 2] += sumYX;\n";
			srcStdStr += "cross_correlation[four_i + 3] += sumYY;\n";
		}
	}
	else {
		if (d_npol == 1) {
			srcStdStr += "cross_correlation[i    ] = sumXX;\n";
		}
		else {
			srcStdStr += "int four_i = 4*i;\n";
			srcStdStr += "cross_correlation[four_i    ] = sumXX;\n";
			srcStdStr += "cross_correlation[four_i + 1] = sumXY;\n";
			srcStdStr += "cross_correlation[four_i + 2] = sumYX;\n";
			srcStdStr += "cross_correlation[four_i + 3] = sumYY;\n";
		}
	}

	srcStdStr += "}\n"; // End function

	if (debugMode) {
		GR_LOG_INFO(d_logger,"Using Correlate Kernel:");
		GR_LOG_INFO(d_logger,srcStdStr.c_str());
	}
	GRCLBase::CompileKernel((const char *)srcStdStr.c_str(),(const char *)fnName.c_str(),true,"-cl-fast-relaxed-math");
}

void clXEngine_impl::buildCharToComplexKernel() {
	// Now we set up our OpenCL kernel
	std::string srcStdStr="";
	std::string fnName = "CharToComplex";

	// Switch division to multiplication for speed.

	srcStdStr += "struct ComplexStruct {\n";
	srcStdStr += "float real;\n";
	srcStdStr += "float imag; };\n";
	srcStdStr += "typedef struct ComplexStruct SComplex;\n";

	if (d_data_type == DTYPE_PACKEDXY) {
		// Need a two's complement lookup table.
		srcStdStr += "__constant char twosComplementLUT[16] = {0, 1, 2, 3, 4, 5, 6, 7, 0,-7,-6,-5,-4,-3,-2,-1};\n";
		// In this mode, we know we have packed 4-bit in.  So we can use that to determine full scale.
		srcStdStr += "#define ONE_OVER_S4BIT_MAX 0.142857142857142857143\n";
		srcStdStr += "#define NTIMES " + std::to_string(d_integration_time) + "\n";
		srcStdStr += "#define d_num_channels " + std::to_string(d_num_channels) + "\n";
		srcStdStr += "#define d_num_inputs " + std::to_string(d_num_inputs) + "\n";
		srcStdStr += "#define a_f_size " + std::to_string(d_num_inputs * d_num_channels) + "\n";

		srcStdStr += "__attribute__((always_inline)) inline size_t map_output_index(size_t input_index) {\n";
		srcStdStr += "   size_t t = input_index / a_f_size;\n";
		srcStdStr += "   size_t full_blocks = t * a_f_size;\n";
		srcStdStr += "   size_t sub_block = input_index - full_blocks;\n";
		srcStdStr += "   size_t a = sub_block / d_num_channels;\n";
		srcStdStr += "   size_t f = sub_block - a * d_num_channels;\n";
		srcStdStr += "   return (f*d_num_inputs*NTIMES + a * NTIMES + t);\n";
		srcStdStr += "}\n";

		srcStdStr += "__kernel void CharToComplex(__global uchar2 * restrict a, __global float4 * restrict c) {\n";
		srcStdStr += "    size_t index =  get_global_id(0);\n";
		srcStdStr += "    uchar2 tmp_char1 = a[index];\n";
		srcStdStr += "    size_t new_index = map_output_index(index);\n";
		srcStdStr += "	  c[new_index].x = (float)twosComplementLUT[tmp_char1.x >> 4] * ONE_OVER_S4BIT_MAX;\n";
		srcStdStr += "	  c[new_index].y = (float)twosComplementLUT[tmp_char1.x & 0x0F] * ONE_OVER_S4BIT_MAX;\n";
		srcStdStr += "	  c[new_index].z = (float)twosComplementLUT[tmp_char1.y >> 4] * ONE_OVER_S4BIT_MAX;\n";
		srcStdStr += "	  c[new_index].w = (float)twosComplementLUT[tmp_char1.y & 0x0F] * ONE_OVER_S4BIT_MAX;\n";
	}
	else {
		// In this mode, we don't know what produced the byte input, so we'll have to use full SCHAR scale
		srcStdStr += "#define ONE_OVER_SCHAR_MAX 0.007874015748031496063\n";
		srcStdStr += "__kernel void CharToComplex(__global char * restrict a, __global SComplex * restrict c) {\n";
		srcStdStr += "    size_t index =  get_global_id(0);\n";
		srcStdStr += "    size_t two_index =  index*2;\n";
		srcStdStr += "	  c[index].real = (float)a[two_index] * ONE_OVER_SCHAR_MAX;\n";
		srcStdStr += "	  c[index].imag = (float)a[two_index+1] * ONE_OVER_SCHAR_MAX;\n";
	}
	srcStdStr += "}\n";

	if (debugMode) {
		GR_LOG_INFO(d_logger,"Using char to complex Kernel:");
		GR_LOG_INFO(d_logger,srcStdStr.c_str());
	}

	try {
		// Create and program from source
		if (char_to_cc_program) {
			delete char_to_cc_program;
			char_to_cc_program = NULL;
		}
		if (char_to_cc_sources) {
			delete char_to_cc_sources;
			char_to_cc_sources = NULL;
		}
		char_to_cc_sources=new cl::Program::Sources(1, std::make_pair(srcStdStr.c_str(), 0));
		char_to_cc_program = new cl::Program(*context, *char_to_cc_sources);

		// Build program
		// This seems faster withtout fast-math
		char_to_cc_program->build(devices);

		char_to_cc_kernel=new cl::Kernel(*char_to_cc_program, (const char *)fnName.c_str());
	}
	catch(cl::Error& e) {
		std::cout << "OpenCL Error compiling kernel for " << fnName << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what() << std::endl;
		std::cout << srcStdStr << std::endl;
		exit(0);
	}

	try {
		preferredWorkGroupSizeMultiple = char_to_cc_kernel->getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(devices[0]);
	}
	catch(cl::Error& e) {
		std::cout << "Error getting kernel preferred work group size multiple" << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what()<< std::endl;
	}

	try {
		maxWorkGroupSize = char_to_cc_kernel->getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0]);
	}
	catch(cl::Error& e) {
		std::cout << "Error getting kernel preferred work group size multiple" << std::endl;
		std::cout << "OpenCL Error " << e.err() << ": " << e.what()<< std::endl;
	}
}

int
clXEngine_impl::work_processor(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items, bool liveWork)
{
	gr::thread::scoped_lock guard(d_setlock);

	int items_remaining = d_integration_time - integration_tracker;

	int items_processed;

	if (noutput_items > items_remaining) {
		items_processed = items_remaining;
	}
	else {
		items_processed = noutput_items;
	}

	if (thread_process_data && ((integration_tracker+items_processed) == d_integration_time)) {
		// If a thread is already processing data and this would trigger a new one,
		// the buffer has backed up.  Let's hold and tell the engine we're not ready for this data.

		/*
		while (thread_process_data) {
			usleep(4);
		}
		*/
		gr::thread::scoped_lock guard(d_thread_active_lock);
		// return 0;
	}

	if (!d_use_internal_synchronizer) {
		// with the internal synchronizer, we'll handle this once when we sync
		if (d_fp && !d_wrote_json) {
			// We're writing to file and we haven't written any bytes to the current file
			unsigned long lowest_tag = -1;

			for (int cur_input=0;cur_input<d_num_inputs;cur_input++) {
				std::vector<gr::tag_t> tags;
				this->get_tags_in_window(tags, cur_input, 0, noutput_items);

				int cur_tag = 0;
				int tag_size = tags.size();

				// Find the lowest tag number in this set.
				while ( cur_tag < tag_size ) {
					long tag_val = pmt::to_long(tags[cur_tag].value);
					if ( (tag_val >=0) && ( (lowest_tag == -1) || (tag_val < lowest_tag) ) ) {
						lowest_tag = tag_val;
						break;
					}
					cur_tag++;
				}

			}

			// If we found a tag, this'll write the lowest value
			if (lowest_tag >= 0) {
				write_json(lowest_tag);
				current_timestamp = lowest_tag;
			}
		}
	}

	// First we need to load the data into a matrix in the format expected by the correlator.
	// For a single polarization it's easy, we can just chain them.
	// For dual polarization, we have to interleave them so it's
	// x0r x0i y0r y0i.....

	for (int cur_block=0;cur_block<items_processed;cur_block++) {
		// For reference: 	frame_size = d_num_channels * d_num_inputs * d_npol;
		int input_start = frame_size * (integration_tracker + cur_block);

		if (d_data_type == DTYPE_BYTE) {
			input_start *= d_data_size; // interleaved IQ, so 2 data_size's at a clip
		}

		if (d_npol == 1) {
			if (d_data_type == DTYPE_BYTE) {
				for (int i=0;i<d_num_inputs;i++) {
					const char *cur_signal = (const char *) input_items[i];
					memcpy(&char_input[input_start + i*d_num_channels*d_data_size],&cur_signal[cur_block*input_size],input_size);
				}
			}
			else {
				for (int i=0;i<d_num_inputs;i++) {
					const gr_complex *cur_signal = (const gr_complex *) input_items[i];
					memcpy(&complex_input[input_start + i*d_num_channels],&cur_signal[cur_block*d_num_channels],input_size);
				}
			}
		}
		else {
			if (d_data_type == DTYPE_BYTE) {
				// We need to interleave....
				for (int i=0;i<d_num_inputs;i++) {
					const char *pol1 = (const char *) input_items[i];
					const char *pol2 = (const char *) input_items[i+d_num_inputs];

					// Each interleaved channel will now be num_channels*2 long
					// X Y X Y X Y...
					for (int k=0;k<d_num_channels;k++) {
						int input_index = input_start + i*num_chan_x2*2+k*4;
						int pol_index = cur_block*num_chan_x2+k*2;
						memcpy(&char_input[input_index],&pol1[pol_index],d_data_size);
						memcpy(&char_input[input_index+2],&pol2[pol_index],d_data_size);
					}
				}
			}
			else if (d_data_type == DTYPE_PACKEDXY) {
				// Already interleaved
				int pol_index = cur_block*num_chan_x2;
				for (int i=0;i<d_num_inputs;i++) {
					const char *pol1 = (const char *) input_items[i];
					int input_index = input_start + i*num_chan_x2;
					// Each interleaved channel will now be num_channels*2 long
					// X Y X Y X Y...
					// In packed xy mode, the input is already xyxy, just packed 4-bit in each byte.
					memcpy(&char_input[input_index],&pol1[pol_index],num_chan_x2);
				}
			}
			else {
				// We need to interleave....
				for (int i=0;i<d_num_inputs;i++) {
					const gr_complex *pol1 = (const gr_complex *) input_items[i];
					const gr_complex *pol2 = (const gr_complex *) input_items[i+d_num_inputs];

					// Each interleaved channel will now be num_channels*2 long
					// X Y X Y X Y...
					for (int k=0;k<d_num_channels;k++) {
						int input_index = input_start + i*num_chan_x2+k*2;
						int pol_index = cur_block*d_num_channels+k;
						// complex_input[input_index++] = pol1[pol_index];
						// complex_input[input_index] = pol2[pol_index];
						// The memcpy is slightly faster.
						// sizeof(gr_complex) is faster than d_data_size
						memcpy(&complex_input[input_index++],&pol1[pol_index],sizeof(gr_complex));
						memcpy(&complex_input[input_index],&pol2[pol_index],sizeof(gr_complex));
					}
				} // for i
			} // else datatype
		} // else interleave

		current_timestamp++;
	} // for curblock

	integration_tracker += items_processed;

	if (integration_tracker == d_integration_time) {
		// Buffer is ready for processing.
		if (!thread_process_data) {
			// thread_is_processing will only be FALSE if thread_process_data == false on the first pass,
			// in which case we don't want to send any pmt's.  Otherwise, we're in async pickup mode.
			if (thread_is_processing && (!d_output_file) && (!d_disable_output)) {
				// So this case is that we have a new block ready and the old one is complete.
				// So before transitioning to the new one, let's send the data from the last one.

				if (d_pipeline_integration < 2) {
					// We're not CPU accumulating too, so just send
					pmt::pmt_t corr_out(pmt::init_c32vector(matrix_flat_length,thread_output_matrix));
					pmt::pmt_t pdu = pmt::cons(pmt::string_to_symbol("triang_matrix"), corr_out);

					if (liveWork) {
						message_port_pub(pmt::mp("xcorr"), pdu);
					}
				}
				else {
					// We're also CPU accumulating.
					if (d_pipeline_integration_counter > d_pipeline_integration) {
						pmt::pmt_t corr_out(pmt::init_c32vector(matrix_flat_length,thread_output_matrix));
						pmt::pmt_t pdu = pmt::cons(pmt::string_to_symbol("triang_matrix"), corr_out);

						if (liveWork) {
							message_port_pub(pmt::mp("xcorr"), pdu);
						}
						d_pipeline_integration_counter = 1;
					}
				}
			}

			// Set up the pointers to the new data the thread should work with.
			thread_output_matrix = output_matrix;
			if (d_data_type == DTYPE_COMPLEX) {
				thread_complex_input = complex_input;

				// Move the current pointer to the other buffer
				if (current_write_buffer == 1) {
					// Move to buffer 2
					complex_input = complex_input2;
					output_matrix = output_matrix2;
					current_write_buffer = 2;
				}
				else {
					// Move to buffer 1
					complex_input = complex_input1;
					output_matrix = output_matrix1;
					current_write_buffer = 1;
				}
			}
			else {
				thread_char_input = char_input;

				// Move the current pointer to the other buffer
				if (current_write_buffer == 1) {
					// Move to buffer 2
					char_input = char_input2;
					output_matrix = output_matrix2;
					current_write_buffer = 2;
				}
				else {
					// Move to buffer 1
					char_input = char_input1;
					output_matrix = output_matrix1;
					current_write_buffer = 1;
				}
			}

			// Trigger the thread to process
			thread_process_data = true;
		}

		integration_tracker = 0;
	}

	// Tell runtime system how many output items we produced.
	return items_processed;
}

int
clXEngine_impl::work_test(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	return work_processor(noutput_items, input_items, output_items, false);
}

int
clXEngine_impl::general_work (int noutput_items,
		gr_vector_int &ninput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	if (d_use_internal_synchronizer && !d_synchronized) {
		// We need to synchronize.
		// Each timestamp will always be t[n+1] = t[n] + 16
		// Look at each input channel's tags and find the highest starting tag.
		// Take the timestamp diff for each input, and that's how many items we need to consume.
		uint64_t highest_tag = 0;
		uint64_t first_input_timestamp;

		bool test_sync = true;

		// Find the highest tag in the first slot.
		for (int cur_input=0;cur_input<d_num_inputs;cur_input++) {
			std::vector<gr::tag_t> tags;
			// We only need the first tag.  No need to get them all.
			this->get_tags_in_window(tags, cur_input, 0, 1);

			uint64_t tag0 = pmt::to_uint64(tags[0].value);

			if (cur_input == 0) {
				first_input_timestamp = tag0;
			}
			else {
				if (tag0 != first_input_timestamp) {
					test_sync = false;
				}
			}

			// Save these so we don't have to get tags and iterate again.
			tag_list[cur_input] = tag0;

			if (tag0 > highest_tag) {
				highest_tag = tag0;
			}
		}

		if (test_sync) {
			// we're actually now synchronized.  We'll set our sync flag and process as if we came in sync'd
			d_synchronized = true;
			current_timestamp = highest_tag;

			if (d_fp && !d_wrote_json) {
					write_json(highest_tag);
			}

	        pmt::pmt_t pdu = pmt::cons( pmt::intern("synctimestamp"), pmt::from_uint64(highest_tag) );
			message_port_pub(pmt::mp("sync"),pdu);

			std::stringstream msg_stream;
			msg_stream << "Synchronized on timestamp " << highest_tag;
			GR_LOG_INFO(d_logger, msg_stream.str());
		}
		else {
			// So we're still not sync'd so we need to figure out what we need to dump.
			for (int cur_input=0;cur_input<d_num_inputs;cur_input++) {

				// tag_diff will increment by 16 with the tag #'s so no need to divide by 16.
				// We need this # anyway.
				uint64_t items_to_consume = highest_tag - tag_list[cur_input];

				if (items_to_consume > noutput_items)
					items_to_consume = noutput_items;

				consume(cur_input,items_to_consume);
			}

			// We're going to return 0 here so we don't forward any data along yet.  That won't happen till we're synchronized.
			return 0;
		}
	}

	int items_processed = work_processor(noutput_items, input_items, output_items, true);

	consume_each (items_processed);
	return items_processed;
}

void clXEngine_impl::runThread() {
	threadRunning = true;

	while (!stop_thread) {
		if (thread_process_data) {
			gr::thread::scoped_lock guard(d_thread_active_lock);
			// This is really a one-time variable set to true on the first pass.
			thread_is_processing = true;

			if (d_data_type == DTYPE_COMPLEX) {
				xcorrelate((XComplex *)thread_complex_input, (XComplex *)thread_output_matrix);
			}
			else {
				xcorrelate(thread_char_input, (XComplex *)thread_output_matrix);
			}

			if (d_pipeline_integration > 1) {
				d_pipeline_integration_counter++;
			}

			// If we've completed our integration
			if ( (d_pipeline_integration < 2) || (d_pipeline_integration_counter > d_pipeline_integration)) {
				// Read the correlation back and wait for the memory transfer to happen.
				queue->enqueueReadBuffer(*cross_correlation_buffer,CL_TRUE,0,output_size,(void *)thread_output_matrix);

				if (d_fp) {
					long nwritten = 0;

					// If we have an open file, we're supposed to rollover files, and we're over our limit
					// reset the file.
					if ((d_fp) && (rollover_size_bytes > 0) && (d_bytesWritten >= rollover_size_bytes)) {
						close();
						open();
					}

					// Optimize write as one call
					long count = fwrite((char *)thread_output_matrix, matrix_flat_length * sizeof(gr_complex),1,d_fp);
					if(count == 0) {
						// Error condition, nothing written for some reason.
						if(ferror(d_fp)) {
							std::cout << "[X-Engine] Write failed with error: " << std::strerror(errno) << std::endl;
						}
					}
				}

				// Given the host pointer, this needs to be done after we use thread_output_matrix above.
				if (d_pipeline_integration > 1) {
					// Now zero out memory for the next cycle
					queue->enqueueFillBuffer(*cross_correlation_buffer,f_zero,0,output_size);

					d_pipeline_integration_counter = 1;
				}
			}

			// clear the trigger.  This will inform that data is ready.
			thread_process_data = false;
		}

		int ct = 0;

		while (!thread_process_data && (ct++ < 4) ) {
			usleep(2);
		}
	}

	threadRunning = false;
}

} /* namespace clenabled */
} /* namespace gr */

