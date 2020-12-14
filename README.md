# gr-clenabled - OpenCL-enabled Common Blocks for GNURadio


Gr-clenabled had a number of lofty goals at the project onset.  The goal was to go through as many GNURadio blocks as possible that are used in common digital communications processing (ASK, FSK, and PSK), convert them to OpenCL, and provide scalability by allowing each OpenCL-enabled block to be assigned to a user-selectable OpenCL device.  This latter scalability feature would allow a system that has 3 graphics cards, or even a combination of GPUs and FPGAs, to have different blocks assigned to run on different cards all within the same flowgraph.  This flexibility would also allow lower-end cards to drive less computational blocks and allow FPGAs to handle the more intensive blocks.


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
	
	h.	**[Updated]** Forward FFT: This block now supports multiple input/output streams using the same FFT parameters.  This allows one GPU to process multiple FFT streams within a single flowgraph.  The ability to handle FFT shifts and taps was also added such that this is a true drop-in replacement for the native FFT block.
	
	i.	**[Updated]** Reverse FFT: Multiple stream support was added here as well. The ability to handle FFT shifts and taps was also added such that this is a true drop-in replacement for the native FFT block.
	
3.	Digital Signal Processing 

	a.	Complex to Mag (used for ASK/OOK)
	
	b.	Quadrature Demod (used for FSK)
	
	c.	Costas Loop (2nd order and 4th Order) - Although performance is horrible on a GPU core: ~ 0.7 MSPS.

	d.	**[New]** Polyphase Channelizer (thanks to Dan Banks and Aaron Giles)
	
	e.	**[New]** Cross-Correlator (time domain and frequency domain, multiple signals) - For the examples included, you'll want gr-xcorrelate (CPU-based cross-correlation with some helper blocks) and gr-lfast for some filter convenience wrappers.

	f.	**[New]** Interferometry X-Engine - This block provides a full X-Engine implementation in GNU Radio based on the industry standard xGPU library.  However, it provides dynamic configuration (xGPU requires re-compiling to change parameters), and more flexibility within the parameters.  The block can also write correlated data directly to disk for performance, and supports a number of input formats: Packed 4-bit FFT vectors (each byte has 4-bit I and 4-bit Q packed as two's complement), IChar (interleaved byte IQ data) complex FFT vectors, or standard complex FFT vectors.

## Supporting Throughput Commandline Tools
There are also a number of performance-measuring command-line tools installed with the module as well:

**test-clenabled**: Provides throughput timing of the implemented blocks
**test-clfilter**: Provides more granular filter timing
**test-clfft**: Provides specific timing capabilities around FFT throughput.
**test-clxcorrelate**: Provides timing of the reference correlator blocks
**test-clxengine**: Provides timing of the full x-engine block

## Important Usage Notes

### [New] Reference Cross-Correlators

Time-domain (OpenCL XCorrelate) and frequency-domain (OpenCL XCorrelate FFT) cross-correlator blocks have been added to support combining inputs from multiple antennas.  

**Time Domain Notes**
For the time-domain block, parallel queues and other techniques were added to this block to try to get as many parallel operations as possible.  However, with the smaller GNURadio block sizes, some other design elements were added to maintain good runtime performance.  The block currently takes complex or float inputs (I'm planning on working in IChar shortly too), and acts as a sink block.  It will produce output PDUs containing the best correlation score and correlation correction lag.  A new CPU-based gr-xcorrelate has a helper block (Extract Delay) that works in tandem with these PDUs that can set variables and control standard delay blocks to align the signals.  This helper block also has a "lock" feature that can allow you to dynamically block/allow the delay changes at runtime with a checkbox or other control.

One design element that was incorporated to keep flowgraph performance optimal is two runtime modes: asynchronous or sequential.  Since we're acting as a sink block and don't need to output realtime streams of data, async mode can take a block of samples for processing, and pass them to another thread for processing in parallel.  The work function can then just return that it processed the samples until the other thread's processing is complete.  At which point the worker thread will signal the work function that it should produce the appropriate PDUs on the main thread and pick up the next block.  This allows the block to correlate as quickly as possible without holding up processing, and should allow for correlation at any flowgraph sample rate.  This also prevents the delay blocks from receiving buffer changes every frame.  In sequential mode, the work function will block until processing is completed, which in some scenarios may be the more appropriate approach.  Async is the default mode for the block.

Another design element that was included is some user-level tuning using 3 configurable parameters:

1. Analysis Window - This defines how many samples should be considered for a single correlation calculation.  The default is 8192.

2. Max Index Search Range - This defines how many samples in either shift direction (forward/backward) should be analyzed.  For OpenCL, this value should be a power of 2 to allow the max() reduction kernel at the end of the processing chain to function optimally.  The default is 512, but this can be adjusted as needed to 256, 1024, 2048, etc.  More searches does require extra processing time, so this parameter also serves as one mechanism to regulate processing time.  Also, if correlation gets too small at the end of the analysis window, it is expected that incorrect offsets could be returned.  So setting this value to say 10-30% of the analysis window minimizes potentially incorrect results.

3. Keep 1 in N Frames - This mechanism can be used to minimize how frequently new delay updates are generated.  For stationary signals, the delays may not change that frequently, so no need to burden down the system and keep producing new varying delay values.  By default this value is 4 and applies to both asynchronous and sequential modes (in async mode, the 1-in-N is calculated from when processing completes and is less deterministic.  In sequential mode, it is truly a deterministic calcualtion).

Examples: There are a number of examples in the examples directory.  Note that you will want to install gr-xcorrelate (up in the ghostop14 github repo) for the extract delay helper function, and gr-lfast for the convenience FFT low pass filter block (this latter block could be dropped from the flowgraph if you do not want to install gr-lfast, just replace it with another filter noting that the FIR filters may lower performance over the FFT version at higher sample rates).

1. "xcorr test opencl" uses a single data source with a controlled delay to demonstrate correct offset calculations.

2. "xcorr test opencl 4 signals" uses 4 RTL-SDR inputs (tested with a kerberossdr input) as an FM receiver and correlates all 4 signals for a single output at 2.4 MSPS without any audio jitter.

3. "xcorr test max rate no ui" drops all of the performance-consuming UI controls, and correlates as an FM receiver at 46 MSPS for audio output without any jitter.  This flowgraph does call out some other limitations in that on the test system above 46 MSPS, the FFT low pass filter block starts to become the bottleneck.  There is no reason why the cross correlator cannot be run at any sample rate in excess of 60 MSPS in async mode, provided the other blocks in your flowgraph can handle the higher rates as well.  Those other blocks will vary in performance based on the CPU of the system they are running on.  The FM audio in this example simply served as a good audio feedback mechanism to indicate when the flowgraph was starting to hit performance issues when audio jitter and delay began to be noticable.

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

The following snippit from the project�s <project>/examples/ kernel1to1_sincos.cl example file shows the cast:

c[index].real = cos((double)a[index].real);

If you don't use the double versions (and use clview to ensure your hardware supports it.... most new hardware does but if you have an old card it may be an issue), the trig functions may only be accurate to 9-10 decimal places which during testing was sufficient to cause the Costas Loop to not decode data.  It also resulted in about 20 dB of noise showing up in the signal source block until it was switched to double.
