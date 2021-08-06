/*

 */

#ifndef PER_RAY_DATA_H
#define PER_RAY_DATA_H

#include <optixu/optixu_vector_types.h>

const int MAX_PRIM_ALONG_RAY = 10;
typedef optix::float2 auditoryPrim[MAX_PRIM_ALONG_RAY];

struct primDes
{
    int x;
    int y;
    auditoryPrim data;

    primDes(int xx, int yy, auditoryPrim data_t) {
        x = xx; y = yy;
        for (int i = 0; i < MAX_PRIM_ALONG_RAY; i++) {
            data[i] = data_t[i];
        }
    };
};
struct PerRayData
{
    //dynamic parameters
    //TODO: consider to moving as attributes
    float TimeSound;
    int isDynamic; //currently not used, conterolled by time
    //----------------------
    //OPTICAL MODEL PARAMETERS
    //----------------------

    optix::float3 wo;             // Outgoing direction, to observer, in world space.
    optix::float3 wi;             // Incoming direction, to light, in world space.
    optix::float3 pos;            // Current surface hit point, in world space
    //float t_hit;                  //hit distance along the ray to compute the point of hit

    //BRDF vars
    int           flags;          // Bitfield with flags. See FLAG_* defines for its contents.

    optix::float3 f_over_pdf;     // BSDF sample throughput, pre-multiplied f_over_pdf = bsdf.f * fabsf(dot(wi, ns) / bsdf.pdf;
    float         pdf;            // The last BSDF sample's pdf, tracked for multiple importance sampling.

    unsigned int  seed;           // Random number generator input.
    float rnd;

    //Ray info
    int          depth;     // how many reflections the ray had
    optix::float3 radiance; // Radiance along the current path segment.

    //----------------------
    //AUDITORY MODEL PARAMETERS
    //----------------------

    //for sdf primitives tracing, including sound
    optix::float4 result;
    int           cur_prim; //number of current primitive
    //for sound tracing

    bool isSoundRay; //is this a ray tracing auditory information
    int numS; //auditory id for ray

    auditoryPrim primitives; //array of prim intersection parameters for ray

    /*<used for main direction definition*/
    optix::float3 dirCamera; /*<camera direction*/

    //used for auditory tracing
   // managed separately by depth
    //can be potentially joined with primitives array above
    optix::float3 Distances[MAX_PRIM_ALONG_RAY]; //array of distances and intersection parameters for ray, those are: Freq, Ainit
};

#endif // PER_RAY_DATA_H
