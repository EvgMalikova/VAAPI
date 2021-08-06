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

rtDeclareVariable(float, blendAdd, , );
//-----------------
//TODO:for SDF
//still have to be implemented as PTX should be generated automatically
//PTX can automatically generated for some primitives like spheres, boxes and etc
//consider further integration with python

inline __device__ float evaluateFunction(float3 x, primParamDesc descPrim)
{
    int type = descPrim.type;
    float f = 10000.0;
    float3 pos1 = descPrim.pos[0];
    float rad1 = descPrim.rad[0];
    float3 pos2 = descPrim.pos[1];
    float rad2 = descPrim.rad[1];
    f = sdfPrim1(x, pos1, pos2, rad1, rad2);

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
    int numFr = 3;
    const int2 ids = Bonds[primIdx];
    const float rad1 = BSRadius[ids.x - 1];
    const float rad2 = BSRadius[ids.y - 1];

    const int type = BSType[ids.x - 1];
    const int type2 = BSType[ids.y - 1];

    //---------------
    int lower = int(floorf(TimeSound));
    int upper = int(ceilf(TimeSound));

    float timeS = TimeSound;
    if (upper > numFrames) upper = numFrames;

    float time = timeS - float(lower);

    //int time = int(floorf(TimeSound)); //integer part
    //int upper=int(time);

    //if (time > numFrames) time = numFrames;

    //for bond interpolation
    float3 pos2 = make_float3(0);
    float3 pos = make_float3(0);

    //for frames
    float3 pos12 = make_float3(0);
    float3 pos11 = make_float3(0);

    float3 pos22 = make_float3(0);
    float3 pos21 = make_float3(0);

    //float3 pos1 = Positions[primIdx + time*PNum]; //getting correct frame

    pos11 = Positions[ids.x - 1 + lower*PNum];
    pos12 = Positions[ids.y - 1 + lower*PNum];

    pos = Positions[ids.x - 1 + lower*PNum];
    pos2 = Positions[ids.y - 1 + lower*PNum];

    if (numFrames > 0) //dynamic
    {
        pos21 = Positions[ids.x - 1 + upper*PNum];
        pos22 = Positions[ids.y - 1 + upper*PNum];

        pos = time*pos21 + (1.0 - time)*pos11; //time interpolation
        pos2 = time*pos22 + (1.0 - time)*pos12; //time interpolation
    }
    primParamDesc descPrim;

    descPrim.type = 2;
    descPrim.pos[0] = pos;
    descPrim.pos[1] = pos2;
    descPrim.rad[0] = rad1;
    descPrim.rad[1] = rad2;
    descPrim.types[0] = type;
    descPrim.types[1] = type2;

    return descPrim;
}

inline __device__   float3 boundIntersection(primParamDesc  descPrim, float3 origin, float3 direction)
{
    float3 dir = normalize(descPrim.pos[0] - descPrim.pos[1]);
    float rad = (length(descPrim.pos[1] - descPrim.pos[0]) + descPrim.rad[1] + descPrim.rad[0]) / 2 + 2 * blendAdd;

    const float3 cent = descPrim.pos[0] + dir* (descPrim.rad[0] - rad);// (descPrim.pos[1] + descPrim.pos[0] + dir*descPrim.rad[0] - dir*descPrim.rad[1]) / 2;

    float radM = rad / 2;
    float3 c1 = cent + dir*radM;
    float3 c2 = cent - dir*radM;

    const float t1 = length(c1 - origin);
    const float t2 = length(c2 - origin);

    const float3 pos_along_ray1 = origin + direction * t1;
    const float3 pos_along_ray2 = origin + direction * t2;

    //float maxRad = fmaxf(descPrim.rad[0], descPrim.rad[1]);
    //float traced_bound = leng + maxRad;
    float tmax = t1 + radM; //to stop sphere tracing

    float tmin = fmaxf(0.0, t1 - radM);
    float3 params = make_float3(tmin, tmax, 0.0);
    if ((length(c1 - pos_along_ray1) < rad))
    {
        params.z = 1.0;
        params.y = t1 + rad;
        params.x = fmaxf(0.0, t1 - rad);
    }
    if (length(c2 - pos_along_ray2) < rad) {
        params.z = 1.0;
        params.y = fmaxf(params.y, t2 + rad);
        float r2 = fmaxf(0.0, t2 - rad);
        params.x = fminf(params.x, r2);
    }
    return params;
    /*
    float rad = fmax(descPrim.rad[0], descPrim.rad[1]) + blendAdd;
    const float3 cent = (descPrim.pos[0] + descPrim.pos[1]) / 2;
    float3 c1 = descPrim.pos[0];
    float3 c2 = descPrim.pos[1];

    const float t1 = length(c1 - origin);
    const float t2 = length(c2 - origin);

    const float3 pos_along_ray1 = origin + direction * t1;
    const float3 pos_along_ray2 = origin + direction * t2;

    float rad1 = length(c1 - cent) + rad;
    float rad2 = length(c2 - cent) + rad;

    float tmax = t1 + rad1; //to stop sphere tracing

    float tmin = fmaxf(0.0, t1 - rad1);
    float3 params = make_float3(tmin, tmax, 0.0);
    if ((length(c1 - pos_along_ray1) < rad1))
    {
        params.z = 1.0;
        params.y = t1 + rad1 * 2;
        params.x = fmaxf(0.0, t1 - rad1);
    }
    if (length(c2 - pos_along_ray2) < rad2) {
        params.z = 1.0;
        params.y = fmaxf(params.y, t2 + rad2 * 2);
        float r2 = fmaxf(0.0, t2 - rad2);
        params.x = fminf(params.x, r2);
    }
    return params;*/
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

    // primParamDesc descPrim = getTimeData(primIdx);
    int numFr = 3;
    const int2 ids = Bonds[primIdx];
    const float rad1 = BSRadius[ids.x - 1];
    const float rad2 = BSRadius[ids.y - 1];

    const int type = BSType[ids.x - 1];
    const int type2 = BSType[ids.y - 1];

    //---------------
    int lower = int(floorf(TimeSound));
    int upper = int(ceilf(TimeSound));

    float timeS = TimeSound;
    if (upper > numFrames) upper = numFrames;

    float time = timeS - float(lower);

    //int time = int(floorf(TimeSound)); //integer part
    //int upper=int(time);

    //if (time > numFrames) time = numFrames;

    //for bond interpolation
    float3 pos2 = make_float3(0);
    float3 pos = make_float3(0);

    //for frames
    float3 pos12 = make_float3(0);
    float3 pos11 = make_float3(0);

    float3 pos22 = make_float3(0);
    float3 pos21 = make_float3(0);

    //float3 pos1 = Positions[primIdx + time*PNum]; //getting correct frame

    pos11 = Positions[ids.x - 1 + lower*PNum];
    pos12 = Positions[ids.y - 1 + lower*PNum];

    pos = Positions[ids.x - 1 + lower*PNum];
    pos2 = Positions[ids.y - 1 + lower*PNum];

    if (numFrames > 0) //dynamic
    {
        pos21 = Positions[ids.x - 1 + upper*PNum];
        pos22 = Positions[ids.y - 1 + upper*PNum];

        pos = time*pos21 + (1.0 - time)*pos11; //time interpolation
        pos2 = time*pos22 + (1.0 - time)*pos12; //time interpolation
    }
    primParamDesc descPrim;

    descPrim.type = 2;
    descPrim.pos[0] = pos;
    descPrim.pos[1] = pos2;
    descPrim.rad[0] = rad1;
    descPrim.rad[1] = rad2;
    descPrim.types[0] = type;
    descPrim.types[1] = type2;

    //---------------
    float3 interSectParams = boundIntersection(descPrim, theRay.origin, theRay.direction);

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
                    infoH.tmin = totalDistance;

                    infoH.desc = descPrim;
                    infoH.maxDist = totalDistance2;
                    rtReportIntersection(MaterialIndex);
                }
            }
        }
    }
}

//--balls and sticks
RT_PROGRAM void boundingbox_bond_mol(int primIdx, float result[6])
{
    const int2 ids = Bonds[primIdx];
    const float rad1 = BSRadius[ids.x - 1];
    const float rad2 = BSRadius[ids.y - 1];

    float3 pos = Positions[ids.x - 1];
    float3 pos2 = Positions[ids.y - 1];

    float3 pos_min = fminf(pos, pos2);
    float3 pos_max = fmaxf(pos, pos2);

    if (numFrames > 0)
    {
        for (int i = 1; i < numFrames; i++)
        {
            pos = Positions[ids.x - 1 + i*PNum];
            pos2 = Positions[ids.y - 1 + i*PNum];

            pos_min = fminf(fminf(pos, pos2), pos_min);
            pos_max = fmaxf(fmaxf(pos, pos2), pos_max);
        }
    }

    float rad = fmaxf(rad1, rad2);
    optix::Aabb* aabb = (optix::Aabb*)result;
    //increase for ao by 5
    aabb->m_min = pos_min - make_float3(rad + blendAdd);
    aabb->m_max = pos_max + make_float3(rad + blendAdd);
}