/*
 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_math.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include "attributeInfo.h"

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

//for SDF

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

    aabb->m_min.x = position.x - radius - 1.0;
    aabb->m_min.y = position.y - radius - 1.0;
    aabb->m_min.z = position.z - radius - 1.0;

    aabb->m_max.x = position.x + radius + 1.0;
    aabb->m_max.y = position.y + radius + 1.0;
    aabb->m_max.z = position.z + radius + 1.0;
}

//intersection for molecular data

RT_PROGRAM void intersection_mol(int primIdx)
{
    float tmin, tmax;
    tmin = 0;
    tmax = RT_DEFAULT_MAX;

    int time = int(floorf(TimeSound)); //integer part
    if (time > numFrames) time = numFrames;

    float fract = TimeSound - float(time); //interpolation part
    float3 pos2 = make_float3(0);
    float3 pos = make_float3(0);
    float3 pos1 = Positions[primIdx + time*PNum]; //getting correct frame
    if (numFrames > 0) //dynamic
    {
        if ((time + 1) < numFrames) //set frames count
        {
            pos2 = Positions[primIdx + (time + 1)*PNum];
        }
        else {
            pos2 = pos1;
        }
        pos = fract*pos2 + (1.0 - fract)*pos1; //time interpolation
    }
    else
        pos = pos; //static

    const float rad = BSRadius[primIdx];
    const int type = BSType[primIdx];
    const float t = length(pos - ray.origin);
    const float3 pos_along_ray = ray.origin + ray.direction * t;
    tmax = t + 2 * rad;
    if (length(pos - pos_along_ray) < (rad))
    { //TDO: return it && rtPotentialIntersection(t)) {
            //float4 result = hit_hook(x, max_iterations, global_t);
          //  dist = sdfPrim(x - pos, rad);

            //TDO: return it && rtPotentialIntersection(t)) {
              //tmin = t;
              // === Raymarching (Sphere Tracing) Procedure ===
        optix::float3 ray_direction = ray.direction;
        optix::float3 eye = ray.origin;
        //    eye.x -= global_t * 1.2f;
        optix::float3 x = eye;// +tmin * ray_direction;

        const float epsilon = 0.002;//delta;
        const float eps = 0.0001;
        float dist = 0;

        float totalDistance = 0.0;//Jitter * tea<4>(current_prd.seed, frame_number);
        int i = 0;
        bool stop = false;
        while (!stop)
        {
            //dist = sdfPrim(x - pos, x - pos2, rad);
            dist = sdfPrim(x - pos, rad);
            // Step along the ray and accumulate the distance from the origin.
            x += abs(dist) * ray_direction;
            //dist_from_origin += dist * fudgeFactor;
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
                pr_pos = pos;
                pr_rad = rad;
                //pr_type = type;

                rtReportIntersection(MaterialIndex);
            }
        }
    }
}

//bounding box
RT_PROGRAM void boundingbox_mol(int primIdx, float result[6])
{
    const float3 pos = Positions[primIdx];
    const float rad = BSRadius[primIdx];

    optix::Aabb* aabb = (optix::Aabb*)result;
    aabb->m_min = pos - make_float3(4 * (rad + 1));
    aabb->m_max = pos + make_float3(4 * (rad + 1));
}