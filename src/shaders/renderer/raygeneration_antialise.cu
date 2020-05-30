/* 
 */


#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "rt_function.h"
#include "random_number_generators.h"
#include "per_ray_data.h"


//optical output buffer
rtBuffer<float4,  2> sysOutputBuffer; // RGBA32F

//auditory output buffer


//dynamic/static scene parameters
rtDeclareVariable(bool, isDynamic, , ); //the scene is dynamic
rtDeclareVariable(float, TimeSound, , );

rtDeclareVariable(float, MultiscaleParam, , );

rtDeclareVariable(rtObject, sysTopObject, , );
//rtDeclareVariable(int, sysIterationIndex, , ); //some staff for antializing


//optix vars similar to cuda kernel executions vars
rtDeclareVariable(uint2, theLaunchDim,   rtLaunchDim, );
rtDeclareVariable(uint2, theLaunchIndex, rtLaunchIndex, );

rtDeclareVariable(float3, sysCameraPosition, , );
rtDeclareVariable(float3, sysCameraU, , );
rtDeclareVariable(float3, sysCameraV, , );
rtDeclareVariable(float3, sysCameraW, , );

/*static __device__ __inline__ optix::uchar4 make_color(const optix::float3& c)
{
    return optix::make_uchar4(static_cast<unsigned char>(__saturatef(c.z)*255.99f),  
        static_cast<unsigned char>(__saturatef(c.y)*255.99f),  
        static_cast<unsigned char>(__saturatef(c.x)*255.99f),  
        255u);                                                 
}*/

static __device__ __inline__  optix::float2 ComputeDirPos(unsigned int seed)
{
    //---------------
    //Linking to cuda threads. This is implemented as in CUDA ADVANCED SAMPLES 
    //link pixel number to thread
    const float2 pixel = make_float2(theLaunchIndex);

    //perform antializing
    const float2 fragment = pixel + rng2(seed);

    // The launch dimension (set with rtContextLaunch) is the full client window in this demo's setup.
    const float2 screen = make_float2(theLaunchDim);

    return (fragment / screen) * 2.0f - 1.0f;

   
}

// Entry point for a pinhole camera.
RT_PROGRAM void raygeneration()
{
  PerRayData prd;
  // Initialize the random number generator seed from the linear pixel index and the iteration index.
  prd.seed = tea<16>(theLaunchIndex.y * theLaunchDim.x + theLaunchIndex.x, 0);

  prd.radiance = make_float3(0.0f);
  prd.depth=1;
  prd.result = make_float4(0.5);
  prd.cur_prim = 0;
  prd.isSoundRay = false;
 
  //compute normalized ray direction[-1,1]
  float2 ndc = ComputeDirPos(prd.seed);
 const float3 origin = sysCameraPosition;
  const float3 direction = optix::normalize(ndc.x * sysCameraU + ndc.y * sysCameraV + sysCameraW);

  // Create ray 
  optix::Ray ray = optix::make_Ray(origin, direction, 0, 0.0f, RT_DEFAULT_MAX);

  // Start tracing ray from the camera and further
  rtTrace(sysTopObject, ray, prd);


  prd.radiance *= make_float3(prd.result);
  sysOutputBuffer[theLaunchIndex] = make_float4(prd.radiance, 1.0f);
}
