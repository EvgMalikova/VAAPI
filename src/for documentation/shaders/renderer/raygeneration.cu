/*
 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_math.h>

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

//This one for interactive widget
//--------------------------------------------------------------------------------------

//Buffers

rtBuffer<float3, 2> movingPoints; // registered point cloud
rtBuffer<float3, 2> fixedPoints; // to be alligned with point cloud
//for SDF

typedef rtCallableProgramId<float(float3, float3)> callT;
rtDeclareVariable(callT, sdfPrim, , );

//TODO: sphere tracing of the widget
RT_CALLABLE_PROGRAM optix::Ray SphereTraceGeometry(optix::Ray  ray, bool& found)
{
    //TODO: consider case when there is no intersection with geometry
    optix::Ray  ray2 = ray;

    float tmin, tmax;
    tmin = 0;
    tmax = RT_DEFAULT_MAX;

    // === Raymarching (Sphere Tracing) Procedure ===

    float eps = 0.001;
    float t = 0.0002;
    float3 rad = make_float3(1.5);

    for (int i = 0; i < 100; i++)
    {
        optix::float3 p = ray.origin + t* ray.direction;
        float hit = abs(sdfPrim(p, rad)); //make it always positive so we can step to border
        if (hit < 0.0002) //|| t > 20.0)
        {
            //there is an intersectuib

            float dx = sdfPrim(p + make_float3(eps, 0, 0), rad) - sdfPrim(p - make_float3(eps, 0, 0), rad);
            float dy = sdfPrim(p + make_float3(0, eps, 0), rad) - sdfPrim(p - make_float3(0, eps, 0), rad);
            float dz = sdfPrim(p + make_float3(0, 0, eps), rad) - sdfPrim(p - make_float3(0, 0, eps), rad);

            //info.normal = normalize(make_float3(dx, dy, dz));
            ray2.origin = p;// ray.origin + t*ray.direction;
            //make it sphere normal
            //ray2.direction = normalize(make_float3(dx, dy, dz));

            found = true;
            break;
        }

        if (t > 20.0) {
            found = false;
            break; //no intersection
        }
        t += abs(hit); //only positive direction
    }

    return ray2;
}

rtDeclareVariable(float3, widgetCenter, , );
//-----------
//For widget Ray generation
static __device__ __inline__  optix::Ray ComputeDirPosWidget(PerRayData& prd)
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

    const float3 origin = widgetCenter;
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

// Entry point for a widget.
//First we should sphere trace it's geometry
RT_PROGRAM void audio_ray_cast()
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

	
	
	
	 prd.dirCamera=optix::normalize(widgetCenter-sysCameraPosition);
    //----------
    //set fail value by default
    movingPoints[theLaunchIndex] = make_float3(-1000);
    fixedPoints[theLaunchIndex] = make_float3(-1000);

    for (int i = 0; i < MAX_PRIM_ALONG_RAY; i++)
    {
        prd.primitives[i] = make_float2(0);
    }

    //compute normalized ray direction[-1,1]
    optix::Ray ray = ComputeDirPosWidget(prd);

    bool found = false;
    optix::Ray ray2 = SphereTraceGeometry(ray, found);

    if (found) {
        prd.TimeSound = TimeSound;

        // Start tracing ray from the camera and further
        rtTrace(sysTopObject, ray2, prd);
        //to this point prd is filled with intersection info

        //conventional gathering of distance information for auditory rendering
        int num = 0;
        for (int i = 0; i < MAX_PRIM_ALONG_RAY - 1; i++)
        {
            sysAuditoryOutputBuffer[theLaunchIndex][i] = prd.primitives[i];
            if (prd.primitives[i].y > 0) num++;
        }
        sysAuditoryOutputBuffer[theLaunchIndex][MAX_PRIM_ALONG_RAY - 1] = make_float2(prd.cur_prim, num);

        //movingPoints[theLaunchIndex] = make_float3(-1000);

        if (num > 0) //there is an intersection
        {
		printf("%d NUM", num);
            //add a moving point and fixed if there is an
            //intersection
            if (prd.primitives[0].x < 1.5) {
                movingPoints[theLaunchIndex] = ray2.origin;
                fixedPoints[theLaunchIndex] = ray2.origin + ray2.direction*prd.primitives[0].x;
            }
        }
        //   else //there is no intersection, identification point
        //   {
        //       movingPoints[theLaunchIndex] = make_float3(-1000);
        //       fixedPoints[theLaunchIndex] = make_float3(-1000);
        //   }
    } //if (num > 0)
    
    //TODO: do something with outputsys buff as postprocessing
      //  }
}