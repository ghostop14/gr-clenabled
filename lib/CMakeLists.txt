# Copyright 2011,2012,2016 Free Software Foundation, Inc.
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

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
link_directories(${FFTW3F_LIBRARY_DIRS})

list(APPEND clenabled_sources
    clMathConst_impl.cc
    GRCLBase.cpp
    clMathOp_impl.cc
    firdes.cc
    window.cc
    fft_filter.cc
    fft.cc
    clFilter_impl.cc
)

set(clenabled_sources "${clenabled_sources}" PARENT_SCOPE)
if(NOT clenabled_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT clenabled_sources)

add_library(gnuradio-clenabled SHARED ${clenabled_sources})
target_link_libraries(gnuradio-clenabled ${Boost_LIBRARIES} ${GNURADIO_ALL_LIBRARIES} OpenCL ${VOLK_LIBRARIES} ${Boost_LIBRARIES} ${FFTW3F_LIBRARIES})
set_target_properties(gnuradio-clenabled PROPERTIES DEFINE_SYMBOL "gnuradio_clenabled_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-clenabled PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
include(GrMiscUtils)
GR_LIBRARY_FOO(gnuradio-clenabled RUNTIME_COMPONENT "clenabled_runtime" DEVEL_COMPONENT "clenabled_devel")

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

include_directories(${CPPUNIT_INCLUDE_DIRS})

list(APPEND test_clenabled_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_clenabled.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_clenabled.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_clFilter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_clMathOp.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_clMathConst.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/clMathOp_impl.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/clMathConst_impl.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/GRCLBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clFilter_impl.cc
    
)

add_executable(test-clenabled ${test_clenabled_sources})

target_link_libraries(
  test-clenabled
  ${GNURADIO_RUNTIME_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CPPUNIT_LIBRARIES}
  ${VOLK_LIBRARIES}
  ${FFTW3F_LIBRARIES}
  OpenCL
  gnuradio-clenabled
)

GR_ADD_TEST(test_clenabled test-clenabled)

########################################################################
# Print summary
########################################################################
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Building for version: ${VERSION} / ${LIBVER}")

