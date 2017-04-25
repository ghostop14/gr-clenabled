/* -*- c++ -*- */

#define CLENABLED_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "clenabled_swig_doc.i"

%{
#include "clenabled/clMathConst.h"
#include "clenabled/clMathOp.h"
#include "clenabled/clFilter.h"
#include "clenabled/clQuadratureDemod.h"
#include "clenabled/clFFT.h"
#include "clenabled/clLog.h"
#include "clenabled/clSNR.h"
#include "clenabled/clComplexToArg.h"
#include "clenabled/clComplexToMagPhase.h"
#include "clenabled/clMagPhaseToComplex.h"
#include "clenabled/clComplexToMag.h"
#include "clenabled/clKernel1To1.h"
#include "clenabled/clKernel2To1.h"
#include "clenabled/clSignalSource.h"
#include "clenabled/clCostasLoop.h"
%}


%include "clenabled/clMathConst.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clMathConst);
%include "clenabled/clMathOp.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clMathOp);
%include "clenabled/clFilter.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clFilter);
%include "clenabled/clQuadratureDemod.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clQuadratureDemod);
%include "clenabled/clFFT.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clFFT);
%include "clenabled/clLog.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clLog);
%include "clenabled/clSNR.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clSNR);
%include "clenabled/clComplexToArg.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clComplexToArg);
%include "clenabled/clComplexToMagPhase.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clComplexToMagPhase);
%include "clenabled/clMagPhaseToComplex.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clMagPhaseToComplex);
%include "clenabled/clComplexToMag.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clComplexToMag);
%include "clenabled/clKernel1To1.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clKernel1To1);
%include "clenabled/clKernel2To1.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clKernel2To1);
%include "clenabled/clSignalSource.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clSignalSource);
%include "clenabled/clCostasLoop.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clCostasLoop);
