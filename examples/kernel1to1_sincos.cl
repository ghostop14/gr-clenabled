struct ComplexStruct {
	float real;
	float imag; 
};

typedef struct ComplexStruct SComplex;

__kernel void fn_sin_cos(__global SComplex * restrict a, __global SComplex * restrict c) {
    /* You have to be careful with trig functions and precision.
       If you call the float versions of sin/cos for example, it may only be accurate to
       5-6 decimal places for CPU and 9-10 for GPU's which won't be accurate enough
       for signal processing.  So make sure you use the double versions.
    */
    
	size_t index =  get_global_id(0);
	c[index].real = cos((double)a[index].real);
	c[index].imag = sin((double)a[index].imag);
}
