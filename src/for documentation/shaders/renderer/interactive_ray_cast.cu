/*
 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "rt_function.h"
#include "per_ray_data.h"
#include "random_number_generators.h"

#include "auditory_rendering.h"

 //optical output buffer
rtBuffer<float4, 2> sysOutputBuffer; // RGBA32F

//auditory output buffer
rtBuffer<auditoryPrim, 2> sysAuditoryOutputBuffer; // RGBA32F

//dynamic/static scene parameters
rtDeclareVariable(int, isDynamic, , ); //the scene is dynamic
rtDeclareVariable(int, computeAuditoryRendering, , ); //should we compute auditory rendering

rtDeclareVariable(rtObject, sysTopObject, , );

//optix vars similar to cuda kernel executions vars
rtDeclareVariable(uint2, theLaunchDim, rtLaunchDim, );
rtDeclareVariable(uint2, theLaunchIndex, rtLaunchIndex, );

rtDeclareVariable(float3, sysCameraPosition, , );
rtDeclareVariable(float3, sysCameraU, , );
rtDeclareVariable(float3, sysCameraV, , );
rtDeclareVariable(float3, sysCameraW, , );

//rtDeclareVariable(int, renderWidget, , );
//rtDeclareVariable(float3, sysCords, , );

static __device__ __inline__  optix::Ray ComputeDirPos(PerRayData& prd)
{
    //---------------
    //Linking to cuda threads. This is implemented as in CUDA ADVANCED SAMPLES
    //link pixel number to thread
    const float2 pixel = make_float2(theLaunchIndex);

    //no antializing
    const float2 fragment = pixel + 0.5;

    // The launch dimension (set with rtContextLaunch) is the full client window in this demo's setup.
    const float2 screen = make_float2(theLaunchDim);

    const float2 ndc = (fragment / screen) * 2.0f - 1.0f;

    const float3 origin = sysCameraPosition;
    const float3 direction = optix::normalize(ndc.x * sysCameraU + ndc.y * sysCameraV + sysCameraW);

    if (isDynamic)
        prd.TimeSound = TimeSound;

    //TODO: we now just compute optical_LaunchDim/auditory_LaunchDim ratio

   /* if (computeAuditoryRendering>0) {
        int numS = 0;
        prd.isSoundRay = isSoundRay(numS, ndc, pixel, screen);
        prd.numS = numS;
    }*/

    // Create ray
    return optix::make_Ray(origin, direction, 0, 0.0f, RT_DEFAULT_MAX);
}

// Entry point for a pinhole camera.
RT_PROGRAM void raygeneration()
{
    PerRayData prd;
    // Initialize the random number generator seed from the linear pixel index and the iteration index.
    prd.seed = tea<16>(theLaunchIndex.y * theLaunchDim.x + theLaunchIndex.x, 0);

    prd.radiance = make_float3(0.0f);
    prd.depth = 0;
    prd.result = make_float4(0.5);
    prd.cur_prim = 0;
    prd.isSoundRay = false;
    prd.rnd = rng(prd.seed);
    prd.TimeSound = 0.0f;
    prd.isDynamic = isDynamic;

    //compute normalized ray direction[-1,1]
    optix::Ray ray = ComputeDirPos(prd);

    // Start tracing ray from the camera and further
    rtTrace(sysTopObject, ray, prd);

    prd.radiance *= make_float3(prd.result);

    sysOutputBuffer[theLaunchIndex] = make_float4(prd.radiance, 1.0f);
}

RT_PROGRAM void auditory_raygeneration()
{
    PerRayData prd;
    // Initialize the random number generator seed from the linear pixel index and the iteration index.
    prd.seed = tea<16>(theLaunchIndex.y * theLaunchDim.x + theLaunchIndex.x, 0);

    prd.radiance = make_float3(0.0f);
    prd.depth = 0;
    prd.result = make_float4(0.5);
    prd.cur_prim = 0;
    prd.isSoundRay = false;
    prd.rnd = rng(prd.seed);
    prd.TimeSound = 0.0f;

    for (int i = 0; i < MAX_PRIM_ALONG_RAY; i++)
    {
        prd.primitives[i] = make_float2(0);
    }

    //compute normalized ray direction[-1,1]
    optix::Ray ray = ComputeDirPos(prd);

    //trace for auditory rays
    //if (prd.isSoundRay) {
    prd.TimeSound = TimeSound;

    // Start tracing ray from the camera and further
    rtTrace(sysTopObject, ray, prd);
    //to this point prd is filled with intersection info

    //intf("launched");
    //TODO: write to auditory buffer
    //related to numS (raw_id)
    int num = 0;
    for (int i = 0; i < MAX_PRIM_ALONG_RAY - 1; i++)
    {
        sysAuditoryOutputBuffer[theLaunchIndex][i] = prd.primitives[i];
        if (prd.primitives[i].y > 0) num++;
    }
    sysAuditoryOutputBuffer[theLaunchIndex][MAX_PRIM_ALONG_RAY - 1] = make_float2(prd.cur_prim, num);
    //if (num > 0)
    //printf("%d ", num);
    //TODO: do something with outputsys buff as postprocessing
      //  }
}