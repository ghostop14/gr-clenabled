// http://stackoverflow.com/questions/20613013/opencl-float-sum-reduction
__kernel void reduction_vector(__global float4* data,__local float4* partial_sums, __global float* output) 
{
    int lid = get_local_id(0);
    int group_size = get_local_size(0);
    partial_sums[lid] = data[get_global_id(0)];
    barrier(CLK_LOCAL_MEM_FENCE);

    for(int i = group_size/2; i>0; i >>= 1) {
        if(lid < i) {
            partial_sums[lid] += partial_sums[lid + i];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    if(lid == 0) {
        output[get_group_id(0)] = dot(partial_sums[0], (float4)(1.0f));
    }
}


#define ALPHA 0.001
// Beta = 1 - alpha
#define BETA = 0.999

struct ComplexStruct {
	float real;
	float imag;
};

typedef struct ComplexStruct SComplex;

__kernel void RMSSum(__global SComplex* restricted data,__local float4* partial_sums, __global float* output) 
{
    int lid = get_local_id(0);
    int group_size = get_local_size(0);
    float curReal=data[get_global_id(0)].real;
    float curImag=data[get_global_id(0)].imag;
    
   	double mag_sqrd = ALPHA * (curReal*curReal + curImag*curImag);
    partial_sums[lid] = mag_sqrd;
   	
    barrier(CLK_LOCAL_MEM_FENCE);

	#pragma unroll
    for(int i = group_size/2; i>0; i >>= 1) {
        if(lid < i) {
            partial_sums[lid] += partial_sums[lid + i];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    if(lid == 0) {
    	// still need to do sqrt(sum/n) outside.  Passing in the extra param
    	// would negatively impact performance.
        output[get_group_id(0)] = dot(partial_sums[0], (float4)(1.0f));
    }
}
