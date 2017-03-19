struct ComplexStruct {
	float real;
	float imag; 
};

typedef struct ComplexStruct SComplex;

__kernel void multiply_const_complex(__global SComplex * restrict a, __global SComplex * restrict c) {
	size_t index =  get_global_id(0);
	c[index].real = a[index].real * 3.0;
	c[index].imag = a[index].imag * 3.0;
}
