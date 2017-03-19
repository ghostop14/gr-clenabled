struct ComplexStruct {
	float real;
	float imag; 
};

typedef struct ComplexStruct SComplex;

__kernel void multiply_complex(__global SComplex * restrict a, __global SComplex * restrict b, __global SComplex * restrict c) {
	size_t index =  get_global_id(0);
	float a_r=a[index].real;
	float a_i=a[index].imag;
	float b_r=b[index].real;
	float b_i=b[index].imag;
	c[index].real = (a_r * b_r) - (a_i*b_i);
	c[index].imag = (a_r * b_i) + (a_i * b_r);
}
