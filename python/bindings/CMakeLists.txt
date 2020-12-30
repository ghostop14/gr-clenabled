# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT clenabled_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT clenabled_sources)

########################################################################
# Check for pygccxml
########################################################################
GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
    )

include(GrPybind)

########################################################################
# Python Bindings
########################################################################

list(APPEND clenabled_python_files
    clComplexFilter_python.cc
    clComplexToArg_python.cc
    clComplexToMag_python.cc
    clComplexToMagPhase_python.cc
    clCostasLoop_python.cc
    clFFT_python.cc
    clFilter_python.cc
    clKernel1To1_python.cc
    clKernel2To1_python.cc
    clLog_python.cc
    clMagPhaseToComplex_python.cc
    clMathConst_python.cc
    clMathOp_python.cc
    clPolyphaseChannelizer_python.cc
    clQuadratureDemod_python.cc
    clSignalSource_python.cc
    clSNR_python.cc
    clxcorrelate_fft_vcf_python.cc
    clXCorrelate_python.cc
    clXEngine_python.cc python_bindings.cc)

GR_PYBIND_MAKE_OOT(clenabled 
   ../..
   gr::clenabled
   "${clenabled_python_files}")

install(TARGETS clenabled_python DESTINATION ${GR_PYTHON_DIR}/clenabled COMPONENT pythonapi)
