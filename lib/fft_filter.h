/*
 * fft_filter.h
 *
 *  Created on: Feb 10, 2017
 *      Author: root
 */

#ifndef LIB_FFT_FILTER_H_
#define LIB_FFT_FILTER_H_

#include "fft.h"

namespace gr {
	namespace clenabled {

      /*!
       * \brief Fast FFT filter with gr_complex input, gr_complex output and float taps
       * \ingroup filter_blk
       *
       * \details
       * This block performs fast convolution using the
       * overlap-and-save algorithm. The filtering is performand in
       * the frequency domain instead of the time domain (see
       * gr::filter::kernel::fir_filter_ccf). For an input signal x
       * and filter coefficients (taps) t, we compute y as:
       *
       * \code
       *    y = ifft(fft(x)*fft(t))
       * \endcode
       *
       * This kernel computes the FFT of the taps when they are set to
       * only perform this operation once. The FFT of the input signal
       * x is done every time.
       *
       * Because this is designed as a very low-level kernel
       * operation, it is designed for speed and avoids certain checks
       * in the filter() function itself. The filter function expects
       * that the input signal is a multiple of d_nsamples in the
       * class that's computed internally to be as fast as
       * possible. The function set_taps will return the value of
       * nsamples that can be used externally to check this
       * boundary. Notice that all implementations of the fft_filter
       * GNU Radio blocks (e.g., gr::filter::fft_filter_ccf) use this
       * value of nsamples to compute the value to call
       * gr::block::set_output_multiple that ensures the scheduler
       * always passes this block the right number of samples.
       */
      class CLENABLED_API fft_filter_ccf
      {
      public:
		int                      d_decimation;
		fft_complex        *d_fwdfft;	    // forward "plan"
		fft_complex        *d_invfft;          // inverse "plan"
		int                      d_nthreads;        // number of FFTW threads to use
		gr_complex              *d_xformed_taps;    // Fourier xformed taps

		virtual void compute_sizes(int ntaps);
		int tailsize() const { return d_ntaps - 1; }

		std::vector<gr_complex>  d_tail;	    // state carried between blocks for overlap-add
		std::vector<float>       d_taps;            // stores time domain taps
		int			 d_ntaps;
		int			 d_nsamples;
		int			 d_fftsize;         // fftsize = ntaps + nsamples - 1
		/*!
		 * \brief Construct an FFT filter for complex vectors with the given taps and decimation rate.
		 *
		 * This is the basic implementation for performing FFT filter for fast convolution
		 * in other blocks (e.g., gr::filter::fft_filter_ccf).
		 *
		 * \param decimation The decimation rate of the filter (int)
		 * \param taps       The filter taps (float)
		 * \param nthreads   The number of threads for the FFT to use (int)
		 */
		fft_filter_ccf(int decimation,
				   const std::vector<float> &taps,
				   int nthreads=1);

		virtual ~fft_filter_ccf();

		/*!
		 * \brief Set new taps for the filter.
		 *
		 * Sets new taps and resets the class properties to handle different sizes
		 * \param taps       The filter taps (complex)
		 */
		virtual int set_taps(const std::vector<float> &taps);

		/*!
		 * \brief Set number of threads to use.
		 */
		virtual void set_nthreads(int n);

		/*!
		 * \brief Returns the taps.
		 */
		std::vector<float> taps() const;

		/*!
		 * \brief Returns the number of taps in the filter.
		 */
			unsigned int ntaps() const;

		/*!
		 * \brief Returns the actual size of the filter.
			 *
			 * \details This value could be equal to ntaps, but we ofter
			 * build a longer filter to allow us to calculate a more
			 * efficient FFT. This value is the actual size of the filters
			 * used in the calculation of the overlap-and-save operation.
		 */
			unsigned int filtersize() const;

		/*!
		 * \brief Get number of threads being used.
		 */
		int nthreads() const;

		/*!
		 * \brief Perform the filter operation
		 *
		 * \param nitems  The number of items to produce
		 * \param input   The input vector to be filtered
		 * \param output  The result of the filter operation
		 */
		virtual int filter(int nitems, const gr_complex *input, gr_complex *output);
      };
	}
}

#endif /* LIB_FFT_FILTER_H_ */
