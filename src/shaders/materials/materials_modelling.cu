/*

 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "../renderer/per_ray_data.h"
#include "../basic_lights.h"
 //#include "per_ray_data.h"
#include "../attributeInfo.h"
#include "../sdfPrimPrograms.h"
#include "transferFunction.h"
using namespace optix;

rtDeclareVariable(attributeInfo, info, attribute info, );

rtDeclareVariable(attributeInfo2, infoH, attribute infoH, );
// Context global variables provided by the renderer system.
rtDeclareVariable(rtObject, sysTopObject, , );

// Semantic variables.
rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );
rtDeclareVariable(float, theIntersectionDistance, rtIntersectionDistance, );

rtDeclareVariable(PerRayData, thePrd, rtPayload, );

//type of rendering
rtDeclareVariable(int, Type, , );
rtDeclareVariable(int, HeteroObjType, , );

//array of lights
rtBuffer<BasicLight> lights;

//typedef rtCallableProgramX<float(float3, primParamDesc)> callM;
//rtDeclareVariable(callM, evalF, , );

typedef rtCallableProgramId<float(float3, primParamDesc)> callM;
rtDeclareVariable(callM, evalF, , );
/*
For heterogeneous objects
*/

//TODO: set as buffer
inline __device__ float3 transfer_function(int t, float d)
{
    // return TFBuffer[t];
    switch (t)
    {
    case 1: //H
        return make_float3(1, 1, 1);
        break;
    case 2: //C
        return make_float3(0.4);
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
    case 7: //Si
        return make_float3(1, 1, 1);//255/236, 255/246, 0);
        break;
    }
    return make_float3(0, 0, 0);
}

RT_CALLABLE_PROGRAM  float eval0(float3 x, primParamDesc descPrim)
{
    int type = descPrim.type;
    float f = 10000.0;
    float3 pos1 = descPrim.pos[0];
    float rad1 = descPrim.rad[0];

    f = sdfPrim0(x - pos1, rad1);//length(x - pos1) - rad1; //sdfPrim1(x, pos1, pos2, rad1, rad2);
    return f;
}

RT_CALLABLE_PROGRAM  float eval1(float3 x, primParamDesc descPrim)
{
    int type = descPrim.type;
    float f = 10000.0;
    float3 pos1 = descPrim.pos[0];
    float rad1 = descPrim.rad[0];
    float3 pos2 = descPrim.pos[1];
    float rad2 = descPrim.rad[1];

    f = sdfPrim1(x, pos1, pos2, rad1, rad2);//length(x - pos1) - rad1; //sdfPrim1(x, pos1, pos2, rad1, rad2);
    return f;
}

RT_CALLABLE_PROGRAM  float eval4(float3 x, primParamDesc descPrim)
{
    int type = descPrim.type;
    float f = 10000.0;
    float3 pos1 = descPrim.pos[0];
    float3 pos2 = descPrim.pos[1];
    float3 pos3 = descPrim.pos[2];
    float3 pos4 = descPrim.pos[3];

    f = sdfPrim4(x, pos1, pos2, pos3, pos4);//length(x - pos1) - rad1; //sdfPrim1(x, pos1, pos2, rad1, rad2);
    return f;
}

RT_CALLABLE_PROGRAM  float evalDefault(float3 x, primParamDesc descPrim)
{
    int type = descPrim.type;
    float f = 10000.0;

    f = sdfPrimDefault(x, descPrim);
    return f;
}

RT_CALLABLE_PROGRAM float3 GetColorBlend(float3 x, primParamDesc desc)
{
    float3 pos = desc.pos[0];
    float3 col1 = transfer_function(desc.types[0], 1.0);

    if (desc.type > 0) {
        float3 pos2 = desc.pos[1];
        float3 col2 = transfer_function(desc.types[1], 1.0);
        float r1 = desc.rad[0];
        float r2 = desc.rad[1];

        float d1 = length(x - pos) - r1 / 2;
        float d2 = length(x - pos2) - r2 / 2;
        float d = length(pos - pos2);
        if (d1 <= 0) return col1;
        if (d2 <= 0) return col2;
        float3 col = d1 / d*col2 + d2 / d*col1;

        return col;
    }
    else return col1;
}

inline __device__  float3  GetColor(float3 x)
{
    if (infoH.desc.type < 4) {
        float3 pos = infoH.desc.pos[0];
        float3 col1 = transfer_function(infoH.desc.types[0], 1.0);

        if (infoH.desc.type > 0) {
            float3 pos2 = infoH.desc.pos[1];
            float3 col2 = transfer_function(infoH.desc.types[1], 1.0);
            float r1 = infoH.desc.rad[0];
            float r2 = infoH.desc.rad[1];

            float d1 = length(x - pos) - r1 / 2;
            float d2 = length(x - pos2) - r2 / 2;
            float d = length(pos - pos2);
            if (d1 <= 0) return col1;
            if (d2 <= 0) return col2;
            float3 col = d1 / d*col2 + d2 / d*col1;

            return col;
        }
        else return col1;
    }
    else { //tetra
        float r = infoH.desc.rad[0];
        float4 col = translucent_grays(abs(0.01 / r) / 10, r / 8, 1); //abs(f / 30), 1);////translucent_grays2(color_id, abs(f / 100));
        return make_float3(col.x, col.y, col.z);
    }
}

__device__ void render_HeteroVolume(float3 normal, float3 hit_point)
{
    float Ka = 0.5;
    float Kd = 0.9;
    float Ks = 0.9;

    float4 col = make_float4(0, 0, 0, 1);// translucent_grays(0.5, 0.1, 0);

    float tstep = 0.1;
    float3 pos = hit_point;// eyeRay.o + eyeRay.d*tnear;
    float3 step = theRay.direction*tstep;

    float4 sum = thePrd.result;// make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here
    float trp = 0.05;
    float trp0 = 0.1;

    float s1 = evalF(pos, infoH.desc);
    if (abs(s1) > tstep)
        step = theRay.direction*abs(s1);

    float i = 0.0;
    float max = thePrd.maxDist;// *2 + 0.4; //bounding box size
    float4 sumcol = make_float4(0.0);
    float tracedDist = 0;

    //float4 col1 = translucent_grays(0.5, 0.01, 0);
    int VolInt = 1;

    while (i < max) //s2 < 0.01)
    {
        if (s1 < tstep / 2)
        {
            // if (abs(s1) > tstep) //sum transparency
            {
                //is used to highlight isosurfaces
                //or create a more shell like effect
                //trp = trp0 + abs(s1) / 10;

                //----------------------
                //COLOR COMPUTATION
                //trp = trp0;
                float3 colorm2 = GetColor(pos);
                VolInt = 1;
                //------------------

                float3 color = Ka *  colorm2;// ambient_light_color;
                float3 color2 = Ka *  colorm2;                                          //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;
                if (i < 0.02)//smooth boundary
                {
                    for (int i = 0; i < 2; ++i)
                    {
                        BasicLight light = lights[i];
                        float3 L = optix::normalize(light.pos - thePrd.last_hit_point);
                        float nDl = optix::dot(thePrd.normal, L);

                        //if (nDl > 0)
                        //    color += Kd * nDl * light.color; // make_float3(1.0);//

                        float phong_exp = 0.2;
                        if (nDl > 0) {
                            color += Kd * nDl * light.color;

                            /*  optix::float3 H = optix::normalize(L - theRay.direction);
                              float nDh = optix::dot(normal, H);
                              if (nDh > 0)
                                  color += Ks * light.color * pow(nDh, phong_exp);
                          */
                        }
                    }
                    //---------------

                    col = make_float4(color.x, color.y, color.z, trp*Ka);
                }
                else
                    col = make_float4(color2.x, color2.y, color2.z, trp*Ka);

                if (VolInt > 0) {
                    //Beer–Lambert law
                    float F = exp(-trp*abs(s1) * 200);
                    col = col*(1.0 - F);
                    sum = sum + col*(1.0f - sum.w);
                }
                else //conventional integration
                {
                    /*col.w = trp * 1.9;
                    col.x *= col.w;
                    col.y *= col.w;
                    col.z *= col.w;

                    float t = sum.w;
                    // "over" operator for front-to-back blending
                    sum = sum + col*(1.0f - t);
                    sum.w = t*(1.0 - trp);*/

                    float F = 1.0 - trp*1.9;
                    col = col*(1.0 - F);
                    sum = sum + col*(1.0f - sum.w);
                }
                // tracedDist += abs(s1);
            }
        }

        //s1 = s2;

        if (VolInt > 0) {
            if (abs(s1) > tstep) {
                step = theRay.direction*abs(s1);
                i += abs(s1);
            }
            else
            {
                i += tstep;
                step = theRay.direction*tstep;
            }
        }
        else { //volume sampling
            if (s1 > tstep) //employ space skipping
            {
                step = theRay.direction*abs(s1);
                i += abs(s1);
            }
            else {
                i += tstep;
                step = theRay.direction*tstep;
            }
        }

        pos += step;
        if (sum.w >= 1.0) {
            i = max + 1;
        }
        else
            s1 = evalF(pos, infoH.desc);
    }

    thePrd.result = sum;
}
/* Compiles various types of programs depending on primType*/
RT_PROGRAM void volume_hetero_close()
{
    float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, infoH.normal));
    optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;
    thePrd.maxDist = infoH.maxDist;
    thePrd.normal = normal;
    thePrd.last_hit_point = hit_point;

    float3 col = GetColor(hit_point);
    //thePrd.result += make_float4(col.x, col.y, col.z, 0.1);
    render_HeteroVolume(normal, hit_point); //for defalt rendering primitive

    thePrd.renderType = 0;
    thePrd.depth++;

    if (thePrd.totalDist > 20.0) //TODO set max trace depth as parameter
        thePrd.depth = 10;

    thePrd.totalDist += length(theRay.origin - hit_point);
    if (thePrd.result.w < 0.5) {
        if (thePrd.depth < 5) //TODO set max trace depth as parameter
        {
            //further ray
            optix::Ray ray = optix::make_Ray(hit_point + theRay.direction *(abs(infoH.maxDist) + 0.5), theRay.direction, 0, 0.0f, RT_DEFAULT_MAX);
            rtTrace(sysTopObject, ray, thePrd);
        }
    }
}

RT_PROGRAM void volume_hetero_any()
{
    //No volume integration
    //just saving preliminary data to array
    thePrd.renderType = 3; //for postprocessing
    if (thePrd.cur_prim < MAX_PRIM_ALONG_RAY)
    { //push intersections
        thePrd.cur_prim++;
        cellPrimDesc cell;
        cell.intersectionDist = infoH.tmin;
        cell.type = 3; //tracing with primitives and compute bounds
        cell.normal = infoH.normal;
        int typeC = int(infoH.desc.rad[0]);
        float3 col = GetColor(infoH.hit_point);

        //thePrd.result += make_float4(col.x, col.y, col.z, 0.1);
        cell.color = make_float4(col.x, col.y, col.z, 0.01);
        cell.maxDist = infoH.maxDist;

        thePrd.cellPrimitives[thePrd.cur_prim - 1] = cell;
        thePrd.prims[thePrd.cur_prim - 1] = infoH.desc;
        rtIgnoreIntersection();
    }
    else {
        rtTerminateRay();
    }
}