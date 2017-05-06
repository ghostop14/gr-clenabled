# gr-clenabled - OpenCL-enabled Common Blocks for GNURadio


Gr-clenabled had a number of lofty goals at the project’s onset.  The goal was to go through as many GNURadio blocks as possible that are used in common digital communications processing (ASK, FSK, and PSK), convert them to OpenCL, and provide scalability by allowing each OpenCL-enabled block to be assigned to a user-selectable OpenCL device.  This latter scalability feature would allow a system that has 3 graphics cards, or even a combination of GPU’s and FPGA’s, to have different blocks assigned to run on different cards all within the same flowgraph.  This flexibility would also allow lower-end cards to drive less computational blocks and allow FPGA’s to handle the more intensive blocks.  


The following blocks are implemented in this project:


1.	Basic Building Blocks

	a.	Signal Source
	
	b.	Multiply
	
	c.	Add
	
	d.	Subtract
	
	e.	Multiply Constant
	
	f.	Add Constant
	
	g.	Filters (FIR and FFT versions)
	
		i.	Low Pass
		
		ii.	High Pass
		
		iii.	Band Pass
		
		iv.	Band Reject
		
		v.	Root-Raised Cosine
		
		vi.	General Tap-based
		
2.	Common Math or Complex Data Functions

	a.	Complex Conjugate
	
	b.	Multiply Conjugate
	
	c.	Complex to Arg
	
	d.	Complex to Mag Phase
	
	e.	Mag Phase to Complex
	
	f.	Log10
	
	g.	SNR Helper (a custom block performing divide->log10->abs)
	
	h.	Forward FFT
	
	i.	Reverse FFT
	
3.	Digital Signal Processing 

	a.	Complex to Mag (used for ASK/OOK)
	
	b.	Quadrature Demod (used for FSK)
	
	c.  Costas Loop (2nd order and 4th Order) - Although performance is horrible on a GPU core: ~ 0.7 MSPS.
	

## Important Usage Notes

### Multiple Blocks

Like running an application on your computer that would take 100% CPU utilization, running blocks on a GPU is expecting full hardware utilization for best performance.  If you try to run 2 applications on your CPU that both require 100% utilization, performance of both will suffer.  The same applies to attempting to run multiple OpenCL blocks on the same hardware at the same time.  With that said, these blocks do support using multiple cards simultaneously.  You can assign specific blocks to run on specific cards to spread out the load and this will perform quite well.  The best recommendation is to take the heaviest processing blocks in your flowgraph that have OpenCL equivalents and start with those.

### Hardware Single and Double Precision

Originally some of the blocks like the signal source, quad demod, and the Costas Loop were written with floating point trig functions for hardware compatibility.  However it turns out that trig function accuracy is defined as "Implementation Specific" in the OpenCL spec and the precision of the floating point versions were causing discrepancies.  As a result the blocks now detect if double precision support is available on the hardware and adjust the code accordingly.  The result is actually that the "signal" produced in the signal source with double precision does not have some secondary peaks seen in the native version's frequency plot (see the example flowgraph in the examples directory for the signal source and run it to see the difference).  This shouldn't be a problem on newer hardware.  You can run the included clview command-line tool which will immediately tell you if your hardware supports double-precision (and most newer cards will).  However it is important to check.  Single precision trig was poor enough to cause a real-world signal to not decode with the Costas Loop.  If you're using custom kernels that include trig functions, keep this in mind.  You will want to typecast your sin/cos parameters to (double) or you could get unusable results.

### Filters

A lot of work went into the filters.  Both FFT and FIR versions are implemented.  The test-clfilter command-line tool can help you pick between the 4 filter options:  OpenCL FIR, GNURadio's FIR, OpenCL FFT, GNURadio's FFT.  Given the number of taps in any one filter and the filter parameters, one of these will be better than the other.  In general the order of processing (best to worst) for practical filters will be: GNURadio FFT (by an order of magnitude), OpenCL FIR, GNURadio FIR, OpenCL FFT.

### Performance Metrics

A full study on each of the blocks is in the docs directory.  It goes through each of the blocks and their measured throughput for a variety of data buffer sizes and compares their performance to the native GNURadio blocks.  It was definitely interesting to see the timing on some of the native code as well.  The details of how the tests were conducted, the results, and observations are all detailed in the study.

### Grouping of Blocks

Once gr-clenabled is installed, there are two high-level groups the blocks are organized into.  The first is 'OpenCL-Accelerated'.  These blocks always perform better than the CPU equivalents.  The other group 'OpenCL-Enabled' has blocks that depending on block size or other parameters may be faster or slower, and some that are just always slower but included here for completeness (like basic multiply/add blocks).  For details on how they perform, again look at the performance metrics in the study before choosing.

## Building gr-clenabled

In the setup_help directory there are some installation notes for various configurations with details to get set up and any prerequisites.

## Included Command-line tools

There are a number of command-line tools that are included with the project that can help with tuning and testing.

clview - clinfo-lite.  A quick overview on OpenCL devices with only the information relevant to the blocks.  You can grab platform and device ids very quickly from the output of this tool and also determine if your hardware supports double precision math (important for trig functions).

test-clenabled - Provides block timing for each of the modules.  You can specify what hardware to run it on and what block size you would like to run it with.

test-clkernel - Used to help test/compile custom kernels and provide timing.

test-clfilter - Provide filter timing information based on a specified number of taps.


## Custom Kernel Notes

If you use trig functions in your kernel, verify your hardware supports double precision math.  You can do this with clview which will immediately tell you if your card supports it.  Then in your kernel, you can still pass floats but make sure you typecast parameters to the trig functions as (double) first or you will notice too much variation in the calculated results.  

The following snippit from the project’s <project>/examples/ kernel1to1_sincos.cl example file shows the cast:

c[index].real = cos((double)a[index].real);

If you don't use the double versions (and use clview to ensure your hardware supports it.... most new hardware does but if you have an old card it may be an issue), the trig functions may only be accurate to 9-10 decimal places which during testing was sufficient to cause the Costas Loop to not decode data.  It also resulted in about 20 dB of noise showing up in the signal source block until it was switched to double.
