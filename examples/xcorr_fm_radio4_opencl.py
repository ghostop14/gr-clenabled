#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Cross Correlate - 4 Inputs
# Author: ghostop14
# GNU Radio version: 3.8.2.0

from distutils.version import StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print("Warning: failed to XInitThreads()")

from PyQt5 import Qt
from gnuradio import eng_notation
from gnuradio import qtgui
from gnuradio.filter import firdes
import sip
from gnuradio import analog
from gnuradio import audio
from gnuradio import blocks
from gnuradio import filter
from gnuradio import gr
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio.fft import logpwrfft
from gnuradio.qtgui import Range, RangeWidget
import clenabled
import correctiq
import osmosdr
import time
import xcorrelate

from gnuradio import qtgui

class xcorr_fm_radio4_opencl(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Cross Correlate - 4 Inputs")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Cross Correlate - 4 Inputs")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "xcorr_fm_radio4_opencl")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(self.settings.value("geometry").toByteArray())
            else:
                self.restoreGeometry(self.settings.value("geometry"))
        except:
            pass

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 2.4e6
        self.stage1_decimation = stage1_decimation = int(samp_rate/600e3)
        self.delay_1 = delay_1 = 0
        self.delay_0 = delay_0 = 0
        self.working_samp_rate = working_samp_rate = 500e3
        self.vol = vol = 1.0
        self.max_search = max_search = 512
        self.lock_output = lock_output = False
        self.gain = gain = 32
        self.fm_width = fm_width = 64000
        self.fm_sample = fm_sample = int(samp_rate/stage1_decimation)
        self.filter_rolloff = filter_rolloff = 10e3
        self.filter_cutoff = filter_cutoff = 90e3
        self.delay_label2 = delay_label2 = delay_1
        self.delay_label1 = delay_label1 = delay_1
        self.delay_label = delay_label = delay_0
        self.delay_2 = delay_2 = 0
        self.corr_frame_size = corr_frame_size = 8192
        self.corr_frame_decim = corr_frame_decim = 6
        self.center_freq = center_freq = 95.7e6
        self.audio_rate = audio_rate = 48000

        ##################################################
        # Blocks
        ##################################################
        self._vol_range = Range(0, 5.0, 0.2, 1.0, 200)
        self._vol_win = RangeWidget(self._vol_range, self.set_vol, 'Volume', "counter_slider", float)
        self.top_grid_layout.addWidget(self._vol_win, 0, 2, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(2, 3):
            self.top_grid_layout.setColumnStretch(c, 1)
        _lock_output_check_box = Qt.QCheckBox('Lock Correlator Lag')
        self._lock_output_choices = {True: True, False: False}
        self._lock_output_choices_inv = dict((v,k) for k,v in self._lock_output_choices.items())
        self._lock_output_callback = lambda i: Qt.QMetaObject.invokeMethod(_lock_output_check_box, "setChecked", Qt.Q_ARG("bool", self._lock_output_choices_inv[i]))
        self._lock_output_callback(self.lock_output)
        _lock_output_check_box.stateChanged.connect(lambda i: self.set_lock_output(self._lock_output_choices[bool(i)]))
        self.top_grid_layout.addWidget(_lock_output_check_box, 3, 3, 1, 1)
        for r in range(3, 4):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(3, 4):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._gain_range = Range(0, 60, 2, 32, 200)
        self._gain_win = RangeWidget(self._gain_range, self.set_gain, 'Gain', "counter_slider", float)
        self.top_grid_layout.addWidget(self._gain_win, 0, 1, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._center_freq_tool_bar = Qt.QToolBar(self)
        self._center_freq_tool_bar.addWidget(Qt.QLabel('Frequency' + ": "))
        self._center_freq_line_edit = Qt.QLineEdit(str(self.center_freq))
        self._center_freq_tool_bar.addWidget(self._center_freq_line_edit)
        self._center_freq_line_edit.returnPressed.connect(
            lambda: self.set_center_freq(eng_notation.str_to_num(str(self._center_freq_line_edit.text()))))
        self.top_grid_layout.addWidget(self._center_freq_tool_bar, 0, 0, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.xcorrelate_ExtractDelay_0_0_0 = xcorrelate.ExtractDelay(0,self.set_delay_2, lock_output)
        self.xcorrelate_ExtractDelay_0_0 = xcorrelate.ExtractDelay(0,self.set_delay_1, lock_output)
        self.xcorrelate_ExtractDelay_0 = xcorrelate.ExtractDelay(0,self.set_delay_0, lock_output)
        self.rtlsdr_source_0_0_0_0 = osmosdr.source(
            args="numchan=" + str(1) + " " + "rtl=3"
        )
        self.rtlsdr_source_0_0_0_0.set_time_unknown_pps(osmosdr.time_spec_t())
        self.rtlsdr_source_0_0_0_0.set_sample_rate(samp_rate)
        self.rtlsdr_source_0_0_0_0.set_center_freq(center_freq, 0)
        self.rtlsdr_source_0_0_0_0.set_freq_corr(0, 0)
        self.rtlsdr_source_0_0_0_0.set_dc_offset_mode(0, 0)
        self.rtlsdr_source_0_0_0_0.set_iq_balance_mode(0, 0)
        self.rtlsdr_source_0_0_0_0.set_gain_mode(False, 0)
        self.rtlsdr_source_0_0_0_0.set_gain(gain, 0)
        self.rtlsdr_source_0_0_0_0.set_if_gain(gain, 0)
        self.rtlsdr_source_0_0_0_0.set_bb_gain(gain, 0)
        self.rtlsdr_source_0_0_0_0.set_antenna('', 0)
        self.rtlsdr_source_0_0_0_0.set_bandwidth(0, 0)
        self.rtlsdr_source_0_0_0 = osmosdr.source(
            args="numchan=" + str(1) + " " + "rtl=2"
        )
        self.rtlsdr_source_0_0_0.set_time_unknown_pps(osmosdr.time_spec_t())
        self.rtlsdr_source_0_0_0.set_sample_rate(samp_rate)
        self.rtlsdr_source_0_0_0.set_center_freq(center_freq, 0)
        self.rtlsdr_source_0_0_0.set_freq_corr(0, 0)
        self.rtlsdr_source_0_0_0.set_dc_offset_mode(0, 0)
        self.rtlsdr_source_0_0_0.set_iq_balance_mode(0, 0)
        self.rtlsdr_source_0_0_0.set_gain_mode(False, 0)
        self.rtlsdr_source_0_0_0.set_gain(gain, 0)
        self.rtlsdr_source_0_0_0.set_if_gain(gain, 0)
        self.rtlsdr_source_0_0_0.set_bb_gain(gain, 0)
        self.rtlsdr_source_0_0_0.set_antenna('', 0)
        self.rtlsdr_source_0_0_0.set_bandwidth(0, 0)
        self.rtlsdr_source_0_0 = osmosdr.source(
            args="numchan=" + str(1) + " " + "rtl=1"
        )
        self.rtlsdr_source_0_0.set_time_unknown_pps(osmosdr.time_spec_t())
        self.rtlsdr_source_0_0.set_sample_rate(samp_rate)
        self.rtlsdr_source_0_0.set_center_freq(center_freq, 0)
        self.rtlsdr_source_0_0.set_freq_corr(0, 0)
        self.rtlsdr_source_0_0.set_dc_offset_mode(0, 0)
        self.rtlsdr_source_0_0.set_iq_balance_mode(0, 0)
        self.rtlsdr_source_0_0.set_gain_mode(False, 0)
        self.rtlsdr_source_0_0.set_gain(gain, 0)
        self.rtlsdr_source_0_0.set_if_gain(gain, 0)
        self.rtlsdr_source_0_0.set_bb_gain(gain, 0)
        self.rtlsdr_source_0_0.set_antenna('', 0)
        self.rtlsdr_source_0_0.set_bandwidth(0, 0)
        self.rtlsdr_source_0 = osmosdr.source(
            args="numchan=" + str(1) + " " + "rtl=0"
        )
        self.rtlsdr_source_0.set_time_unknown_pps(osmosdr.time_spec_t())
        self.rtlsdr_source_0.set_sample_rate(samp_rate)
        self.rtlsdr_source_0.set_center_freq(center_freq, 0)
        self.rtlsdr_source_0.set_freq_corr(0, 0)
        self.rtlsdr_source_0.set_dc_offset_mode(0, 0)
        self.rtlsdr_source_0.set_iq_balance_mode(0, 0)
        self.rtlsdr_source_0.set_gain_mode(False, 0)
        self.rtlsdr_source_0.set_gain(gain, 0)
        self.rtlsdr_source_0.set_if_gain(gain, 0)
        self.rtlsdr_source_0.set_bb_gain(gain, 0)
        self.rtlsdr_source_0.set_antenna('', 0)
        self.rtlsdr_source_0.set_bandwidth(0, 0)
        self.rational_resampler_xxx_0_0 = filter.rational_resampler_fff(
                interpolation=int(audio_rate/1e3),
                decimation=int(fm_sample/10000),
                taps=None,
                fractional_bw=None)
        self.qtgui_waterfall_sink_x_0 = qtgui.waterfall_sink_c(
            1024, #size
            firdes.WIN_BLACKMAN_hARRIS, #wintype
            center_freq, #fc
            samp_rate, #bw
            "", #name
            1 #number of inputs
        )
        self.qtgui_waterfall_sink_x_0.set_update_time(0.10)
        self.qtgui_waterfall_sink_x_0.enable_grid(False)
        self.qtgui_waterfall_sink_x_0.enable_axis_labels(True)



        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_waterfall_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_waterfall_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_waterfall_sink_x_0.set_color_map(i, colors[i])
            self.qtgui_waterfall_sink_x_0.set_line_alpha(i, alphas[i])

        self.qtgui_waterfall_sink_x_0.set_intensity_range(-60, -10)

        self._qtgui_waterfall_sink_x_0_win = sip.wrapinstance(self.qtgui_waterfall_sink_x_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_waterfall_sink_x_0_win, 4, 0, 1, 4)
        for r in range(4, 5):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 4):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_number_sink_0_0_0 = qtgui.number_sink(
            gr.sizeof_float,
            0,
            qtgui.NUM_GRAPH_HORIZ,
            1
        )
        self.qtgui_number_sink_0_0_0.set_update_time(0.10)
        self.qtgui_number_sink_0_0_0.set_title("Improvement Over Reference")

        labels = ["", '', '', '', '',
            '', '', '', '', '']
        units = ['', '', '', '', '',
            '', '', '', '', '']
        colors = [("blue", "red"), ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black"),
            ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black")]
        factor = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]

        for i in range(1):
            self.qtgui_number_sink_0_0_0.set_min(i, -10)
            self.qtgui_number_sink_0_0_0.set_max(i, 10)
            self.qtgui_number_sink_0_0_0.set_color(i, colors[i][0], colors[i][1])
            if len(labels[i]) == 0:
                self.qtgui_number_sink_0_0_0.set_label(i, "Data {0}".format(i))
            else:
                self.qtgui_number_sink_0_0_0.set_label(i, labels[i])
            self.qtgui_number_sink_0_0_0.set_unit(i, units[i])
            self.qtgui_number_sink_0_0_0.set_factor(i, factor[i])

        self.qtgui_number_sink_0_0_0.enable_autoscale(False)
        self._qtgui_number_sink_0_0_0_win = sip.wrapinstance(self.qtgui_number_sink_0_0_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_number_sink_0_0_0_win, 6, 0, 1, 3)
        for r in range(6, 7):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 3):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_number_sink_0 = qtgui.number_sink(
            gr.sizeof_float,
            0,
            qtgui.NUM_GRAPH_HORIZ,
            2
        )
        self.qtgui_number_sink_0.set_update_time(0.10)
        self.qtgui_number_sink_0.set_title('Max Power')

        labels = ['Ref Signal', 'Correlated', '', '', '',
            '', '', '', '', '']
        units = ['', '', '', '', '',
            '', '', '', '', '']
        colors = [("blue", "red"), ("blue", "red"), ("black", "black"), ("black", "black"), ("black", "black"),
            ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black")]
        factor = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]

        for i in range(2):
            self.qtgui_number_sink_0.set_min(i, -60)
            self.qtgui_number_sink_0.set_max(i, -10)
            self.qtgui_number_sink_0.set_color(i, colors[i][0], colors[i][1])
            if len(labels[i]) == 0:
                self.qtgui_number_sink_0.set_label(i, "Data {0}".format(i))
            else:
                self.qtgui_number_sink_0.set_label(i, labels[i])
            self.qtgui_number_sink_0.set_unit(i, units[i])
            self.qtgui_number_sink_0.set_factor(i, factor[i])

        self.qtgui_number_sink_0.enable_autoscale(False)
        self._qtgui_number_sink_0_win = sip.wrapinstance(self.qtgui_number_sink_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_number_sink_0_win, 5, 0, 1, 3)
        for r in range(5, 6):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 3):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_freq_sink_x_0_0 = qtgui.freq_sink_c(
            1024, #size
            firdes.WIN_BLACKMAN_hARRIS, #wintype
            center_freq, #fc
            samp_rate, #bw
            "Correlated", #name
            1
        )
        self.qtgui_freq_sink_x_0_0.set_update_time(0.10)
        self.qtgui_freq_sink_x_0_0.set_y_axis(-80, -10)
        self.qtgui_freq_sink_x_0_0.set_y_label('Relative Gain', 'dB')
        self.qtgui_freq_sink_x_0_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_0_0.enable_autoscale(False)
        self.qtgui_freq_sink_x_0_0.enable_grid(False)
        self.qtgui_freq_sink_x_0_0.set_fft_average(1.0)
        self.qtgui_freq_sink_x_0_0.enable_axis_labels(True)
        self.qtgui_freq_sink_x_0_0.enable_control_panel(False)



        labels = ['Reference', 'Delayed', '', '', '',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
            "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_freq_sink_x_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_freq_sink_x_0_0.set_line_label(i, labels[i])
            self.qtgui_freq_sink_x_0_0.set_line_width(i, widths[i])
            self.qtgui_freq_sink_x_0_0.set_line_color(i, colors[i])
            self.qtgui_freq_sink_x_0_0.set_line_alpha(i, alphas[i])

        self._qtgui_freq_sink_x_0_0_win = sip.wrapinstance(self.qtgui_freq_sink_x_0_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_freq_sink_x_0_0_win, 1, 2, 2, 2)
        for r in range(1, 3):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(2, 4):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_freq_sink_x_0 = qtgui.freq_sink_c(
            1024, #size
            firdes.WIN_BLACKMAN_hARRIS, #wintype
            center_freq, #fc
            samp_rate, #bw
            "Uncorrelated", #name
            5
        )
        self.qtgui_freq_sink_x_0.set_update_time(0.10)
        self.qtgui_freq_sink_x_0.set_y_axis(-80, -10)
        self.qtgui_freq_sink_x_0.set_y_label('Relative Gain', 'dB')
        self.qtgui_freq_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_0.enable_autoscale(False)
        self.qtgui_freq_sink_x_0.enable_grid(False)
        self.qtgui_freq_sink_x_0.set_fft_average(1.0)
        self.qtgui_freq_sink_x_0.enable_axis_labels(True)
        self.qtgui_freq_sink_x_0.enable_control_panel(False)



        labels = ['Reference', 'Signal1', 'Signal2', 'Signal3', 'Correlated',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
            "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(5):
            if len(labels[i]) == 0:
                self.qtgui_freq_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_freq_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_freq_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_freq_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_freq_sink_x_0.set_line_alpha(i, alphas[i])

        self._qtgui_freq_sink_x_0_win = sip.wrapinstance(self.qtgui_freq_sink_x_0.pyqwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._qtgui_freq_sink_x_0_win, 1, 0, 2, 2)
        for r in range(1, 3):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.logpwrfft_x_0_0 = logpwrfft.logpwrfft_c(
            sample_rate=samp_rate,
            fft_size=1024,
            ref_scale=2,
            frame_rate=30,
            avg_alpha=1.0,
            average=False)
        self.logpwrfft_x_0 = logpwrfft.logpwrfft_c(
            sample_rate=samp_rate,
            fft_size=1024,
            ref_scale=2,
            frame_rate=30,
            avg_alpha=1.0,
            average=False)
        self.lfast_low_pass_filter_0_0_0_0_0 = filter.fft_filter_ccc(1, firdes.low_pass(1, samp_rate, filter_cutoff, filter_rolloff, firdes.WIN_HAMMING, 6.76), 1)
        self.lfast_low_pass_filter_0_0_0_0 = filter.fft_filter_ccc(1, firdes.low_pass(1, samp_rate, filter_cutoff, filter_rolloff, firdes.WIN_HAMMING, 6.76), 1)
        self.lfast_low_pass_filter_0_0_0 = filter.fft_filter_ccc(1, firdes.low_pass(1, samp_rate, filter_cutoff, filter_rolloff, firdes.WIN_HAMMING, 6.76), 1)
        self.lfast_low_pass_filter_0_0 = filter.fft_filter_ccc(1, firdes.low_pass(1, samp_rate, filter_cutoff, filter_rolloff, firdes.WIN_HAMMING, 6.76), 1)
        self._delay_label2_tool_bar = Qt.QToolBar(self)

        if None:
            self._delay_label2_formatter = None
        else:
            self._delay_label2_formatter = lambda x: str(x)

        self._delay_label2_tool_bar.addWidget(Qt.QLabel('Correcting Delay [0,3]' + ": "))
        self._delay_label2_label = Qt.QLabel(str(self._delay_label2_formatter(self.delay_label2)))
        self._delay_label2_tool_bar.addWidget(self._delay_label2_label)
        self.top_grid_layout.addWidget(self._delay_label2_tool_bar, 3, 2, 1, 1)
        for r in range(3, 4):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(2, 3):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._delay_label1_tool_bar = Qt.QToolBar(self)

        if None:
            self._delay_label1_formatter = None
        else:
            self._delay_label1_formatter = lambda x: str(x)

        self._delay_label1_tool_bar.addWidget(Qt.QLabel('Correcting Delay [0,2]' + ": "))
        self._delay_label1_label = Qt.QLabel(str(self._delay_label1_formatter(self.delay_label1)))
        self._delay_label1_tool_bar.addWidget(self._delay_label1_label)
        self.top_grid_layout.addWidget(self._delay_label1_tool_bar, 3, 1, 1, 1)
        for r in range(3, 4):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._delay_label_tool_bar = Qt.QToolBar(self)

        if None:
            self._delay_label_formatter = None
        else:
            self._delay_label_formatter = lambda x: str(x)

        self._delay_label_tool_bar.addWidget(Qt.QLabel('Correcting Delay [0,1]' + ": "))
        self._delay_label_label = Qt.QLabel(str(self._delay_label_formatter(self.delay_label)))
        self._delay_label_tool_bar.addWidget(self._delay_label_label)
        self.top_grid_layout.addWidget(self._delay_label_tool_bar, 3, 0, 1, 1)
        for r in range(3, 4):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.correctiq_correctiq_auto_0_0_0_0 = correctiq.correctiq_auto(samp_rate, center_freq, gain, 2)
        self.correctiq_correctiq_auto_0_0_0 = correctiq.correctiq_auto(samp_rate, center_freq, gain, 2)
        self.correctiq_correctiq_auto_0_0 = correctiq.correctiq_auto(samp_rate, center_freq, gain, 2)
        self.correctiq_correctiq_auto_0 = correctiq.correctiq_auto(samp_rate, center_freq, gain, 2)
        self.clenabled_XCorrelate_0 = clenabled.clXCorrelate(1,1,0,0,False,4,corr_frame_size,2,gr.sizeof_float,max_search,corr_frame_decim,True)
        self.blocks_sub_xx_0 = blocks.sub_ff(1)
        self.blocks_multiply_const_vxx_0_0 = blocks.multiply_const_ff(-1)
        self.blocks_multiply_const_vxx_0 = blocks.multiply_const_ff(vol)
        self.blocks_max_xx_0_0 = blocks.max_ff(1024, 1)
        self.blocks_max_xx_0 = blocks.max_ff(1024, 1)
        self.blocks_keep_one_in_n_0 = blocks.keep_one_in_n(gr.sizeof_gr_complex*1, stage1_decimation)
        self.blocks_delay_0_0_0_0_0 = blocks.delay(gr.sizeof_gr_complex*1, delay_2)
        self.blocks_delay_0_0_0_0 = blocks.delay(gr.sizeof_gr_complex*1, delay_1)
        self.blocks_delay_0_0_0 = blocks.delay(gr.sizeof_gr_complex*1, delay_0)
        self.blocks_complex_to_mag_0_0_0_0 = blocks.complex_to_mag(1)
        self.blocks_complex_to_mag_0_0_0 = blocks.complex_to_mag(1)
        self.blocks_complex_to_mag_0_0 = blocks.complex_to_mag(1)
        self.blocks_complex_to_mag_0 = blocks.complex_to_mag(1)
        self.blocks_add_xx_0 = blocks.add_vcc(1)
        self.audio_sink_0 = audio.sink(48000, '', True)
        self.analog_wfm_rcv_0 = analog.wfm_rcv(
        	quad_rate=int(samp_rate/stage1_decimation),
        	audio_decimation=10,
        )



        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.clenabled_XCorrelate_0, 'corr'), (self.xcorrelate_ExtractDelay_0, 'corr'))
        self.msg_connect((self.clenabled_XCorrelate_0, 'corr'), (self.xcorrelate_ExtractDelay_0_0, 'corr'))
        self.msg_connect((self.clenabled_XCorrelate_0, 'corr'), (self.xcorrelate_ExtractDelay_0_0_0, 'corr'))
        self.connect((self.analog_wfm_rcv_0, 0), (self.rational_resampler_xxx_0_0, 0))
        self.connect((self.blocks_add_xx_0, 0), (self.blocks_keep_one_in_n_0, 0))
        self.connect((self.blocks_add_xx_0, 0), (self.logpwrfft_x_0_0, 0))
        self.connect((self.blocks_add_xx_0, 0), (self.qtgui_freq_sink_x_0, 4))
        self.connect((self.blocks_add_xx_0, 0), (self.qtgui_freq_sink_x_0_0, 0))
        self.connect((self.blocks_add_xx_0, 0), (self.qtgui_waterfall_sink_x_0, 0))
        self.connect((self.blocks_complex_to_mag_0, 0), (self.clenabled_XCorrelate_0, 0))
        self.connect((self.blocks_complex_to_mag_0_0, 0), (self.clenabled_XCorrelate_0, 1))
        self.connect((self.blocks_complex_to_mag_0_0_0, 0), (self.clenabled_XCorrelate_0, 2))
        self.connect((self.blocks_complex_to_mag_0_0_0_0, 0), (self.clenabled_XCorrelate_0, 3))
        self.connect((self.blocks_delay_0_0_0, 0), (self.blocks_add_xx_0, 1))
        self.connect((self.blocks_delay_0_0_0_0, 0), (self.blocks_add_xx_0, 2))
        self.connect((self.blocks_delay_0_0_0_0_0, 0), (self.blocks_add_xx_0, 3))
        self.connect((self.blocks_keep_one_in_n_0, 0), (self.analog_wfm_rcv_0, 0))
        self.connect((self.blocks_max_xx_0, 0), (self.blocks_sub_xx_0, 0))
        self.connect((self.blocks_max_xx_0, 0), (self.qtgui_number_sink_0, 0))
        self.connect((self.blocks_max_xx_0_0, 0), (self.blocks_sub_xx_0, 1))
        self.connect((self.blocks_max_xx_0_0, 0), (self.qtgui_number_sink_0, 1))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.audio_sink_0, 0))
        self.connect((self.blocks_multiply_const_vxx_0_0, 0), (self.qtgui_number_sink_0_0_0, 0))
        self.connect((self.blocks_sub_xx_0, 0), (self.blocks_multiply_const_vxx_0_0, 0))
        self.connect((self.correctiq_correctiq_auto_0, 0), (self.lfast_low_pass_filter_0_0_0_0_0, 0))
        self.connect((self.correctiq_correctiq_auto_0_0, 0), (self.lfast_low_pass_filter_0_0_0_0, 0))
        self.connect((self.correctiq_correctiq_auto_0_0_0, 0), (self.lfast_low_pass_filter_0_0_0, 0))
        self.connect((self.correctiq_correctiq_auto_0_0_0_0, 0), (self.lfast_low_pass_filter_0_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0, 0), (self.blocks_add_xx_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0, 0), (self.blocks_complex_to_mag_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0, 0), (self.logpwrfft_x_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0, 0), (self.qtgui_freq_sink_x_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0_0, 0), (self.blocks_complex_to_mag_0_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0_0, 0), (self.blocks_delay_0_0_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0_0, 0), (self.qtgui_freq_sink_x_0, 1))
        self.connect((self.lfast_low_pass_filter_0_0_0_0, 0), (self.blocks_complex_to_mag_0_0_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0_0_0, 0), (self.blocks_delay_0_0_0_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0_0_0, 0), (self.qtgui_freq_sink_x_0, 2))
        self.connect((self.lfast_low_pass_filter_0_0_0_0_0, 0), (self.blocks_complex_to_mag_0_0_0_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0_0_0_0, 0), (self.blocks_delay_0_0_0_0_0, 0))
        self.connect((self.lfast_low_pass_filter_0_0_0_0_0, 0), (self.qtgui_freq_sink_x_0, 3))
        self.connect((self.logpwrfft_x_0, 0), (self.blocks_max_xx_0, 0))
        self.connect((self.logpwrfft_x_0_0, 0), (self.blocks_max_xx_0_0, 0))
        self.connect((self.rational_resampler_xxx_0_0, 0), (self.blocks_multiply_const_vxx_0, 0))
        self.connect((self.rtlsdr_source_0, 0), (self.correctiq_correctiq_auto_0_0_0_0, 0))
        self.connect((self.rtlsdr_source_0_0, 0), (self.correctiq_correctiq_auto_0_0_0, 0))
        self.connect((self.rtlsdr_source_0_0_0, 0), (self.correctiq_correctiq_auto_0_0, 0))
        self.connect((self.rtlsdr_source_0_0_0_0, 0), (self.correctiq_correctiq_auto_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "xcorr_fm_radio4_opencl")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.set_fm_sample(int(self.samp_rate/self.stage1_decimation))
        self.set_stage1_decimation(int(self.samp_rate/600e3))
        self.lfast_low_pass_filter_0_0.set_taps(firdes.low_pass(1, self.samp_rate, self.filter_cutoff, self.filter_rolloff, firdes.WIN_HAMMING, 6.76))
        self.lfast_low_pass_filter_0_0_0.set_taps(firdes.low_pass(1, self.samp_rate, self.filter_cutoff, self.filter_rolloff, firdes.WIN_HAMMING, 6.76))
        self.lfast_low_pass_filter_0_0_0_0.set_taps(firdes.low_pass(1, self.samp_rate, self.filter_cutoff, self.filter_rolloff, firdes.WIN_HAMMING, 6.76))
        self.lfast_low_pass_filter_0_0_0_0_0.set_taps(firdes.low_pass(1, self.samp_rate, self.filter_cutoff, self.filter_rolloff, firdes.WIN_HAMMING, 6.76))
        self.logpwrfft_x_0.set_sample_rate(self.samp_rate)
        self.logpwrfft_x_0_0.set_sample_rate(self.samp_rate)
        self.qtgui_freq_sink_x_0.set_frequency_range(self.center_freq, self.samp_rate)
        self.qtgui_freq_sink_x_0_0.set_frequency_range(self.center_freq, self.samp_rate)
        self.qtgui_waterfall_sink_x_0.set_frequency_range(self.center_freq, self.samp_rate)
        self.rtlsdr_source_0.set_sample_rate(self.samp_rate)
        self.rtlsdr_source_0_0.set_sample_rate(self.samp_rate)
        self.rtlsdr_source_0_0_0.set_sample_rate(self.samp_rate)
        self.rtlsdr_source_0_0_0_0.set_sample_rate(self.samp_rate)

    def get_stage1_decimation(self):
        return self.stage1_decimation

    def set_stage1_decimation(self, stage1_decimation):
        self.stage1_decimation = stage1_decimation
        self.set_fm_sample(int(self.samp_rate/self.stage1_decimation))
        self.blocks_keep_one_in_n_0.set_n(self.stage1_decimation)

    def get_delay_1(self):
        return self.delay_1

    def set_delay_1(self, delay_1):
        self.delay_1 = delay_1
        self.set_delay_label1(self._delay_label1_formatter(self.delay_1))
        self.set_delay_label2(self._delay_label2_formatter(self.delay_1))
        self.blocks_delay_0_0_0_0.set_dly(self.delay_1)

    def get_delay_0(self):
        return self.delay_0

    def set_delay_0(self, delay_0):
        self.delay_0 = delay_0
        self.set_delay_label(self._delay_label_formatter(self.delay_0))
        self.blocks_delay_0_0_0.set_dly(self.delay_0)

    def get_working_samp_rate(self):
        return self.working_samp_rate

    def set_working_samp_rate(self, working_samp_rate):
        self.working_samp_rate = working_samp_rate

    def get_vol(self):
        return self.vol

    def set_vol(self, vol):
        self.vol = vol
        self.blocks_multiply_const_vxx_0.set_k(self.vol)

    def get_max_search(self):
        return self.max_search

    def set_max_search(self, max_search):
        self.max_search = max_search

    def get_lock_output(self):
        return self.lock_output

    def set_lock_output(self, lock_output):
        self.lock_output = lock_output
        self._lock_output_callback(self.lock_output)
        self.xcorrelate_ExtractDelay_0.set_lock(self.lock_output)
        self.xcorrelate_ExtractDelay_0_0.set_lock(self.lock_output)
        self.xcorrelate_ExtractDelay_0_0_0.set_lock(self.lock_output)

    def get_gain(self):
        return self.gain

    def set_gain(self, gain):
        self.gain = gain
        self.correctiq_correctiq_auto_0.set_gain(self.gain)
        self.correctiq_correctiq_auto_0_0.set_gain(self.gain)
        self.correctiq_correctiq_auto_0_0_0.set_gain(self.gain)
        self.correctiq_correctiq_auto_0_0_0_0.set_gain(self.gain)
        self.rtlsdr_source_0.set_gain(self.gain, 0)
        self.rtlsdr_source_0.set_if_gain(self.gain, 0)
        self.rtlsdr_source_0.set_bb_gain(self.gain, 0)
        self.rtlsdr_source_0_0.set_gain(self.gain, 0)
        self.rtlsdr_source_0_0.set_if_gain(self.gain, 0)
        self.rtlsdr_source_0_0.set_bb_gain(self.gain, 0)
        self.rtlsdr_source_0_0_0.set_gain(self.gain, 0)
        self.rtlsdr_source_0_0_0.set_if_gain(self.gain, 0)
        self.rtlsdr_source_0_0_0.set_bb_gain(self.gain, 0)
        self.rtlsdr_source_0_0_0_0.set_gain(self.gain, 0)
        self.rtlsdr_source_0_0_0_0.set_if_gain(self.gain, 0)
        self.rtlsdr_source_0_0_0_0.set_bb_gain(self.gain, 0)

    def get_fm_width(self):
        return self.fm_width

    def set_fm_width(self, fm_width):
        self.fm_width = fm_width

    def get_fm_sample(self):
        return self.fm_sample

    def set_fm_sample(self, fm_sample):
        self.fm_sample = fm_sample

    def get_filter_rolloff(self):
        return self.filter_rolloff

    def set_filter_rolloff(self, filter_rolloff):
        self.filter_rolloff = filter_rolloff
        self.lfast_low_pass_filter_0_0.set_taps(firdes.low_pass(1, self.samp_rate, self.filter_cutoff, self.filter_rolloff, firdes.WIN_HAMMING, 6.76))
        self.lfast_low_pass_filter_0_0_0.set_taps(firdes.low_pass(1, self.samp_rate, self.filter_cutoff, self.filter_rolloff, firdes.WIN_HAMMING, 6.76))
        self.lfast_low_pass_filter_0_0_0_0.set_taps(firdes.low_pass(1, self.samp_rate, self.filter_cutoff, self.filter_rolloff, firdes.WIN_HAMMING, 6.76))
        self.lfast_low_pass_filter_0_0_0_0_0.set_taps(firdes.low_pass(1, self.samp_rate, self.filter_cutoff, self.filter_rolloff, firdes.WIN_HAMMING, 6.76))

    def get_filter_cutoff(self):
        return self.filter_cutoff

    def set_filter_cutoff(self, filter_cutoff):
        self.filter_cutoff = filter_cutoff
        self.lfast_low_pass_filter_0_0.set_taps(firdes.low_pass(1, self.samp_rate, self.filter_cutoff, self.filter_rolloff, firdes.WIN_HAMMING, 6.76))
        self.lfast_low_pass_filter_0_0_0.set_taps(firdes.low_pass(1, self.samp_rate, self.filter_cutoff, self.filter_rolloff, firdes.WIN_HAMMING, 6.76))
        self.lfast_low_pass_filter_0_0_0_0.set_taps(firdes.low_pass(1, self.samp_rate, self.filter_cutoff, self.filter_rolloff, firdes.WIN_HAMMING, 6.76))
        self.lfast_low_pass_filter_0_0_0_0_0.set_taps(firdes.low_pass(1, self.samp_rate, self.filter_cutoff, self.filter_rolloff, firdes.WIN_HAMMING, 6.76))

    def get_delay_label2(self):
        return self.delay_label2

    def set_delay_label2(self, delay_label2):
        self.delay_label2 = delay_label2
        Qt.QMetaObject.invokeMethod(self._delay_label2_label, "setText", Qt.Q_ARG("QString", self.delay_label2))

    def get_delay_label1(self):
        return self.delay_label1

    def set_delay_label1(self, delay_label1):
        self.delay_label1 = delay_label1
        Qt.QMetaObject.invokeMethod(self._delay_label1_label, "setText", Qt.Q_ARG("QString", self.delay_label1))

    def get_delay_label(self):
        return self.delay_label

    def set_delay_label(self, delay_label):
        self.delay_label = delay_label
        Qt.QMetaObject.invokeMethod(self._delay_label_label, "setText", Qt.Q_ARG("QString", self.delay_label))

    def get_delay_2(self):
        return self.delay_2

    def set_delay_2(self, delay_2):
        self.delay_2 = delay_2
        self.blocks_delay_0_0_0_0_0.set_dly(self.delay_2)

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
        Qt.QMetaObject.invokeMethod(self._center_freq_line_edit, "setText", Qt.Q_ARG("QString", eng_notation.num_to_str(self.center_freq)))
        self.correctiq_correctiq_auto_0.set_freq(self.center_freq)
        self.correctiq_correctiq_auto_0_0.set_freq(self.center_freq)
        self.correctiq_correctiq_auto_0_0_0.set_freq(self.center_freq)
        self.correctiq_correctiq_auto_0_0_0_0.set_freq(self.center_freq)
        self.qtgui_freq_sink_x_0.set_frequency_range(self.center_freq, self.samp_rate)
        self.qtgui_freq_sink_x_0_0.set_frequency_range(self.center_freq, self.samp_rate)
        self.qtgui_waterfall_sink_x_0.set_frequency_range(self.center_freq, self.samp_rate)
        self.rtlsdr_source_0.set_center_freq(self.center_freq, 0)
        self.rtlsdr_source_0_0.set_center_freq(self.center_freq, 0)
        self.rtlsdr_source_0_0_0.set_center_freq(self.center_freq, 0)
        self.rtlsdr_source_0_0_0_0.set_center_freq(self.center_freq, 0)

    def get_audio_rate(self):
        return self.audio_rate

    def set_audio_rate(self, audio_rate):
        self.audio_rate = audio_rate





def main(top_block_cls=xcorr_fm_radio4_opencl, options=None):

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()

    tb.show()

    def sig_handler(sig=None, frame=None):
        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    def quitting():
        tb.stop()
        tb.wait()

    qapp.aboutToQuit.connect(quitting)
    qapp.exec_()

if __name__ == '__main__':
    main()
