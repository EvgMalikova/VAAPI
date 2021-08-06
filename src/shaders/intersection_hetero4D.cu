/*
All basic variables for SDFs heterogeneous objects ray-tracing
*/
#include "sdfGeometryVariables.h"

using namespace optix;

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_math.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include "attributeInfo.h"
#include "renderer/random_number_generators.h"

#include "sdfPrimPrograms.h"

rtBuffer<float3>    Positions;
rtBuffer<float>    BSRadius;
rtBuffer<int>    BSType;

//Connectivity info
rtBuffer<int4>    Tets;

rtDeclareVariable(float, MultiscaleParam, , );

rtDeclareVariable(float, sysSceneEpsilon, , );

//for dynamic staff
rtDeclareVariable(int, PNum, , );
rtDeclareVariable(int, numFrames, , );
rtDeclareVariable(float, TimeSound, , );
//-----------------

//for sphere tracing of various primitives

inline __device__ float evaluateFunction(float3 x, primParamDesc descPrim)
{
    int type = descPrim.type;
    float f = 10000.0;
    float3 pos1 = descPrim.pos[0];
    float3 pos2 = descPrim.pos[1];
    float3 pos3 = descPrim.pos[2];
    float3 pos4 = descPrim.pos[3];

    //TODO: replace with prim
    f = sdfPrim4(x, pos1, pos2, pos3, pos4);

    return f;
}

inline __device__ float3 computeNormal(float eps, float3 x, primParamDesc descPrim)
{
    float dx = evaluateFunction(x + make_float3(eps, 0, 0), descPrim) - evaluateFunction(x - make_float3(eps, 0, 0), descPrim);
    float dy = evaluateFunction(x + make_float3(0, eps, 0), descPrim) - evaluateFunction(x - make_float3(0, eps, 0), descPrim);
    float dz = evaluateFunction(x + make_float3(0, 0, eps), descPrim) - evaluateFunction(x - make_float3(0, 0, eps), descPrim);

    return normalize(make_float3(dx, dy, dz));
}
inline __device__ float SphereTraceForward(float epsilon, float t, float tmax, primParamDesc descPrim)
{
    optix::float3 ray_direction = theRay.direction;
    optix::float3 x = theRay.origin + theRay.direction*t;

    float dist;

    float totalDistance = t;
    int i = 0;
    bool stop = false;
    while (!stop)
    {
        dist = evaluateFunction(x, descPrim);// sdfPrim1(x, pos, pos2, rad1, rad2);

                                        // Step along the ray and accumulate the distance from the origin.
        x += abs(dist) * ray_direction;
        totalDistance += abs(dist);

        // Check if we're close enough or too far.
        if (abs(dist) < epsilon || totalDistance >= tmax)
        {
            stop = true;
        }
        if (dist < 0) {
            //x -= abs(dist) * ray_direction;
            totalDistance -= abs(dist);
            stop = true;
        }
    }
    return totalDistance;
}

inline __device__ float SphereTraceBack(float epsilon, float t, float tmax, primParamDesc descPrim)
{
    optix::float3 ray_direction = theRay.direction;
    optix::float3 x = theRay.origin + theRay.direction*tmax;

    float dist;

    //TODO: getType

    float totalDistance = tmax - t;
    int i = 0;
    bool stop = false;

    while (!stop)
    {
        //dist = sdfPrim1(x, pos, pos2, rad1, rad2);
        dist = evaluateFunction(x, descPrim);// sdfPrim1(x, pos, pos2, rad1, rad2);

                                        // Step along the ray and accumulate the distance from the origin.
        x -= abs(dist) * ray_direction;
        totalDistance -= abs(dist);

        // Check if we're close enough or too far.
        if (abs(dist) < epsilon || totalDistance <= 0)
        {
            stop = true;
        }
        if (dist < 0) {
            x += abs(dist) * ray_direction;
            totalDistance += abs(dist);
            stop = true;
        }
    }
    return totalDistance;
}

//------------------------

inline __device__  primParamDesc getTimeData(int primIdx)
{
    int lower = int(floorf(TimeSound));
    int upper = int(ceilf(TimeSound));

    float timeS = TimeSound;
    if (upper > numFrames) upper = numFrames;

    float time = timeS - float(lower);

    float3 pos2 = make_float3(0);
    float3 pos = make_float3(0);

    int4 idx = Tets[primIdx];
    int ids[4];
    ids[0] = idx.x;
    ids[1] = idx.y;
    ids[2] = idx.z;
    ids[3] = idx.w;

    primParamDesc descPrim;
    descPrim.type = 4; //tetra

    float3 cent = make_float3(0);
    for (int j = 0; j < 4; j++)
    {
        float3 pos = Positions[ids[j]];
        descPrim.pos[j] = pos;
        cent += pos;
    }
    /*
    for (int j = 0; j < 4; j++)
    {
        int id = ids[j] + lower*PNum;
        float3 pos1 = Positions[id]; //getting correct frame

        if (numFrames > 0) //dynamic
        {
            id = ids[j] + lower*PNum;
            pos2 = Positions[id];
            pos = time*pos2 + (1.0 - time)*pos1; //time interpolation
        }
        else
        {
            pos = pos1; //static
        }
        descPrim.pos[j] = pos;
        cent += pos;
    }
    */
    cent /= 4;
    const int type = BSType[primIdx];

    descPrim.rad[0] = length(cent - descPrim.pos[0]);

    descPrim.types[0] = type;

    return descPrim;
}

inline __device__   float3 boundIntersection(primParamDesc  descPrim, float3 origin, float3 direction)
{
    float3 cent = make_float3(0);
    for (int j = 0; j < 4; j++)
    {
        cent += descPrim.pos[j];
    }
    cent /= 4.0;
    float rad = length(cent - descPrim.pos[0]);
    const float t = length(cent - origin);
    const float3 pos_along_ray = origin + direction * t;
    float tmax = abs(t + rad);

    float tmin = t;// fmaxf(0.0, t);

    float3 params = make_float3(tmin, tmax, 0.0);
    (length(cent - pos_along_ray) < (rad + 0.1)) ? params.z = 1.0 : params.z = 0.0; //within bounding sphere

    return params;
}

//Main program
RT_PROGRAM void intersection_mol(int primIdx)
{
    float tmin, tmax;
    tmin = 0;
    tmax = RT_DEFAULT_MAX;
    float epsilon = 0.01;//delta;
    if (epsilon > sysSceneEpsilon) epsilon = sysSceneEpsilon;
    float eps = 0.001;
    if (eps > sysSceneEpsilon) eps = sysSceneEpsilon;

    /* ------------------
    /* 1) Reading data and accessing current positions for current time
    /---------------------------------------------------------------*/

    primParamDesc descPrim = getTimeData(primIdx);

    //---------------
    float3 interSectParams = boundIntersection(descPrim, theRay.origin, theRay.direction);
    if (interSectParams.z > 0.0)
    {
        tmin = interSectParams.x;
        tmax = interSectParams.y;
        float total = tmin;

        if (rtPotentialIntersection(total))
        {
            //compute normal for primitive
            float3 x = theRay.origin + theRay.direction*total;

            float3 cNormal = computeNormal(eps, x, descPrim);

            //varNormal = normalize(make_float3(dx, dy, dz));

            //for material

            infoH.normal = cNormal;
            infoH.hit_point = theRay.origin + theRay.direction * (total);
            infoH.tmin = total;

            infoH.desc = descPrim;
            infoH.maxDist = tmax - total;
            rtReportIntersection(MaterialIndex);
        }
    }
}

//bounding box
RT_PROGRAM void primitive_bounds(int primIdx, float result[6])
{
    int4 idx = Tets[primIdx];
    int ids[4];
    ids[0] = idx.x;
    ids[1] = idx.y;
    ids[2] = idx.z;
    ids[3] = idx.w;

    float3 minX = Positions[idx.x];
    float3 maxX = Positions[idx.x];
    // for (int lower = 0; lower < PNum; lower++)
   //  {
    for (int j = 0; j < 4; j++)
    {
        int id = ids[j];// +lower*PNum;
        float3 pos = Positions[id]; //getting correct frame

        minX.x = min(minX.x, pos.x);
        maxX.x = max(maxX.x, pos.x);

        minX.y = min(minX.y, pos.y);
        maxX.y = max(maxX.y, pos.y);

        minX.z = min(minX.z, pos.z);
        maxX.z = max(maxX.z, pos.z);
        //      }
    }
    optix::Aabb *aabb = (optix::Aabb *) result;

    aabb->m_min = minX;
    aabb->m_max = maxX;
}