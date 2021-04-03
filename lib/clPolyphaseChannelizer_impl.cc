/* -*- c++ -*- */
/*
 * Copyright 2020 Aaron Giles and Dan Banks.
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

#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY

#include <gnuradio/io_signature.h>
#include "clPolyphaseChannelizer_impl.h"

namespace gr {
  namespace clenabled {

    clPolyphaseChannelizer::sptr
    clPolyphaseChannelizer::make(int openCLPlatformType, int devSelector, int platformId, int devId, const std::vector<float> &taps, int buf_items, int num_channels, int ninputs_per_iter, const std::vector<int> &ch_map, int setDebug)
    {
        if (setDebug == 1)
            return gnuradio::get_initial_sptr
                (new clPolyphaseChannelizer_impl(openCLPlatformType, devSelector, platformId, devId,
                                                 taps, buf_items, num_channels, ninputs_per_iter, ch_map, true));
        else
            return gnuradio::get_initial_sptr
                (new clPolyphaseChannelizer_impl(openCLPlatformType, devSelector, platformId, devId,
                                                 taps, buf_items, num_channels, ninputs_per_iter, ch_map, false));
    }

    /*
     * The private constructor
     */
    clPolyphaseChannelizer_impl::clPolyphaseChannelizer_impl(int openCLPlatformType, int devSelector, int platformId, int devId,
                                                             const std::vector<float> &taps, int buf_items, int num_channels, int ninputs_per_iter, const std::vector<int> &ch_map, bool setDebug)
      : gr::block("clPolyphaseChannelizer",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
        d_taps(taps),
        d_buf_items(buf_items),
        d_num_channels(num_channels),
        d_ninputs_per_iter(ninputs_per_iter),
        d_ch_map(ch_map),
        GRCLBase(DTYPE_COMPLEX, sizeof(gr_complex), openCLPlatformType, devSelector, platformId, devId, setDebug)
    {
        if(buf_items % num_channels != 0)
        {
            throw std::invalid_argument("buf_items must be a multiple of num_channels");
        }
        set_history(taps.size());
        set_output_multiple(d_ch_map.size()*d_buf_items/d_ninputs_per_iter);
        init_opencl();
        init_clfft();
    }

    /*
     * Our virtual destructor.
     */
    clPolyphaseChannelizer_impl::~clPolyphaseChannelizer_impl()
    {
    }

    void
    clPolyphaseChannelizer_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
        ninput_items_required[0] = d_ninputs_per_iter*noutput_items/d_ch_map.size() + history() - d_num_channels;
    }

    int
    clPolyphaseChannelizer_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const gr_complex *in = (const gr_complex *) input_items[0];
        gr_complex *out = (gr_complex *) output_items[0];

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.


        queue->enqueueWriteBuffer(*d_in_clmem, CL_FALSE, 0, (d_buf_items+history()-d_num_channels)*sizeof(gr_complex), in);
        cl::NDRange global_work_size = cl::NDRange((size_t)d_buf_items/d_ninputs_per_iter, (size_t)d_num_channels);
        queue->enqueueNDRangeKernel(*d_kernel, cl::NullRange, global_work_size);
        clfftEnqueueTransform(d_plan_handle, CLFFT_BACKWARD, 1, &((*queue)()), 0, NULL, NULL, &((*d_filt_clmem)()), &((*d_fft_clmem)()), NULL);
        cl::NDRange global_work_size_chmap = cl::NDRange((size_t)d_buf_items/d_ninputs_per_iter, (size_t)d_ch_map.size());
        queue->enqueueNDRangeKernel(*d_kernel_chmap, cl::NullRange, global_work_size_chmap);
        queue->enqueueReadBuffer(*d_mapout_clmem, CL_TRUE, 0, d_ch_map.size()*d_buf_items/d_ninputs_per_iter*sizeof(gr_complex), out);

        consume_each (d_buf_items);

      // Tell runtime system how many output items we produced.
        return d_ch_map.size()*d_buf_items/d_ninputs_per_iter;
    }

    void
    clPolyphaseChannelizer_impl::init_opencl()
    {
        d_in_clmem = new cl::Buffer(
            *context,
            CL_MEM_READ_ONLY,
            (d_buf_items+history()-d_num_channels) * sizeof(gr_complex));

        d_filt_clmem = new cl::Buffer(
            *context,
            CL_MEM_READ_WRITE,
            d_num_channels*d_buf_items/d_ninputs_per_iter * sizeof(gr_complex));

        d_fft_clmem = new cl::Buffer(
            *context,
            CL_MEM_READ_WRITE,
            d_num_channels*d_buf_items/d_ninputs_per_iter * sizeof(gr_complex));

        d_mapout_clmem = new cl::Buffer(
            *context,
            CL_MEM_WRITE_ONLY,
            d_ch_map.size()*d_buf_items/d_ninputs_per_iter * sizeof(gr_complex));

        d_taps_clmem = new cl::Buffer(
            *context,
            CL_MEM_READ_ONLY,
            d_taps.size() * sizeof(float));

        queue->enqueueWriteBuffer(*d_taps_clmem, CL_TRUE, 0, d_taps.size()*sizeof(float), &d_taps[0]);

        d_chmap_clmem = new cl::Buffer(
            *context,
            CL_MEM_READ_ONLY,
            d_ch_map.size() * sizeof(int));

        queue->enqueueWriteBuffer(*d_chmap_clmem, CL_TRUE, 0, d_ch_map.size()*sizeof(int), &d_ch_map[0]);

        buildProgram();
    }



    void clPolyphaseChannelizer_impl::buildProgram()
    {
        d_programCode = "";
        d_programCode += "__kernel void filterpfb2(const __global float2 * restrict in, __global float2 * restrict out, const int num_taps, const __global float * restrict taps, const int num_channels, const int input_rate)\n";
        d_programCode += "{\n";
        d_programCode += "    const int i = get_global_id(0); // which output sample of the jth subfilter\n";
        d_programCode += "    const int j = get_global_id(1); // which subfilter\n";

        d_programCode += "    float2 acc=0;\n";
        d_programCode += "    for (int k=j;k<num_taps;k+=num_channels)\n";
        d_programCode += "    {\n";
        d_programCode += "        acc = fma(in[i*input_rate-k+num_taps-1], taps[k], acc);\n";
        d_programCode += "    }\n";
        d_programCode += "    out[i*num_channels+(j+i*(num_channels-input_rate))%num_channels]=acc;\n";
        d_programCode += "}\n";

        d_programCode += "__kernel void channel_map(const __global float2 * restrict in, __global float2 * restrict out, const int num_channels, const __global int * restrict channels)\n";
        d_programCode += "{\n";
        d_programCode += "    const int i = get_global_id(0); // which output sample of the jth channel\n";
        d_programCode += "    const int j = get_global_id(1); // which output channel index\n";

        d_programCode += "    const int num_out_channels = get_global_size(1);\n";

        d_programCode += "    out[i*num_out_channels+j] = in[i*num_channels+channels[j]];\n";
        d_programCode += "}\n";

        try {
            cl::Program::Sources sources = cl::Program::Sources(1, std::make_pair((const char *)d_programCode.c_str(), 0));
            d_program = new cl::Program(*context, sources);
            d_program->build(devices);

            d_kernel = new cl::Kernel(*d_program, "filterpfb2");
            int num_taps = d_taps.size();
            d_kernel->setArg(0, *d_in_clmem);
            d_kernel->setArg(1, *d_filt_clmem);
            d_kernel->setArg(2, num_taps);
            d_kernel->setArg(3, *d_taps_clmem);
            d_kernel->setArg(4, d_num_channels);
            d_kernel->setArg(5, d_ninputs_per_iter);

            d_kernel_chmap = new cl::Kernel(*d_program, "channel_map");
            d_kernel_chmap->setArg(0, *d_fft_clmem);
            d_kernel_chmap->setArg(1, *d_mapout_clmem);
            d_kernel_chmap->setArg(2, d_num_channels);
            d_kernel_chmap->setArg(3, *d_chmap_clmem);

        }
        catch(cl::Error& e) {
            std::cout << "OpenCL error building program and kernels." << std::endl;
            std::cout << "OpenCL error " << e.err() << ": " << e.what() << std::endl;
            //std::cout << kernelCode << std::endl;
            exit(0);
        }
    }

    void clPolyphaseChannelizer_impl::init_clfft()
    {
        clfftStatus err;
        clfftSetupData fft_setup;
        clfftDim dim = CLFFT_1D;
        size_t cl_lengths[1] = {(size_t)d_num_channels};

        err = clfftInitSetupData(&fft_setup);
        err = clfftSetup(&fft_setup);
        err = clfftCreateDefaultPlan(&d_plan_handle, (*context)(), dim, cl_lengths);
        err = clfftSetPlanBatchSize(d_plan_handle, d_buf_items/d_ninputs_per_iter);
        err = clfftSetPlanPrecision(d_plan_handle, CLFFT_SINGLE);
        err = clfftSetPlanScale(d_plan_handle, CLFFT_BACKWARD, 1.0f);
        err = clfftSetResultLocation(d_plan_handle, CLFFT_OUTOFPLACE);
        err = clfftSetLayout(d_plan_handle, CLFFT_COMPLEX_INTERLEAVED, CLFFT_COMPLEX_INTERLEAVED);

        err = clfftBakePlan(d_plan_handle, 1, &(*queue)(), NULL, NULL);
    }

    bool
    clPolyphaseChannelizer_impl::stop()
    {
        clfftStatus clerr;
        clerr = clfftTeardown();
        if (clerr!=CLFFT_SUCCESS)
        {
            printf("error tearing down clFFT\n");
        }


        if (d_in_clmem) {
            delete d_in_clmem;
            d_in_clmem = NULL;
        }

        if (d_filt_clmem) {
            delete d_filt_clmem;
            d_filt_clmem = NULL;
        }

        if (d_fft_clmem) {
            delete d_fft_clmem;
            d_fft_clmem = NULL;
        }

        if (d_mapout_clmem) {
            delete d_mapout_clmem;
            d_mapout_clmem = NULL;
        }

        if (d_taps_clmem) {
            delete d_taps_clmem;
            d_taps_clmem = NULL;
        }

        if (d_chmap_clmem) {
            delete d_chmap_clmem;
            d_chmap_clmem = NULL;
        }

        try {
            if (d_kernel != NULL) {
                delete kernel;
                kernel=NULL;
            }
            if (d_kernel_chmap != NULL) {
                delete d_kernel_chmap;
                d_kernel_chmap=NULL;
            }
        }
        catch (...) {
            kernel=NULL;
            d_kernel_chmap=NULL;
            std::cout<<"Kernel delete error." << std::endl;
        }

        if (d_program) {
            delete d_program;
            d_program = NULL;
        }

        return GRCLBase::stop();
    }

  } /* namespace clenabled */
} /* namespace gr */

