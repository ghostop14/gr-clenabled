__kernel void multiply_float_const(__global float * restrict a, __global float * restrict c) {
	size_t index =  get_global_id(0);
	c[index] = a[index] * 3.0;
}
