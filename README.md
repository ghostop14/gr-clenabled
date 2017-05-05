# gr-clenabled - OpenCL-enabled common blocks for GNURadio


Gr-clenabled had a number of lofty goals at the project’s onset.  the goal was to go through as many GNURadio blocks as possible that are used in common digital communications processing (ASK, FSK, and PSK), convert them to OpenCL, and provide scalability by allowing each OpenCL-enabled block to be assigned to a user-selectable OpenCL device.  This latter scalability feature would allow a system that has 3 graphics cards, or even a combination of GPU’s and FPGA’s, to have different blocks assigned to run on different cards all within the same flowgraph.  This flexibility would also allow lower-end cards to drive less computational blocks and allow FPGA’s to handle the more intensive blocks.  


The following blocks are implemented in this project:


1.	Basic Building Blocks

	a.	Signal Source
	
	b.	Multiply
	
	c.	Add
	
	d.	Subtract
	
	e.	Multiply Constant
	
	f.	Add Constant
	
	g.	Filters
	
		i.	Low Pass
		
		ii.	High Pass
		
		iii.	Band Pass
		
		iv.	Band Reject
		
		v.	Root-Raised Cosine
		
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
	

## Building gr-clenabled

In the setup_help directory there are some installation notes for various configurations with details to get set up and any prerequisites.

## Included Command-line tools

There are a number of command-line tools that are included with the project that can help with tuning and testing.

test-clenabled - Provides block timing for each of the modules.  You can specify what hardware to run it on and what block size you would like to run it with.

test-clkernel - Used to help test/compile custom kernels and provide timing.

test-clfilter - Provide filter timing information based on a specified number of taps.
