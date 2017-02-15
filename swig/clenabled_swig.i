/* -*- c++ -*- */

#define CLENABLED_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "clenabled_swig_doc.i"

%{
#include "clenabled/clMathConst.h"
#include "clenabled/clMathOp.h"
#include "clenabled/clFilter.h"
%}


%include "clenabled/clMathConst.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clMathConst);
%include "clenabled/clMathOp.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clMathOp);
%include "clenabled/clFilter.h"
GR_SWIG_BLOCK_MAGIC2(clenabled, clFilter);
