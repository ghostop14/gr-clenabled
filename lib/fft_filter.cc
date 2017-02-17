/*
 * fft_filter.cc
 *
 *  Created on: Feb 10, 2017
 *      Author: root
 */

#include "fft_filter.h"
#include <volk/volk.h>

namespace gr {
  namespace clenabled {


  fft_filter_ccf::fft_filter_ccf(int decimation,
			     const std::vector<float> &taps,
			     int nthreads)
: d_fftsize(-1), d_decimation(decimation), d_fwdfft(NULL),
  d_invfft(NULL), d_nthreads(nthreads), d_xformed_taps(NULL)
  {
	  set_taps(taps);
  }

  fft_filter_ccf::~fft_filter_ccf()
  {
	if (d_fwdfft != NULL) {
	  delete d_fwdfft;
	  d_fwdfft = NULL;
	}

	if (d_invfft != NULL) {
	  delete d_invfft;
	  d_invfft = NULL;
	}

	if(d_xformed_taps != NULL) {
	  volk_free(d_xformed_taps);
	  d_xformed_taps = NULL;
	}
  }

  /*
   * determines d_ntaps, d_nsamples, d_fftsize, d_xformed_taps
   */
  int
  fft_filter_ccf::set_taps(const std::vector<float> &taps)
  {
	int i = 0;
		d_taps = taps;
	compute_sizes(taps.size());

	d_tail.resize(tailsize());
	for(i = 0; i < tailsize(); i++)
	  d_tail[i] = 0;

	gr_complex *in = d_fwdfft->get_inbuf();
	gr_complex *out = d_fwdfft->get_outbuf();

	float scale = 1.0 / d_fftsize;

	// Compute forward xform of taps.
	// Copy taps into first ntaps slots, then pad with zeros
	for(i = 0; i < d_ntaps; i++)
	  in[i] = gr_complex(taps[i] * scale, 0.0f);

	for(i=d_ntaps; i < d_fftsize; i++)
	  in[i] = gr_complex(0.0f, 0.0f);

	d_fwdfft->execute();		// do the xform

	// now copy output to d_xformed_taps
	for(i = 0; i < d_fftsize; i++)
	  d_xformed_taps[i] = out[i];

	return d_nsamples;
  }

  // determine and set d_ntaps, d_nsamples, d_fftsize
  void
  fft_filter_ccf::compute_sizes(int ntaps)
  {
	int old_fftsize = d_fftsize;
	d_ntaps = ntaps;
	d_fftsize = (int) (2 * pow(2.0, ceil(log(double(ntaps)) / log(2.0))));
	d_nsamples = d_fftsize - d_ntaps + 1;
	/*
	if(VERBOSE) {
	  std::cerr << "fft_filter_ccf: ntaps = " << d_ntaps
			<< " fftsize = " << d_fftsize
			<< " nsamples = " << d_nsamples << std::endl;
	}
	*/
	// compute new plans
	if(d_fftsize != old_fftsize) {
		try {
			if (d_fwdfft != NULL) {
				delete d_fwdfft;
			}
		}
		catch (...) {
			std::cout << "FFT_Filter_ccf::compute_sizes: exception deleting d_fwdfft" << std::endl;
		}

		try {
			if (d_invfft) {
				delete d_invfft;
			}
		}
		catch (...) {
			std::cout << "FFT_Filter_ccf::compute_sizes: exception deleting d_invfft" << std::endl;
		}

		try {
			if(d_xformed_taps != NULL) {
				try {
				volk_free(d_xformed_taps);
				}
				catch (...) {

				}
			}
		}
		catch (...) {
			std::cout << "FFT_Filter_ccf::compute_sizes: exception volk_free(d_xformed_taps)" << std::endl;
		}


		d_fwdfft = new fft_complex(d_fftsize, true, d_nthreads);
		d_invfft = new fft_complex(d_fftsize, false, d_nthreads);

		d_xformed_taps = (gr_complex*)volk_malloc(sizeof(gr_complex)*d_fftsize,volk_get_alignment());
	}
}

  void
  fft_filter_ccf::set_nthreads(int n)
  {
	d_nthreads = n;
	if(d_fwdfft)
	  d_fwdfft->set_nthreads(n);
	if(d_invfft)
	  d_invfft->set_nthreads(n);
  }

  std::vector<float>
  fft_filter_ccf::taps() const
  {
    return d_taps;
  }

  unsigned int
  fft_filter_ccf::ntaps() const
  {
    return d_ntaps;
  }

  unsigned int
  fft_filter_ccf::filtersize() const
  {
    return d_fftsize;
  }

  int
  fft_filter_ccf::nthreads() const
  {
	  return d_nthreads;
  }

  int
  fft_filter_ccf::filter(int nitems, const gr_complex *input, gr_complex *output)
  {
	if (!d_fwdfft || !d_invfft)
		return 0;

	int dec_ctr = 0;
	int j = 0;
	int ninput_items = nitems * d_decimation;

	for(int i = 0; i < ninput_items; i += d_nsamples) {
	  // Move block of data to forward FFT buffer
	  memcpy(d_fwdfft->get_inbuf(), &input[i], d_nsamples * sizeof(gr_complex));

	  // zero out any data past d_nsamples to fft_size
	  for(j = d_nsamples; j < d_fftsize; j++)
		d_fwdfft->get_inbuf()[j] = 0;

	  // Run the transform
	  d_fwdfft->execute();	// compute fwd xform

	  // Get the fwd FFT data out
	  gr_complex *a = d_fwdfft->get_outbuf();
	  gr_complex *b = d_xformed_taps;

	  // set up the inv FFT buffer to receive the complex multiplied data
	  gr_complex *c = d_invfft->get_inbuf();

	  // THIS Volk Function just does complex multiplication.  c[i]=a[i]*b[i] in the complex domain.
	  volk_32fc_x2_multiply_32fc_a(c, a, b, d_fftsize);

	  // Run the inverse FFT
	  d_invfft->execute();	// compute inv xform

	  // add in the overlapping tail
	  for(j = 0; j < tailsize(); j++)
		d_invfft->get_outbuf()[j] += d_tail[j];

	  // copy d_nsamples to output buffer and increment for decimation!
	  j = dec_ctr;
	  while(j < d_nsamples) {
		*output++ = d_invfft->get_outbuf()[j];
		j += d_decimation;
	  }
	  dec_ctr = (j - d_nsamples);

	  // stash the tail
	  memcpy(&d_tail[0], d_invfft->get_outbuf() + d_nsamples,
		 tailsize() * sizeof(gr_complex));
	}

	return nitems;
  }

  } //namespace clenabled
} // namespace gr


