#!/usr/bin/python2.7
# ##!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2017 <+YOU OR YOUR COMPANY+>.
# 
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import clenabled
import time

primes = (2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53,
          59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131,
          137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223,
          227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311)

primes_transformed = ((4377 + 4516j),
                      (-1706.1268310546875 + 1638.4256591796875j),
                      (-915.2083740234375 + 660.69427490234375j),
                      (-660.370361328125 + 381.59600830078125j),
                      (-499.96044921875 + 238.41630554199219j),
                      (-462.26748657226562 + 152.88948059082031j),
                      (-377.98440551757812 + 77.5928955078125j),
                      (-346.85821533203125 + 47.152004241943359j),
                      (-295 + 20j),
                      (-286.33609008789062 - 22.257017135620117j),
                      (-271.52999877929688 - 33.081821441650391j),
                      (-224.6358642578125 - 67.019538879394531j),
                      (-244.24473571777344 - 91.524826049804688j),
                      (-203.09068298339844 - 108.54627227783203j),
                      (-198.45195007324219 - 115.90768432617188j),
                      (-182.97744750976562 - 128.12318420410156j),
                      (-167 - 180j),
                      (-130.33688354492188 - 173.83778381347656j),
                      (-141.19784545898438 - 190.28807067871094j),
                      (-111.09677124023438 - 214.48896789550781j),
                      (-70.039543151855469 - 242.41630554199219j),
                      (-68.960540771484375 - 228.30015563964844j),
                      (-53.049201965332031 - 291.47097778320312j),
                      (-28.695289611816406 - 317.64553833007812j),
                      (57 - 300j),
                      (45.301143646240234 - 335.69509887695312j),
                      (91.936195373535156 - 373.32437133789062j),
                      (172.09465026855469 - 439.275146484375j),
                      (242.24473571777344 - 504.47515869140625j),
                      (387.81732177734375 - 666.6788330078125j),
                      (689.48553466796875 - 918.2142333984375j),
                      (1646.539306640625 - 1694.1956787109375j))

class qa_clFFT (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()
        self.fft_size = 32

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        src_data = tuple([complex(primes[2 * i], primes[2 * i + 1]) for i in range(self.fft_size)])
        expected_result = primes_transformed

        src = blocks.vector_source_c(src_data)
        s2v = blocks.stream_to_vector(gr.sizeof_gr_complex, self.fft_size)
        op = clenabled.clFFT(self.fft_size, -1, 1, 4)
        # op = fft.fft_vcc(self.fft_size, True, [], False)
        v2s = blocks.vector_to_stream(gr.sizeof_gr_complex, self.fft_size)
        dst = blocks.vector_sink_c()
        self.tb.connect(src, s2v, op, v2s, dst)
        start = time.time()
        for i in range(100):
            self.tb.run()
        end = time.time()
        runTime = (end - start) / 100
        print("Time to execute: ", runTime)
        result_data = dst.data()

if __name__ == '__main__':
    gr_unittest.run(qa_clFFT, "qa_clFFT.xml")
