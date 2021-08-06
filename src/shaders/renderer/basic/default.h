#pragma once
#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_math.h>

#include "../per_ray_data.h"
//#include "../random_number_generators.h"

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

/*Tracks camera, of listener position in case of auditory rendering*/
rtDeclareVariable(float3, sysCameraPosition, , );
rtDeclareVariable(float3, sysCameraU, , );
rtDeclareVariable(float3, sysCameraV, , );
rtDeclareVariable(float3, sysCameraW, , );


rtDeclareVariable(float3, sysBackground, , );