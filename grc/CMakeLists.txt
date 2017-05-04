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
    clenabled_clAdd.xml 
    clenabled_clAddConst.xml
    clenabled_clMultConst.xml
    clenabled_clBandPassFilter.xml
    clenabled_clBandRejectFilter.xml
    clenabled_clComplexConjugate.xml
    clenabled_clComplexToArg.xml
    clenabled_clComplexToMag.xml 
    clenabled_clComplexToMagPhase.xml
    clenabled_clFFT.xml
	clenabled_clFIRTapFilter.xml
    clenabled_clHighPassFilter.xml
    clenabled_clLog10.xml
    clenabled_clLowPassFilter.xml
    clenabled_clMagPhaseToComplex.xml
    clenabled_clMultiply.xml
    clenabled_clMultiplyConjugate.xml
    clenabled_clQuadratureDemod.xml
    clenabled_clRootRaisedCosine.xml
    clenabled_clSNR.xml
    clenabled_clSubtract.xml
    clenabled_clKernel1To1.xml
    clenabled_clKernel2To1.xml
    clenabled_clSignalSource.xml
    clenabled_clCostasLoop.xml DESTINATION share/gnuradio/grc/blocks
)
