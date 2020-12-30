/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

namespace py = pybind11;

// Headers for binding functions
/**************************************/
/* The following comment block is used for
/* gr_modtool to insert function prototypes
/* Please do not delete
/**************************************/
// BINDING_FUNCTION_PROTOTYPES(
    void bind_clComplexFilter(py::module& m);
    void bind_clComplexToArg(py::module& m);
    void bind_clComplexToMag(py::module& m);
    void bind_clComplexToMagPhase(py::module& m);
    void bind_clCostasLoop(py::module& m);
    void bind_clFFT(py::module& m);
    void bind_clFilter(py::module& m);
    void bind_clKernel1To1(py::module& m);
    void bind_clKernel2To1(py::module& m);
    void bind_clLog(py::module& m);
    void bind_clMagPhaseToComplex(py::module& m);
    void bind_clMathConst(py::module& m);
    void bind_clMathOp(py::module& m);
    void bind_clPolyphaseChannelizer(py::module& m);
    void bind_clQuadratureDemod(py::module& m);
    void bind_clSignalSource(py::module& m);
    void bind_clSNR(py::module& m);
    void bind_clxcorrelate_fft_vcf(py::module& m);
    void bind_clXCorrelate(py::module& m);
    void bind_clXEngine(py::module& m);
// ) END BINDING_FUNCTION_PROTOTYPES


// We need this hack because import_array() returns NULL
// for newer Python versions.
// This function is also necessary because it ensures access to the C API
// and removes a warning.
void* init_numpy()
{
    import_array();
    return NULL;
}

PYBIND11_MODULE(clenabled_python, m)
{
    // Initialize the numpy C API
    // (otherwise we will see segmentation faults)
    init_numpy();

    // Allow access to base block methods
    py::module::import("gnuradio.gr");

    /**************************************/
    /* The following comment block is used for
    /* gr_modtool to insert binding function calls
    /* Please do not delete
    /**************************************/
    // BINDING_FUNCTION_CALLS(
    bind_clComplexFilter(m);
    bind_clComplexToArg(m);
    bind_clComplexToMag(m);
    bind_clComplexToMagPhase(m);
    bind_clCostasLoop(m);
    bind_clFFT(m);
    bind_clFilter(m);
    bind_clKernel1To1(m);
    bind_clKernel2To1(m);
    bind_clLog(m);
    bind_clMagPhaseToComplex(m);
    bind_clMathConst(m);
    bind_clMathOp(m);
    bind_clPolyphaseChannelizer(m);
    bind_clQuadratureDemod(m);
    bind_clSignalSource(m);
    bind_clSNR(m);
    bind_clxcorrelate_fft_vcf(m);
    bind_clXCorrelate(m);
    bind_clXEngine(m);
    // ) END BINDING_FUNCTION_CALLS
}