/*

 */

#ifndef PER_RAY_DATA_H
#define PER_RAY_DATA_H

#include <optixu/optixu_vector_types.h>
#include "../primDesc.h"
 // Set by BSDFs which support direct lighting. Not set means specular interaction. Cleared in the closesthit program.
 // Used to decide when to do direct lighting and multuiple importance sampling on implicit light hits.
#define FLAG_DIFFUSE        0x00000002

// Set if (0.0f <= wo_dot_ng), means looking onto the front face. (Edge-on is explicitly handled as frontface for the material stack.)
#define FLAG_FRONTFACE      0x00000010

// Highest bit set means terminate path.
#define FLAG_TERMINATE      0x80000000

// Keep flags active in a path segment which need to be tracked along the path.
// In this case only the last surface interaction is kept.
// It's needed to track the last bounce's diffuse state in case a ray hits a light implicitly for multiple importance sampling.
// FLAG_DIFFUSE is reset in the closesthit program.
#define FLAG_CLEAR_MASK     FLAG_DIFFUSE

const int MAX_PRIM_ALONG_RAY = 60;
typedef optix::float2 auditoryPrim[MAX_PRIM_ALONG_RAY];

typedef primParamDesc sdfPrims[MAX_PRIM_ALONG_RAY];

struct cellPrimDesc {
    float intersectionDist;
    int type; //0-main,1-secondary

    float maxDist;
    optix::float4 color;
    optix::float3 normal;

    //primParamDesc desc;
};

typedef cellPrimDesc  cellPrim[MAX_PRIM_ALONG_RAY];

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
    float totalDist;
    //dynamic parameters
    //TODO: consider to moving as attributes
    float TimeSound;
    // float ao;
    int length;
    float maxDist;
    // int isDynamic; //currently not used, conterolled by time

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

    //auditoryPrim primitives; //array of prim intersection parameters for ray
    //for depth sorting of primitives with intersection
    cellPrim cellPrimitives;
    sdfPrims  prims;

    /*<used for main direction definition*/
    optix::float3 dirCamera; /*<camera direction*/

    bool block;

    //used for auditory tracing
   // managed separately by depth
    //can be potentially joined with primitives array above
   // optix::float3 Distances[MAX_PRIM_ALONG_RAY]; //array of distances and intersection parameters for ray, those are: Freq, Ainit

    int renderType;  //is used to compute complex materials
    //0 - no intersection with any object
    //1 - intersection with basic SDF material
    //2 - perform the last precomputation to the integrated object
    optix::float3 last_hit_point;
    //float maxDist; //bounding box size for volume rendering
    optix::float3 normal;
};

struct PerAudioRayData
{
    float totalDist;
    //dynamic parameters
    //TODO: consider to moving as attributes
    float TimeSound;
    float ao;
    int length;
    float maxDist;
    int isDynamic; //currently not used, conterolled by time
                   //----------------------
                   //OPTICAL MODEL PARAMETERS
                   //----------------------
                   /*
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

                   */
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
              //for depth sorting of primitives with intersection
    cellPrim cellPrimitives;

    /*<used for main direction definition*/
    optix::float3 dirCamera; /*<camera direction*/

    bool block;

    //used for auditory tracing
    // managed separately by depth
    //can be potentially joined with primitives array above
    optix::float3 Distances[MAX_PRIM_ALONG_RAY]; //array of distances and intersection parameters for ray, those are: Freq, Ainit

    int renderType;  //is used to compute complex materials
                     //0 - no intersection with any object
                     //1 - intersection with basic SDF material
                     //2 - perform the last precomputation to the integrated object
    optix::float3 last_hit_point;
    //float maxDist; //bounding box size for volume rendering
    optix::float3 normal;
    // optix::float3direction
    //to delete later
    optix::float4 sum;
};

#endif // PER_RAY_DATA_H
