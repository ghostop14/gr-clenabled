// Define these parameters dynamically in the kernel source code
// then compile the kernel.  You'll only need to recompile
// if the number of input items or filter taps changes (which it shouldn't
// unless the filter parameters change.
#define N 8192
#define K 128

__kernel void td_FIR_float
( __global const float *restrict InputArray, // Length N
__global const float *restrict FilterArray, // Length K
__global float *restrict OutputArray // Length N+K-1
)
{
	__local float local_copy_input_array[2*K+N];
	__local float local_copy_filter_array[K];
	InputArray += get_group_id(0) * N;
	FilterArray += get_group_id(0) * K;
	OutputArray += get_group_id(0) * (N+K);
	// Copy from global to local
	local_copy_input_array[get_local_id(0)] = InputArray[get_local_id(0)];
	if (get_local_id(0) < K)
		local_copy_filter_array[get_local_id(0)] = FilterArray[get_local_id(0)];
	barrier(CLK_LOCAL_MEM_FENCE);
	// Perform Compute
	float result=0.0f;
	for (int i=0; i<K; i++) {
		result += local_copy_filter_array[K-1-i]*local_copy_input_array[get_local_id(0)+i];
	}
	OutputArray[get_local_id(0)] = result;
}

// Complex:
struct ComplexStruct {
	float real;
	float imag;
};

typedef struct ComplexStruct SComplex;

__kernel void td_FIR_complex
( __global const SComplex *restrict InputArray, // Length N
__global const float *restrict FilterArray, // Length K
__global SComplex *restrict OutputArray // Length N+K-1
)
{
	__local SComplex local_copy_input_array[2*K+N];
	__local float local_copy_filter_array[K];
	
	// These were from multiple data sets and multiple filters.
	// In our case we're just working with 1 filter and input at a time.
	// InputArray += get_group_id(0) * N;
	// FilterArray += get_group_id(0) * K;
	// OutputArray += get_group_id(0) * (N+K);
	
	// Copy from global to local
	local_copy_input_array[get_local_id(0)].real = InputArray[get_local_id(0)].real;
	local_copy_input_array[get_local_id(0)].imag = InputArray[get_local_id(0)].imag;
	
	// Load the filter, remember we're going to call this kernel N times
	// So we only want to load the filter array with how many filter entries we have.
	if (get_local_id(0) < K)
		local_copy_filter_array[get_local_id(0)] = FilterArray[get_local_id(0)];
	
	// This causes a thread wait till all the filters and data arrays have been loaded.
	// Now we're sync'd so we can process the data.
	barrier(CLK_LOCAL_MEM_FENCE);
	
	// Perform Compute
	SComplex result;
	result.real=0.0f;
	result.imag=0.0f;
	
	// Unroll the loop for speed.
	#pragma unroll
	for (int i=0; i<K; i++) {
//		result += local_copy_filter_array[K-1-i]*local_copy_input_array[get_local_id(0)+i];
		result.real += local_copy_filter_array[K-1-i]*local_copy_input_array[get_local_id(0)+i].real;
		result.imag += local_copy_filter_array[K-1-i]*local_copy_input_array[get_local_id(0)+i].imag;
	}
	
	OutputArray[get_local_id(0)].real = result.real;
	OutputArray[get_local_id(0)].imag = result.imag;
}
