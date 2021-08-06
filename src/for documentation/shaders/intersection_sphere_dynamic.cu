/*
All basic variables for SDFs visual-auditory ray-tracing
 */
#include "sdfGeometryVariables.h"

using namespace optix;

rtBuffer<float3>    Positions;
rtBuffer<float3>    Positions2;
rtBuffer<float>    BSRadius;
rtBuffer<int>    BSType;

//TODO: remove them and think about something with prd or more complex
//variable
rtDeclareVariable(float3, pr_pos, attribute primitive_pos, );
rtDeclareVariable(float, pr_rad, attribute primitive_rad, );

//TODO:for SDF
//still have to be implemented as PTX should be generated automatically
//PTX can automatically generated for some primitives like spheres, boxes and etc
//consider further integration with python

//for SDF

typedef rtCallableProgramId<float(float3, float3, float)> callT;
rtDeclareVariable(callT, sdfPrim, , );

//rtDeclareVariable(float, TimeSound, , );

//------------------------------------------------------
//---intersection with dynamic molecule, use of morphing
//------------------------------------------------------

//intersection for molecular data

RT_PROGRAM void intersection_dyn_mol(int primIdx)
{
    float tmin, tmax;
    tmin = 0;
    tmax = RT_DEFAULT_MAX;

    const float3 pos = Positions[primIdx];
    const float3 pos2 = Positions2[primIdx];

    float3 leng = pos2 - pos;
    const float3 cent = leng / 2.0 + pos;
    const float rad = BSRadius[primIdx];
    const int type = BSType[primIdx];

    //const float3 pp=pos2*TimeSound+(1-TimeSound)*pos;
    float3 pp = 0.5*pos2 + 0.5*pos; //new center
    const float t = length(pp - theRay.origin);
    const float3 pos_along_ray = theRay.origin + theRay.direction * t;
    tmax = t + length(leng)*4.0; //to stop sphere tracing

   // if (length(pp - pos_along_ray) < length(leng) + rad)
    { //TDO: return it && rtPotentialIntersection(t)) {
        //tmin = t;
        // === Raymarching (Sphere Tracing) Procedure ===
        optix::float3 ray_direction = theRay.direction;
        optix::float3 eye = theRay.origin;
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
            dist = sdfPrim(x - pos, x - pos2, rad);

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
                float dx = sdfPrim(x - pos + make_float3(eps, 0, 0), x - pos2 + make_float3(eps, 0, 0), rad) - sdfPrim(x - pos - make_float3(eps, 0, 0), x - pos2 - make_float3(eps, 0, 0), rad);
                float dy = sdfPrim(x - pos + make_float3(0, eps, 0), x - pos2 + make_float3(eps, 0, 0), rad) - sdfPrim(x - pos - make_float3(0, eps, 0), x - pos2 - make_float3(eps, 0, 0), rad);
                float dz = sdfPrim(x - pos + make_float3(0, 0, eps), x - pos2 + make_float3(eps, 0, 0), rad) - sdfPrim(x - pos - make_float3(0, 0, eps), x - pos2 - make_float3(eps, 0, 0), rad);

                //varNormal = normalize(make_float3(dx, dy, dz));

                //for material
                //fill attribute data for material
                float2 inf = make_float2(totalDistance, __int_as_float(type));
                info.primInfo = inf;
                info.type = type;
                info.normal = normalize(make_float3(dx, dy, dz));
                info.hit_point = eye + ray_direction * (totalDistance);
                pr_pos = pos;
                pr_rad = rad;
                //pr_type = type;

                rtReportIntersection(MaterialIndex);
            }
        }
    }
}

//bounding box
RT_PROGRAM void boundingbox_dyn_mol(int primIdx, float result[6])
{
    const float3 pos = Positions[primIdx];
    const float rad = BSRadius[primIdx];

    const float3 pos2 = Positions2[primIdx];

    float3 pos_min = fminf(pos, pos2);
    float3 pos_max = fmaxf(pos, pos2);

    optix::Aabb* aabb = (optix::Aabb*)result;
    aabb->m_min = pos_min - make_float3(rad);
    aabb->m_max = pos_max + make_float3(rad);
}