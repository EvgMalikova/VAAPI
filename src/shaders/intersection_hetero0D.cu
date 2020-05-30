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
rtBuffer<int2>    Bonds;

rtDeclareVariable(float, MultiscaleParam, , );

rtDeclareVariable(float, sysSceneEpsilon, , );

//for dynamic staff
rtDeclareVariable(int, PNum, , );
rtDeclareVariable(int, numFrames, , );
rtDeclareVariable(float, TimeSound, , );
//-----------------
//TODO:for SDF
//still have to be implemented as PTX should be generated automatically
//PTX can automatically generated for some primitives like spheres, boxes and etc
//consider further integration with python

//------------
//All inherited SDF functions
//for ray-casting approach structure
//typedef rtCallableProgramX<float3(primParamDesc, float3, float3)> callBoundT;
//rtDeclareVariable(callBoundT, boundIntersection, , );

//typedef rtCallableProgramX<primParamDesc(int)> callReadDataT;
//rtDeclareVariable(callReadDataT, getTimeData, , );

//for sphere tracing of various primitives


inline __device__ float evaluateFunction(float3 x, primParamDesc descPrim)
{
    int type = descPrim.type;
    float f = 10000.0;
    float3 pos1 = descPrim.pos[0];
    float rad1 = descPrim.rad[0];

    f = sdfPrim0(x - pos1, rad1);

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
    float3 pos1 = Positions[primIdx + lower*PNum]; //getting correct frame

    if (numFrames > 0) //dynamic
    {
        pos2 = Positions[primIdx + upper*PNum];
        pos = time*pos2 + (1.0 - time)*pos1; //time interpolation
    }
    else
        pos = pos1; //static

    const float rad = BSRadius[primIdx];
    const int type = BSType[primIdx];
    primParamDesc descPrim;

    descPrim.type = 0; //sphere data type
    descPrim.pos[0] = pos;

    descPrim.rad[0] = rad;

    descPrim.types[0] = type;

    return descPrim;
}

inline __device__   float3 boundIntersection(primParamDesc  descPrim, float3 origin, float3 direction)
{
    const float t = length(descPrim.pos[0] - origin);
    const float3 pos_along_ray = origin + direction * t;
    float tmax = t + 2 * descPrim.rad[0];

    float tmin = fmaxf(0.0, t - descPrim.rad[0]);

    float3 params = make_float3(tmin, tmax, 0.0);
    (length(descPrim.pos[0] - pos_along_ray) < descPrim.rad[0]) ? params.z = 1.0 : params.z = 0.0; //within bounding sphere

    return params;
}

//Main program
RT_PROGRAM void intersection_mol(int primIdx)
{
    float tmin, tmax;
    tmin = 0;
    tmax = RT_DEFAULT_MAX;
    float epsilon = 0.001;//delta;
    if (epsilon > sysSceneEpsilon) epsilon = sysSceneEpsilon;
    float eps = 0.001;
    if (eps > sysSceneEpsilon) eps = sysSceneEpsilon;

    /* ------------------
    /* 1) Reading data and accessing current positions for current time
    /---------------------------------------------------------------*/

    primParamDesc descPrim = getTimeData(primIdx);

    //---------------
    float3 interSectParams = boundIntersection(descPrim, theRay.origin, theRay.direction);
    //if within bounding volume intersection
    //float3 interSectParams = BoundingSubVolumesIntersect(descPrim);
    if (interSectParams.z > 0.0)
    {
        tmin = interSectParams.x;
        tmax = interSectParams.y;
        float totalDistance = tmin;
        // === Raymarching (Sphere Tracing) Procedure ===

        totalDistance = SphereTraceForward(epsilon, tmin, tmax, descPrim);

        // Found potential intersection?
        if (totalDistance < tmax) //we found intersection
        {
            float totalDistance2 = SphereTraceBack(epsilon, totalDistance, tmax, descPrim);
            //------------
            if (totalDistance2 > epsilon)
            { //it is sufficiently large subvolume to ray-cast
                if (rtPotentialIntersection(totalDistance))
                {
                    //compute normal for primitive
                    float3 x = theRay.origin + theRay.direction*totalDistance;

                    float3 cNormal = computeNormal(eps, x, descPrim);

                    //varNormal = normalize(make_float3(dx, dy, dz));

                    //for material

                    infoH.normal = cNormal;
                    infoH.hit_point = theRay.origin + theRay.direction * (totalDistance);
					infoH.tmin=totalDistance;

                    infoH.desc = descPrim;
                    infoH.maxDist = totalDistance2;
                    rtReportIntersection(MaterialIndex);
                }
            }
        }
    }
}

//bounding box
RT_PROGRAM void primitive_bounds(int primIdx, float result[6])
{
    const float3 position = Positions[primIdx];
    //we compute only bounding sphere like parameters
    //the exact bounding box of SDF primitive can be much smaller
    const float radius = BSRadius[primIdx];

    optix::Aabb *aabb = (optix::Aabb *) result;

    aabb->m_min.x = position.x - radius;
    aabb->m_min.y = position.y - radius;
    aabb->m_min.z = position.z - radius;

    aabb->m_max.x = position.x + radius;
    aabb->m_max.y = position.y + radius;
    aabb->m_max.z = position.z + radius;
}