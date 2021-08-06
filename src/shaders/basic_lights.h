#ifndef BASICLIGHT_H
#define BASICLIGHT_H
#include <optixu/optixu_vector_types.h>
struct BasicLight
{
    optix::float3 pos;
    optix::float3 color;
   // int    casts_shadow;

   // int    padding;
};

#endif // BASICLIGHT_H