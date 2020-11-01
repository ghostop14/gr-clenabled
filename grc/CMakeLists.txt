# Copyright 2011 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

install(FILES
    clenabled_clAdd.block.yml 
    clenabled_clAddConst.block.yml
    clenabled_clMultConst.block.yml
    clenabled_clBandPassFilter.block.yml
    clenabled_clBandRejectFilter.block.yml
    clenabled_clComplexConjugate.block.yml
    clenabled_clComplexToArg.block.yml
    clenabled_clComplexToMag.block.yml 
    clenabled_clComplexToMagPhase.block.yml
    clenabled_clFFT.block.yml
	clenabled_clFIRTapFilter.block.yml
    clenabled_clHighPassFilter.block.yml
    clenabled_clLog10.block.yml
    clenabled_clLowPassFilter.block.yml
    clenabled_clMagPhaseToComplex.block.yml
    clenabled_clMultiply.block.yml
    clenabled_clMultiplyConjugate.block.yml
    clenabled_clQuadratureDemod.block.yml
    clenabled_clRootRaisedCosine.block.yml
    clenabled_clSNR.block.yml
    clenabled_clSubtract.block.yml
    clenabled_clKernel1To1.block.yml
    clenabled_clKernel2To1.block.yml
    clenabled_clSignalSource.block.yml
    clenabled_clCostasLoop.block.yml
    clenabled_clComplexFilter.block.yml
    clenabled_clPolyphaseChannelizer.block.yml
    clenabled_clXCorrelate.block.yml DESTINATION share/gnuradio/grc/blocks
)
