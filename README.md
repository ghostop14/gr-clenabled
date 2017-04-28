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
	

Building gr-clenabled

This study was meant to give back to the general SDR and open source community to provide a framework for moving towards ever-increasing digital processing speeds.  The module does require clFFT be installed.  Depending on your linux distro you may be able to ‘sudo apt-get install libclfft-dev’ to get all of the libraries and files to build the module.  It can also be installed directly from source.
Also note that these modules were developed using GNURadio 3.7.10 (the latest at the time of development).  Therefore if you run into any issues, first check that you have the latest GNURadio version and you have the latest Swig and Doxygen installed (see GNURadio’s reference on building OOT modules for more details).  Some people have issues building OOT modules in general when using older 3.7.9 on Ubuntu.  The best approach is to install the latest GNURadio from pybombs.

You will also need an OpenCL implementation installed.  For CPU-based OpenCL, download the Intel software at https://software.intel.com/en-us/intel-opencl/download.  intel_sdk_for_opencl_2016_ubuntu_6.3.0.1904_x64.tar.gz was used for CPU-based testing.  While you may get an OS version issue, it should still install.  

On Ubuntu if you get an error about not finding cl.hpp, download and install the NVIDIA SDK Toolkit.  Once installed find cl.hpp in the installed directory and link it into /usr/include/CL/.

However running OpenCL on a CPU provides arguably a worse multithreading solution.  The real benefit comes from running OpenCL on accelerated hardware.  But this requires the appropriate drivers and libraries.  For instance on Linux running NVIDIA cards you may want to first ‘sudo apt-get install nvidia-opencl-icd’.  PLEASE READ THE DOCUMENTATION FOR GETTING OPENCL WORKING ON YOUR CARD AND VERSION OF LINUX BEFORE PROCEEDING!  Obviously the OpenCL blocks won’t compile or run until an OpenCL implementation is properly configured.
Once you have OpenCL set up, ‘sudo apt-get install clinfo’.  If you can run clinfo and see your card you are ready to proceed.
Now that you have OpenCL correctly set up, clfft installed, and a working GNURadio 3.7.10+ installation, make sure that gnuradio-dev is also installed if installing from a repo.


For a more detailed discussion of the command-line tools available in this module, see the study paper in the docs directory.

## Building

In the setup_help directory there are some installation notes for various configurations with details to get set up and any prerequisites.

