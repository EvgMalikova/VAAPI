/*
 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_math.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include "attributeInfo.h"
#include "renderer/random_number_generators.h"
using namespace optix;

rtBuffer<float3>    Positions;
rtBuffer<float>    BSRadius;
rtBuffer<int>    BSType;

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float3, pr_pos, attribute primitive_pos, );
rtDeclareVariable(float, pr_rad, attribute primitive_rad, );
//rtDeclareVariable(int, pr_type, attribute primitive_type, );

//rtDeclareVariable(optix::float3, varNormal, attribute NORMAL, ); //for direct tracing of sdf spheres, or defined set of primitives with definde BB

//sets Material index to call
//0 - optical type
//1 - auditory type

rtDeclareVariable(int, MaterialIndex, , );
rtDeclareVariable(int, PNum, , );
rtDeclareVariable(int, numFrames, , );
rtDeclareVariable(float, TimeSound, , );
rtDeclareVariable(attributeInfo, info, attribute info, );

//TODO:for SDF
//still have to be implemented as PTX should be generated automatically
//PTX can automatically generated for some primitives like spheres, boxes and etc
//consider further integration with python

typedef rtCallableProgramId<float(float3, float)> callT;
rtDeclareVariable(callT, sdfPrim, , );

//for accel build
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

//intersection for molecular data

RT_PROGRAM void intersection_mol(int primIdx)
{
    float tmin, tmax;
    tmin = 0;
    tmax = RT_DEFAULT_MAX;

    //---------------
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
    const float t = length(pos - ray.origin);
    const float3 pos_along_ray = ray.origin + ray.direction * t;
    tmax = t + 2 * rad;
    if (length(pos - pos_along_ray) < (rad))
    {
        tmin = fmaxf(0.0, t - rad);
        // === Raymarching (Sphere Tracing) Procedure ===
        optix::float3 ray_direction = ray.direction;
        optix::float3 eye = ray.origin;
        //    eye.x -= global_t * 1.2f;
        optix::float3 x = eye + tmin * ray_direction;

        const float epsilon = 0.002;//delta;
        const float eps = 0.0001;
        float dist = 0;

        float totalDistance = tmin;//fmaxf(0.0, t - rad);//Jitter * tea<4>(current_prd.seed, frame_number);
        int i = 0;
        bool stop = false;
        while (!stop)
        {
            dist = sdfPrim(x - pos, rad);
            // Step along the ray and accumulate the distance from the origin.
            x += abs(dist) * ray_direction;
            totalDistance += abs(dist);

            // Check if we're close enough or too far.
            if (dist < epsilon || totalDistance > tmax)
            {
                stop = true;
            }
            else i++;
        }

        // Found intersection?
        if (abs(dist) < epsilon)
        {
            if (rtPotentialIntersection(totalDistance))
            {
                //compute normal for primitive
                float dx = sdfPrim(x - pos + make_float3(eps, 0, 0), rad) - sdfPrim(x - pos - make_float3(eps, 0, 0), rad);
                float dy = sdfPrim(x - pos + make_float3(0, eps, 0), rad) - sdfPrim(x - pos - make_float3(0, eps, 0), rad);
                float dz = sdfPrim(x - pos + make_float3(0, 0, eps), rad) - sdfPrim(x - pos - make_float3(0, 0, eps), rad);

                //varNormal = normalize(make_float3(dx, dy, dz));

                //for material
                //fill attribute data for material
                float2 inf = make_float2(totalDistance, __int_as_float(type));
                info.primInfo = inf;
                info.type = type;
                info.normal = normalize(make_float3(dx, dy, dz));
                info.hit_point = ray.origin + ray.direction * (totalDistance);
                info.pos[0] = pos;
                info.rad[0] = rad;
                pr_pos = pos;
                pr_rad = rad;

                rtReportIntersection(MaterialIndex);
            }
        }
    }
}

//bounding box
RT_PROGRAM void boundingbox_mol(int primIdx, float result[6])
{
    float3 posMin = Positions[primIdx];
    float3 posMax = Positions[primIdx];
    float rad = BSRadius[primIdx];

    if (numFrames > 0)//dynamic
    {
        for (int i = 1; i < numFrames; i++) {
            float3 pos1 = Positions[primIdx + i*PNum]; //getting correct frame
            posMin.x = fminf(pos1.x, posMin.x);
            posMin.y = fminf(pos1.y, posMin.y);
            posMin.z = fminf(pos1.z, posMin.z);

            posMax.x = fmaxf(pos1.x, posMax.x);
            posMax.y = fmaxf(pos1.y, posMax.y);
            posMax.z = fmaxf(pos1.z, posMax.z);
        }
    }

    optix::Aabb* aabb = (optix::Aabb*)result;
    aabb->m_min = posMin - make_float3(rad);
    aabb->m_max = posMax + make_float3(rad);
}