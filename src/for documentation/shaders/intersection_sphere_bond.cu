/*
All basic variables for SDFs visual-auditory ray-tracing
 */
#include "sdfGeometryVariables.h"


using namespace optix;

rtBuffer<float3>    Positions;
rtBuffer<int2>    Bonds;
rtBuffer<float>    BSRadius;
rtBuffer<int>    BSType;


rtDeclareVariable(float3, pr_pos, attribute primitive_pos, );
rtDeclareVariable(float, pr_rad, attribute primitive_rad, );
//rtDeclareVariable(int, pr_type, attribute primitive_type, );

//rtDeclareVariable(optix::float3, varNormal, attribute NORMAL, ); //for direct tracing of sdf spheres, or defined set of primitives with definde BB


//TODO:for SDF
//still have to be implemented as PTX should be generated automatically
//PTX can automatically generated for some primitives like spheres, boxes and etc
//consider further integration with python

//for SDF

typedef rtCallableProgramId<float(float3, float3, float3, float, float)> callT;
rtDeclareVariable(callT, sdfPrim, , );

//for dynamic staff
rtDeclareVariable(int, PNum, , );
rtDeclareVariable(int, numFrames, , );
rtDeclareVariable(float, TimeSound, , );

//------------------------------------------------------
//---intersection with dynamic molecule, use of morphing
//------------------------------------------------------
inline __device__ float3 transfer_function(int t)
{
    // return TFBuffer[t];
    switch (t)
    {
    case 1: //H
        return make_float3(1, 1, 1);
        break;
    case 2: //C
        return make_float3(0.5);
        break;
    case 3: //N
        return make_float3(0, 0, 0.5);
        break;
    case 4: //S
        return make_float3(1, 1, 0);
        break;
    case 5: //O
        return make_float3(1, 0, 0);
        break;
    case 6: //P
        return make_float3(1, 0.5, 0);
        break;
    }
    return make_float3(0, 0, 0);
}
//intersection for molecular data

RT_PROGRAM void intersection_bond_mol(int primIdx)
{
    float tmin, tmax;
    tmin = 0;
    tmax = RT_DEFAULT_MAX;

    const int2 ids = Bonds[primIdx];
    const float rad1 = BSRadius[ids.x - 1];
    const float rad2 = BSRadius[ids.y - 1];

    //---------------

    int time = int(floorf(TimeSound)); //integer part
    if (time > numFrames) time = numFrames;

    float fract = TimeSound - float(time); //interpolation part

    //for bond interpolation
    float3 pos2 = make_float3(0);
    float3 pos = make_float3(0);

    //for frames
    float3 pos12 = make_float3(0);
    float3 pos11 = make_float3(0);

    float3 pos22 = make_float3(0);
    float3 pos21 = make_float3(0);

    //float3 pos1 = Positions[primIdx + time*PNum]; //getting correct frame

    pos11 = Positions[ids.x - 1 + time*PNum];
    pos12 = Positions[ids.y - 1 + time*PNum];

    if (numFrames > 0) //dynamic
    {
        if ((time + 1) < numFrames) //set frames count
        {
            pos21 = Positions[ids.x - 1 + (time + 1)*PNum];
            pos22 = Positions[ids.y - 1 + (time + 1)*PNum];
        }
        else {
            pos21 = pos11;
            pos22 = pos12;
        }
        pos = fract*pos12 + (1.0 - fract)*pos11; //time interpolation
        pos2 = fract*pos22 + (1.0 - fract)*pos21; //time interpolation
    }
    else
    {
        //pos = pos; //static

        pos = Positions[ids.x - 1 + time*PNum];
        pos2 = Positions[ids.y - 1 + time*PNum];
    }
    //--------------------

    float3 leng = pos2 - pos;
    const float3 cent = leng / 2.0 + pos;
    //const float rad = BSRadius[primIdx];
    const int type = BSType[ids.x - 1];
    const int type2 = BSType[ids.y - 1];

    float3 col1 = transfer_function(type);
    float3 col2 = transfer_function(type2);

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
            dist = sdfPrim(x, pos, pos2, rad1, rad2);

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
                float dx = sdfPrim(x + make_float3(eps, 0, 0), pos, pos2, rad1, rad2) - sdfPrim(x - make_float3(eps, 0, 0), pos, pos2, rad1, rad2);
                float dy = sdfPrim(x + make_float3(0, eps, 0), pos, pos2, rad1, rad2) - sdfPrim(x - make_float3(0, eps, 0), pos, pos2, rad1, rad2);
                float dz = sdfPrim(x + make_float3(0, 0, eps), pos, pos2, rad1, rad2) - sdfPrim(x - make_float3(0, 0, eps), pos, pos2, rad1, rad2);

                //varNormal = normalize(make_float3(dx, dy, dz));

                //compute color
                float d1 = length(x - pos) - rad1;
                float d2 = length(x - pos2) - rad2;
                float3 color;
                /*if (abs(d1) < 0.0001) color = col1;
                else if (abs(d2) < 0.0001) color = col2;
                else*/
                {
                    float d = abs(d1) + abs(d2);
                    color = (d1 / d)*col2 + (d2 / d)*col1;
                }
                //length sdfPrim(x, pos, pos2, rad1, rad2);

                //for material
                //fill attribute data for material
                float2 inf = make_float2(totalDistance, __int_as_float(type));
                info.primInfo = inf;
                info.type = 0; //don't use mapping
                info.useScalar = color;
                info.normal = normalize(make_float3(dx, dy, dz));
                info.hit_point = theRay.origin + theRay.direction * (totalDistance);
                pr_pos = pos;
                pr_rad = rad1;
                //pr_type = type;

                rtReportIntersection(MaterialIndex);
            }
        }
    }
}

//bounding box
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
    aabb->m_min = pos_min - make_float3(rad);
    aabb->m_max = pos_max + make_float3(rad);
}