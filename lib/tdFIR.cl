#define N 4096
#define K 128
__kernel void tdFIR
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
result += local_copy_of_filter_array[K-1-i]*local_copy_of_input_array[get_local_id(0)+i];
}
OutputArray[get_local_id(0)] = result;
}
