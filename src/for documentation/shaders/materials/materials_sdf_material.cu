/*
*/
#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "../renderer/rt_function.h"
#include "../renderer/per_ray_data.h"
#include "transferFunction.h"
#include "procedures.h"
#include "../attributeInfo.h"
using namespace optix;

rtDeclareVariable(attributeInfo, info, attribute info, );
// the attribute is shared with the material
                                                           //the main purpose of the material is to sort all primitives along the ray
                                                           //the actual mapping to material happens in raytracing function,
                                                           //thus sdf surfaces and volume objects are rendered on the same basis
rtDeclareVariable(float3, pr_pos, attribute primitive_pos, );
rtDeclareVariable(float, pr_rad, attribute primitive_rad, );
//rtDeclareVariable(int, pr_type, attribute primitive_type, );

//rtDeclareVariable(optix::float3, varNormal, attribute NORMAL, ); //for direct tracing of sdf spheres, or defined set of primitives with definde BB
rtDeclareVariable(PerRayData, prd, rtPayload, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, rayDist, rtIntersectionDistance, );
rtDeclareVariable(rtObject, sysTopObject, , );




typedef rtCallableProgramId<float2(float, float, float)> callT;
rtDeclareVariable(callT, dynamic, , );

RT_CALLABLE_PROGRAM optix::float2 highlight(float pr, float trbf, float TimeSound)
{
    //no perRayData or attributes
    float r = pr;
    float multOp = 1.0;
    // if (prd.isSoundRay)
    {
        if (fabs(TimeSound - trbf / 10) < pr) //we hit this sphere in time
        {
            //Was particle reached
            float Dur = fabs(TimeSound - trbf / 10); // TimeSound / SoundTimeCoef - trbf;
                                                         //printf(" koef %f \n", TimeSound / SoundTimeCoef);
                                                         //float ArbDur = 4.0 / SoundTimeCoef; //duration of each atom oscilattion
                                                         //if (Dur > 0.0f)
            {
                float mult = 1.0;// exp(ArbDur - Dur);
                r *= cos(Dur)*1.2*mult;
                multOp = 5.4*(cos(Dur)*mult) + 0.1;
            }
            //multOp = 0.5; //highlightment coefficients
            //r *= 1.2;

            // printf("%f ", multOp);
        }
    }
    return optix::make_float2(r, multOp);
}

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

RT_PROGRAM void volume_any_hit()
{
    float trbf = info.primInfo.x;

    // int idx = __float_as_int(primInfo.y);
    float3 hit_sample = ray.origin + ray.direction * trbf;

    float3 pos = pr_pos;
    float r = pr_rad;
    float multOp = 1.0;

    //highlightment in time
    //--------------------------------------------------
    float2 hi = dynamic(r, trbf, prd.TimeSound);
    r = hi.x;
    multOp = hi.y;
    //-----------------------------

    float4 color_sample = make_float4(0.2f);// , 0.9f, 0.9f, 0.9f);//translucent_grays(drbf, t, tf_type);

                                            //int t = __float_as_int(primInfo.y);
    const int t = __float_as_int(info.primInfo.y);

    int maxSteps = 10;
    float step = 2 * r / maxSteps;
    for (int i = 0; i < maxSteps; i++) {
        float3 pp2 = make_float3(hit_sample.x - pos.x, hit_sample.y - pos.y, hit_sample.z - pos.z);
        //float f = sdSphere(pp2, pr_rad);//getAtomVelocity(int(pos.w))* wScale*(sdSphere(pp2, getAtomRadius(int(pos.w))));
                                                                              //-0.25- -0.5 - red-yellow
        float scaleKoef = 0;                                                                      //1.7 -1.8 - green
        float t22 = pr_rad;
        if (t22 < 0) scaleKoef = -0.5;
        else scaleKoef = 1.7;
        //attribute function
        float f = scaleKoef* (sdSphere(pp2, pr_rad));//getAtomVelocity(int(pos.w))* wScale*(sdSphere(pp2, getAtomRadius(int(pos.w))));

                                                                              //check we are inside
        if (f <= 0) {
            float3 mainCol = transfer_function(t);
            float4 col = translucent_grays2(f, abs(t22)); //transfere function
            //col /= 3.0;
            col = make_float4(mainCol.x, mainCol.y, mainCol.z, col.w / 4);
            //0.1 - purple
            //0.3 - blue
            //0.4 - light blue+
            //0.6 - blue-green
            //0.7  -green
            //0.9 - yellow
            //0.0 - red
            // pre-multiply alpha
            col.x *= col.w*multOp;
            col.y *= col.w*multOp;
            col.z *= col.w*multOp;
            // "over" operator for front-to-back blending
            color_sample = color_sample + col*(1.0f - color_sample.w);
        }
        hit_sample += ray.direction * step;
    }

    //summ for ray
    prd.result += color_sample*(1.0f - prd.result.w);

    //perform Blin-Phong
    float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));
    prd.radiance += normal * 0.5f + 0.5f;

    rtIgnoreIntersection();
}

RT_PROGRAM void auditory_volume_any_hit()
{
    //auditory - write all info to array
    if (prd.cur_prim < MAX_PRIM_ALONG_RAY)
    {
        prd.primitives[prd.cur_prim] = make_float2(info.primInfo.x, float(info.type));// info.primInfo;
        prd.cur_prim++;

        // float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varNormal));
         //prd.radiance = normal * 0.5f + 0.5f;
        rtIgnoreIntersection();
    }
}

//auditory raytracing
RT_PROGRAM void auditory_raytrace_hit()
{
    //auditory - write all info to array
    if (prd.cur_prim < MAX_PRIM_ALONG_RAY)
    {
        prd.primitives[prd.cur_prim] = make_float2(info.primInfo.x, 5.0);//float(info.type));// info.primInfo;
        prd.cur_prim++;

        float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));
        float3 dir2 = optix::normalize(ray.direction);
        float3 dirNew = prd.dirCamera - dir2;//2 * optix::normalize(info.normal) + dir2;

        float trbf = info.primInfo.x;

        // int idx = __float_as_int(primInfo.y);
        float3 hit_point = ray.origin + ray.direction * trbf;

        //OptiX::Ray refl_ray( hit_point, R, radiance_ray_type,
        //scene_epsilon );
        //optix::Ray refl_ray = ray;
        //refl_ray.origin = hit_point;
        //refl_ray.direction = dirNew;
        //return optix::make_Ray(origin, direction, 0, 0.0f, RT_DEFAULT_MAX);

        optix::Ray refl_ray = optix::make_Ray(hit_point, dirNew, 0, 0.00f, RT_DEFAULT_MAX);

        //continue ray tracing with reflected ray
        rtTrace(sysTopObject, refl_ray, prd);
		//printf("tracing done");
        //rtIgnoreIntersection();
    }
}