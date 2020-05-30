#pragma once
#include <optix_math.h>


__device__
inline float sdSphere(optix::float3 p, float s)
{
    return length(p) - s;
}/**/
