/*
*/
#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "renderer/rt_function.h"
#include "renderer/per_ray_data.h"
#include "transferFunction.h"
#include "primitives.h"

using namespace optix;

rtDeclareVariable(float2, primInfo, attribute primInfo, ); // the attribute is shared with the material
                                                           //the main purpose of the material is to sort all primitives along the ray
                                                           //the actual mapping to material happens in raytracing function, 
                                                           //thus sdf surfaces and volume objects are rendered on the same basis
rtDeclareVariable(float3, pr_pos, attribute primitive_pos, );
rtDeclareVariable(float, pr_rad, attribute primitive_rad, );

rtDeclareVariable(optix::float3, varNormal, attribute NORMAL, ); //for direct tracing of sdf spheres, or defined set of primitives with definde BB
rtDeclareVariable(PerRayData,  prd,            rtPayload, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, rayDist, rtIntersectionDistance, );

typedef rtCallableProgramId<float2(float,float,float)> callT;
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

RT_PROGRAM void volume_any_hit()
{

        float trbf = primInfo.x;

        int idx = __float_as_int(primInfo.y);
        float3 hit_sample = ray.origin + ray.direction * trbf;

        float3 pos = pr_pos;
        float r = pr_rad;
        float multOp = 1.0;

        //highlightment in time
        //--------------------------------------------------
       float2 hi= dynamic(r, trbf, prd.TimeSound);
       r = hi.x;
       multOp = hi.y;
        //-----------------------------


        float4 color_sample = make_float4(0.2f);// , 0.9f, 0.9f, 0.9f);//translucent_grays(drbf, t, tf_type);
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
                //f *= pos.w; //sign(r)
                float4 col = translucent_grays2(f, abs(t22)); //transfere function
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
        float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varNormal));
        prd.radiance+= normal * 0.5f + 0.5f;
       



       
        rtIgnoreIntersection();
   
}

RT_PROGRAM void auditory_volume_any_hit()
{
    //auditory - write all info to array
    if (prd.cur_prim < MAX_PRIM_ALONG_RAY)
    {
        prd.primitives[prd.cur_prim] = primInfo;
        prd.cur_prim++;
       

       // float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varNormal));
        //prd.radiance = normal * 0.5f + 0.5f;
        rtIgnoreIntersection();
    }
}
//D:\nvidia\volume\final\superbuild\build\jane-build\Release
//TODO: not finished
RT_PROGRAM void volume_texture_hit()
{
    float3 hit_sample = ray.origin + ray.direction * rayDist;

    
    //template
    float4 color_sample = make_float4(0.2f);// , 0.9f, 0.9f, 0.9f);//translucent_grays(drbf, t, tf_type);
    int maxSteps = 10;
    float step = 2 * 10.0 / maxSteps;
    for (int i = 0; i < maxSteps; i++) {

        
        //attribute function
        float f = 1.0;//scaleKoef* (sdSphere(pp2, pr_rad));//getAtomVelocity(int(pos.w))* wScale*(sdSphere(pp2, getAtomRadius(int(pos.w))));
        float t22 = 0.4;
                                                     //check we are inside
        {
            
            float4 col = translucent_grays2(f, abs(t22)); //transfere function
                                                          //0.1 - purple
                                                          //0.3 - blue
                                                          //0.4 - light blue+
                                                          //0.6 - blue-green
                                                          //0.7  -green
                                                          //0.9 - yellow
                                                          //0.0 - red
                                                          // pre-multiply alpha
            col.x *= col.w;
            col.y *= col.w;
            col.z *= col.w;
            // "over" operator for front-to-back blending
            color_sample = color_sample + col*(1.0f - color_sample.w);
        }
        hit_sample += ray.direction * step;

    }

    //summ for ray
    prd.radiance = make_float3(color_sample);// *(1.0f - prd.result.w);

    //perform Blin-Phong
    float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varNormal));
    prd.radiance += normal * 0.5f + 0.5f;
}

RT_PROGRAM void volume_closest_hit()
{
}

