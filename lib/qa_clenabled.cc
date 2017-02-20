/*
 * Copyright 2012 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

/*
 * This class gathers together all the test cases for the gr-filter
 * directory into a single test suite.  As you create new test cases,
 * add them here.
 */

#include "qa_clenabled.h"
#include "qa_clMathConst.h"
#include "qa_clMathOp.h"
#include "qa_clFilter.h"
#include "qa_clQuadratureDemod.h"
#include "qa_clFFT.h"

CppUnit::TestSuite *
qa_clenabled::suite()
{
  CppUnit::TestSuite *s = new CppUnit::TestSuite("clenabled");
  s->addTest(gr::clenabled::qa_clMathConst::suite());
  s->addTest(gr::clenabled::qa_clMathOp::suite());
  s->addTest(gr::clenabled::qa_clFilter::suite());
  s->addTest(gr::clenabled::qa_clQuadratureDemod::suite());
  s->addTest(gr::clenabled::qa_clFFT::suite());

  return s;
}
