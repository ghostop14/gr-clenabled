#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Cross Correlation Test
# Author: ghostop14
# GNU Radio version: 3.8.2.0

from gnuradio import analog
from gnuradio import audio
from gnuradio import blocks
from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import gr
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import uhd
import time
import clenabled
import xcorrelate


class xcorr_test_max_rate_no_ui_opencl(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "Cross Correlation Test")

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 46e6
        self.stage1_decimation = stage1_decimation = int(samp_rate/500e3)
        self.max_search = max_search = 400
        self.lag = lag = 10
        self.gain = gain = 32
        self.delay_0 = delay_0 = 0
        self.corr_frame_size = corr_frame_size = 8192
        self.corr_frame_decim = corr_frame_decim = 200
        self.center_freq = center_freq = 95.7e6

        ##################################################
        # Blocks
        ##################################################
        self.xcorrelate_ExtractDelay_0 = xcorrelate.ExtractDelay(0,self.set_delay_0, False)
        self.uhd_usrp_source_0 = uhd.usrp_source(
            ",".join(("", "num_recv_frames=128")),
            uhd.stream_args(
                cpu_format="fc32",
                args='',
                channels=list(range(0,1)),
            ),
        )
        self.uhd_usrp_source_0.set_center_freq(center_freq, 0)
        self.uhd_usrp_source_0.set_rx_agc(False, 0)
        self.uhd_usrp_source_0.set_gain(gain, 0)
        self.uhd_usrp_source_0.set_antenna('RX2', 0)
        self.uhd_usrp_source_0.set_samp_rate(samp_rate)
        # No synchronization enforced.
        self.uhd_usrp_source_0.set_processor_affinity([0])
        self.rational_resampler_xxx_0_0 = filter.rational_resampler_fff(
                interpolation=48,
                decimation=50,
                taps=None,
                fractional_bw=None)
        self.lfast_low_pass_filter_0_0 = filter.fft_filter_ccc(1, firdes.low_pass(1, samp_rate, 100e3, 10e3, firdes.WIN_HAMMING, 6.76), 1)
        self.lfast_low_pass_filter_0_0.set_processor_affinity([1])
        self.clenabled_XCorrelate_0 = clenabled.clXCorrelate(1,1,0,0,False,2,8192,2,gr.sizeof_float,512,4,True)
        self.blocks_keep_one_in_n_0 = blocks.keep_one_in_n(gr.sizeof_gr_complex*1, stage1_decimation)
        self.blocks_delay_0_0_0 = blocks.delay(gr.sizeof_gr_complex*1, delay_0)
        self.blocks_delay_0 = blocks.delay(gr.sizeof_gr_complex*1, lag)
        self.blocks_complex_to_mag_0_0 = blocks.complex_to_mag(1)
        self.blocks_complex_to_mag_0 = blocks.complex_to_mag(1)
        self.blocks_add_xx_0 = blocks.add_vcc(1)
        self.audio_sink_0 = audio.sink(48000, '', True)
        self.audio_sink_0.set_processor_affinity([2])
        self.analog_wfm_rcv_0 = analog.wfm_rcv(
        	quad_rate=500e3,
        	audio_decimation=10,
        )



        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.clenabled_XCorrelate_0, 'corr'), (self.xcorrelate_ExtractDelay_0, 'corr'))
        self.connect((self.analog_wfm_rcv_0, 0), (self.rational_resampler_xxx_0_0, 0))
        self.connect((self.blocks_add_xx_0, 0), (self.blocks_keep_one_in_n_0, 0))
        self.connect((self.blocks_complex_to_mag_0, 0), (self.clenabled_XCorrelate_0, 0))
        self.connect((self.blocks_complex_to_mag_0_0, 0), (self.clenabled_XCorrelate_0, 1))
        self.connect((self.blocks_delay_0, 0), (self.blocks_complex_to_mag_0_0, 0))
        self.connect((self.blocks_delay_0, 0), (self.blocks_delay_0_0_0, 0))
        self.connect((self.blocks_delay_0_0_0, 0), (self.blocks_add_xx_0, 1))
        self.connect((self.blocks_keep_one_in_n_0, 0), (self.analog_wfm_rcv_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0, 0), (self.blocks_add_xx_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0, 0), (self.blocks_complex_to_mag_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0, 0), (self.blocks_delay_0, 0))
        self.connect((self.rational_resampler_xxx_0_0, 0), (self.audio_sink_0, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.lfast_low_pass_filter_0_0, 0))


    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.set_stage1_decimation(int(self.samp_rate/500e3))
        self.lfast_low_pass_filter_0_0.set_taps(firdes.low_pass(1, self.samp_rate, 100e3, 10e3, firdes.WIN_HAMMING, 6.76))
        self.uhd_usrp_source_0.set_samp_rate(self.samp_rate)

    def get_stage1_decimation(self):
        return self.stage1_decimation

    def set_stage1_decimation(self, stage1_decimation):
        self.stage1_decimation = stage1_decimation
        self.blocks_keep_one_in_n_0.set_n(self.stage1_decimation)

    def get_max_search(self):
        return self.max_search

    def set_max_search(self, max_search):
        self.max_search = max_search

    def get_lag(self):
        return self.lag

    def set_lag(self, lag):
        self.lag = lag
        self.blocks_delay_0.set_dly(self.lag)

    def get_gain(self):
        return self.gain

    def set_gain(self, gain):
        self.gain = gain
        self.uhd_usrp_source_0.set_gain(self.gain, 0)
        self.uhd_usrp_source_0.set_gain(self.gain, 1)

    def get_delay_0(self):
        return self.delay_0

    def set_delay_0(self, delay_0):
        self.delay_0 = delay_0
        self.blocks_delay_0_0_0.set_dly(self.delay_0)

    def get_corr_frame_size(self):
        return self.corr_frame_size

    def set_corr_frame_size(self, corr_frame_size):
        self.corr_frame_size = corr_frame_size

    def get_corr_frame_decim(self):
        return self.corr_frame_decim

    def set_corr_frame_decim(self, corr_frame_decim):
        self.corr_frame_decim = corr_frame_decim

    def get_center_freq(self):
        return self.center_freq

    def set_center_freq(self, center_freq):
        self.center_freq = center_freq
        self.uhd_usrp_source_0.set_center_freq(self.center_freq, 0)
        self.uhd_usrp_source_0.set_center_freq(self.center_freq, 1)





def main(top_block_cls=xcorr_test_max_rate_no_ui_opencl, options=None):
    if gr.enable_realtime_scheduling() != gr.RT_OK:
        print("Error: failed to enable real-time scheduling.")
    tb = top_block_cls()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        sys.exit(0)

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    tb.start()

    try:
        input('Press Enter to quit: ')
    except EOFError:
        pass
    tb.stop()
    tb.wait()


if __name__ == '__main__':
    main()
