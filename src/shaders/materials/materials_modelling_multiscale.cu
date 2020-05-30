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

rtDeclareVariable(attributeInfo2, infoH, attribute infoH, );
// Context global variables provided by the renderer system.
rtDeclareVariable(rtObject, sysTopObject, , );

rtDeclareVariable(float, MultiscaleParam, , );
rtDeclareVariable(float, TimeSound, , );
// Semantic variables.
rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );
rtDeclareVariable(float, theIntersectionDistance, rtIntersectionDistance, );

rtDeclareVariable(PerRayData, thePrd, rtPayload, );

//type of rendering
rtDeclareVariable(int, Type, , );
rtDeclareVariable(int, HeteroObjType, , );

//array of lights
rtBuffer<BasicLight> lights;

rtDeclareVariable(float3, sysCameraPosition, , );

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

RT_CALLABLE_PROGRAM  float eval3(float3 x, primParamDesc descPrim)
{
    int type = descPrim.type;
    float f = 10000.0;
    float3 pos1 = descPrim.pos[0];
    float3 pos2 = descPrim.pos[1];
    float3 pos3 = descPrim.pos[2];

    float rad1 = descPrim.rad[0];
    float rad2 = descPrim.rad[1];
    float rad3 = descPrim.rad[2];

    f = sdfPrim3(x, pos1, pos2, pos3, rad1, rad2, rad3);//length(x - pos1) - rad1; //sdfPrim1(x, pos1, pos2, rad1, rad2);
    return f;
}

inline __device__ float4  blendColor(float dMd, float3 x, float3 dir, float3 pos, float3 pos2, float3 pos3, float r1, float r2, float r3, float3 col1, float3 col2, float3 col3)
{
    float dNorm = 10;

    float3 vib_color[3];
    vib_color[0] = make_float3(0, 1.0, 0); //green
    vib_color[1] = make_float3(0, 0.0, 1.0); //blue
    vib_color[2] = make_float3(1, 1, 0); //yellow

    float3 wt_col = make_float3(0);
    dNorm = length(pos - pos2) + length(pos3 - pos2) + length(pos - pos3);
    dNorm /= 3;

    float d1 = (length(x - pos) - r1*(1 + dMd)) / dNorm;
    float d2 = (length(x - pos2) - r2*(1 + dMd)) / dNorm;
    float d3 = (length(x - pos3) - r3*(1 + dMd)) / dNorm;

    d1 = optix::clamp(d1, 0.01, 1.0);
    d2 = optix::clamp(d2, 0.01, 1.0);
    d3 = optix::clamp(d3, 0.01, 1.0);

    float3 color = make_float3(0);
    float tr = abs(dMd - 0.1);
    color += (1 - d1)*col1;
    color += (1 - d2)*col2;
    color += (1 - d3)*col3;
    //color += dMd*make_float3(1);
    //color*=tr;

    //dMd=1 - rep1;
    //dMd=0 -rep2
    //dMd*10

    d1 = (length(x - pos) - r1);
    d2 = (length(x - pos2) - r2);
    d3 = (length(x - pos3) - r3);

    //  d1 = optix::clamp(d1, -0.9, 1.0);
    //  d2 = optix::clamp(d2, -0.9, 1.0);
    //  d3 = optix::clamp(d3, -0.9, 1.0);
    float3 col = make_float3(0);
    if (d1 < 0.1)
        col += abs(d1 / r1)*col1;
    if (d2 < 0.1)
        col += abs(d2 / r2)*col2;
    if (d3 < 0.1)
        col += abs(d3 / r3)*col3;

    //------------
    //interpolate between two models
    float3 coll = make_float3(0);
    d1 = optix::clamp(d1, -0.9, 0.0);
    d2 = optix::clamp(d2, -0.9, 0.0);
    d3 = optix::clamp(d3, -0.9, 0.0);
    coll += abs(1 - d1)*col1*abs(d1);
    coll += abs(1 - d2)*col2*abs(d2);
    coll += abs(1 - d3)*col3*abs(d3);
    //coll ;
    col = dMd*coll + (1 - dMd)*col;

    //col+=dMd*make_float3(1);

    //d1 += 0.1;
    //d2 += 0.1;
    //d3 += 0.1;
    tr = abs(1 - min(min(d1, d2), d3)) / 2;//abs(min(min(d1 / r1, d2 / r2), d3 / r3)) / 2 +
                                           //tr /= 3;
                                           //tr=clamp(tr,0,1);
                                           // tr = (3 - d1 - d2 - d3) / 3;
    return make_float4(col.x, col.y, col.z, tr);
}

RT_CALLABLE_PROGRAM float3 GetColorBlend(float3 x, primParamDesc desc)
{
    //---level of detail
    float dist_cam = length(sysCameraPosition - (desc.pos[0] + desc.pos[1] + desc.pos[2]) / 3);
    float d;
    if (dist_cam < 20.0)
    {
        float interp = (dist_cam - 10) / 10.0;
        d = optix::clamp(interp, 0.0, 1.0);
    }
    else d = 1;

    //MultiscaleParam=d;

    float3 col1 = transfer_function(desc.types[0], d);
    float3 col2 = transfer_function(desc.types[1], d);
    float3 col3 = transfer_function(desc.types[2], d);

    //--------level of detail continue
    float rad1 = desc.rad[0];
    float rad2 = desc.rad[1];
    float rad3 = desc.rad[2];
    float4 color = make_float4(0);
    float3 vib_color[3];
    vib_color[0] = make_float3(0, 1.0, 0); //green
    vib_color[1] = make_float3(0, 0.0, 1.0); //blue
    vib_color[2] = make_float3(1.0, 1.0, 0); //yellow

    float3 col21;
    float3 col31;

    //return mod1*d + (1.0 - d)*mod2;
    float r1 = d*rad1 / 2 + (1 - d)*rad1;
    float r2 = d*rad2 / 2 + (1 - d)*rad2;
    float r3 = d*rad3 / 2 + (1 - d)*rad3;

    float3 weights[3];

    if (d <= 1.0)
    {
        float3 pos = desc.pos[0];
        float3 pos2 = desc.pos[1];
        float3 pos3 = desc.pos[2];
        //first vibration vector in molecule
        float3 vib2[3];
        float3 vib3[3];

        vib2[0] = (pos2 - pos) / 3.5 * -cos(TimeSound * 20)*(1 - d);
        vib3[0] = (pos3 - pos) / 3.5 * (cos(TimeSound * 20))*(1 - d);

        vib2[1] = (pos2 - pos) / 3.5 * sin(TimeSound * 20)*(1 - d);
        vib3[1] = (pos3 - pos) / 3.5 * (sin(TimeSound * 20))*(1 - d);

        float3 vib_dir = pos2 - 2 * pos + pos3;

        vib2[2] = vib_dir / 3.5 * sin(TimeSound * 20)*(1 - d);
        vib3[2] = vib_dir / 3.5 * (sin(TimeSound * 20))*(1 - d);

        float d_min = 0;
        int vib_min = 0;

        for (int i = 0; i < 2; i++)
        {
            float3 pp3 = pos3 + vib3[i];
            float3 pp2 = pos2 + vib2[i];

            col31 = d*col3 + (1 - d)*vib_color[i] * 1.5;
            col21 = d*col2 + (1 - d)*vib_color[i] * 1.5;

            color += blendColor(d, x, theRay.direction, pos, pp2, pp3, r1, r2, r3, col1, col21, col31);
        }
        color;///= 3.0;
    }
    else {
        color = blendColor(d, x, theRay.direction, desc.pos[0], desc.pos[1], desc.pos[2], r1, r2, r3, col1, col2, col3);
    }

    return make_float3(color.x, color.y, color.z);
}

inline __device__  float4  GetColor(float3 x)
{
    //---level of detail
    float dist_cam = length(sysCameraPosition - (infoH.desc.pos[0] + infoH.desc.pos[1] + infoH.desc.pos[2]) / 3);
    float d;
    if (dist_cam < 20.0)
    {
        float interp = (dist_cam - 10) / 10.0;
        d = optix::clamp(interp, 0.0, 1.0);
    }
    else d = 1;

    //MultiscaleParam=d;

    float3 col1 = transfer_function(infoH.desc.types[0], d);
    float3 col2 = transfer_function(infoH.desc.types[1], d);
    float3 col3 = transfer_function(infoH.desc.types[2], d);

    //--------level of detail continue
    float rad1 = infoH.desc.rad[0];
    float rad2 = infoH.desc.rad[1];
    float rad3 = infoH.desc.rad[2];
    float4 color = make_float4(0);
    float3 vib_color[3];
    vib_color[0] = make_float3(0, 1.0, 0); //green
    vib_color[1] = make_float3(0, 0.0, 1.0); //blue
    vib_color[2] = make_float3(1.0, 1.0, 0); //yellow

    float3 col21;
    float3 col31;

    //return mod1*d + (1.0 - d)*mod2;
    float r1 = d*rad1 / 2 + (1 - d)*rad1;
    float r2 = d*rad2 / 2 + (1 - d)*rad2;
    float r3 = d*rad3 / 2 + (1 - d)*rad3;

    float3 weights[3];

    if (d <= 1.0)
    {
        float3 pos = infoH.desc.pos[0];
        float3 pos2 = infoH.desc.pos[1];
        float3 pos3 = infoH.desc.pos[2];
        //first vibration vector in molecule
        float3 vib2[3];
        float3 vib3[3];

        vib2[0] = (pos2 - pos) / 3.5 * -cos(TimeSound * 20)*(1 - d);
        vib3[0] = (pos3 - pos) / 3.5 * (cos(TimeSound * 20))*(1 - d);

        vib2[1] = (pos2 - pos) / 3.5 * sin(TimeSound * 20)*(1 - d);
        vib3[1] = (pos3 - pos) / 3.5 * (sin(TimeSound * 20))*(1 - d);

        float3 vib_dir = pos2 - 2 * pos + pos3;

        vib2[2] = vib_dir / 3.5 * sin(TimeSound * 20)*(1 - d);
        vib3[2] = vib_dir / 3.5 * (sin(TimeSound * 20))*(1 - d);

        float d_min = 0;
        int vib_min = 0;

        for (int i = 0; i < 2; i++)
        {
            float3 pp3 = pos3 + vib3[i];
            float3 pp2 = pos2 + vib2[i];

            col31 = d*col3 + (1 - d)*vib_color[i] * 1.5;
            col21 = d*col2 + (1 - d)*vib_color[i] * 1.5;

            color += blendColor(d, x, theRay.direction, pos, pp2, pp3, r1, r2, r3, col1, col21, col31);
        }
        color;///= 3.0;
    }
    else {
        color = blendColor(d, x, theRay.direction, infoH.desc.pos[0], infoH.desc.pos[1], infoH.desc.pos[2], r1, r2, r3, col1, col2, col3);
    }

    return color;
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
                VolInt = 1;
                //------------------
                                       //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;
                col = GetColor(pos);

                //Beer–Lambert law
                float F = exp(-trp*abs(s1) * 200);
                col = col*(1.0 - F);
                sum = sum + col*(1.0f - sum.w);
            }
        }

        if (abs(s1) > tstep) {
            step = theRay.direction*abs(s1);
            i += abs(s1);
        }
        else
        {
            i += tstep;
            step = theRay.direction*tstep;
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

    thePrd.result = GetColor(hit_point);
    // render_HeteroVolume(normal, hit_point); //for defalt rendering primitive

    thePrd.renderType = 0;
    thePrd.depth++;

    if (thePrd.totalDist > 20.0) {//TODO set max trace depth as parameter
        thePrd.depth = 10;
    }

    thePrd.totalDist += length(theRay.origin - hit_point);
    /*if (thePrd.result.w < 0.5) {
        if (thePrd.depth < 5) //TODO set max trace depth as parameter
        {
            //further ray
            optix::Ray ray = optix::make_Ray(hit_point + theRay.direction *(abs(infoH.maxDist) + 0.1), theRay.direction, 0, 0.0f, RT_DEFAULT_MAX);
            rtTrace(sysTopObject, ray, thePrd);
        }
    }*/
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
        //float3 col = GetColor(infoH.hit_point);

        //thePrd.result += make_float4(col.x, col.y, col.z, 0.1);
        cell.color = GetColor(infoH.hit_point);
        cell.maxDist = infoH.maxDist;

        thePrd.cellPrimitives[thePrd.cur_prim - 1] = cell;
        thePrd.prims[thePrd.cur_prim - 1] = infoH.desc;
        rtIgnoreIntersection();
    }
    else {
        rtTerminateRay();
    }
}