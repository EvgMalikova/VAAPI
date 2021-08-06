/*

 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "../renderer/per_ray_data.h"
#include "../basic_lights.h"
#include "transferFunction.h"
 //#include "per_ray_data.h"
#include "../attributeInfo.h"
using namespace optix;

rtDeclareVariable(attributeInfo, info, attribute info, );
// Context global variables provided by the renderer system.
rtDeclareVariable(rtObject, sysTopObject, , );

// Semantic variables.
rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );
rtDeclareVariable(float, theIntersectionDistance, rtIntersectionDistance, );

rtDeclareVariable(PerRayData, thePrd, rtPayload, );

//rtDeclareVariable(optix::float3, varNormal,    attribute NORMAL, );
//rtDeclareVariable(optix::float3, varHit, attribute hit_point, );

//-----------
//for textures

rtTextureSampler<float, 3> tex0;
rtTextureSampler<float, 3> tex1;
rtTextureSampler<float, 3> tex2;

rtDeclareVariable(int, numTexDefined, , );

//
//for sdf
typedef rtCallableProgramId<float(float3, float3)> callT;
rtDeclareVariable(callT, sdfPrim, , );

//type of rendering
rtDeclareVariable(int, Type, , );

//array of lights
rtBuffer<BasicLight> lights;

//rtDeclareVariable(optix::float3, varTexCoord,  attribute TEXCOORD, );

// This closest hit program only uses the geometric normal and the shading normal attributes.
// OptiX will remove all code from the intersection programs for unused attributes automatically.

// Note that the matching between attribute outputs from the intersection program and
// the inputs in the closesthit and anyhit programs is done with the type (here float3) and
// the user defined attribute semantic (e.g. here NORMAL).
// The actual variable name doesn't need to match but it's recommended for clarity.
__device__ float computeVal(float3 p)
{
    float s1 = 0;

    switch (numTexDefined) {
    case 1:
    {
        s1 = tex3D<float>(tex0, p.x*0.5f + 0.5f, p.y*0.5f + 0.5f, p.z*0.5f + 0.5f);
        break;
    }
    case 2:
    {
        float s_min = tex3D(tex0, p.x*0.5f + 0.5f, p.y*0.5f + 0.5f, p.z*0.5f + 0.5f);
        float s_max = tex3D(tex1, p.x*0.5f + 0.5f, p.y*0.5f + 0.5f, p.z*0.5f + 0.5f);

        s1 = thePrd.TimeSound*s_max + (1.0 - thePrd.TimeSound)*s_min;//trace back to iso value and shift

                                                                     // return s1;
        break;
    }
    }
    return s1;
}

__device__ void render_Surface(float3 normal, float3 hit_point)
{
    float Ka = 0.5;
    float Kd = 0.5;
    float Ks = 0.2;

    float s1 = computeVal(hit_point) / 6;

    float4 col = translucent_grays(0.5, s1, 0);
    float3 color = Ka *  make_float3(col);// ambient_light_color;

//	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

    for (int i = 0; i < lights.size(); ++i)
    {
        BasicLight light = lights[i];
        float3 L = optix::normalize(light.pos - hit_point);
        float nDl = optix::dot(normal, L);

        //if (nDl > 0)
        //    color += Kd * nDl * light.color; // make_float3(1.0);//

        float phong_exp = 0.1;
        if (nDl > 0) {
            color += Kd * nDl * light.color;

            /* optix::float3 H = optix::normalize(L - theRay.direction);
            float nDh = optix::dot(normal, H);
            if (nDh > 0)
            color += Ks * light.color * pow(nDh, phong_exp);*/
        }
    }

    thePrd.radiance = color;
}

__device__ void render_Volume(float3 normal, float3 hit_point)
{
    float tstep = 0.01;
    float3 pos = hit_point;// eyeRay.o + eyeRay.d*tnear;
    float3 step = theRay.direction*tstep;
    float t = theIntersectionDistance;

    //GetSDF function Value
    int jj = 0;
    float eps = 0.01;
    float4 sum = make_float4(0.2);

    bool stop = false;
    float s1 = sdfPrim(pos, make_float3(0.8));// interpolateSDF(time, pos, texSDF, texSDF_F);

    render_Surface(normal, hit_point);
    // blend
    sum = sum + make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.01)*(1 - sum.w);
    while (!stop)//we are inside the object

    {
        float sample = computeVal(pos) / 6;

        float4 col = translucent_grays(0.1 + s1 / 6, sample, 0);

        //kompute koef k_d
        int isoNum = 4;

        //isosurf highlightment
        //TODO: raymarch transmit;
        /*float rayMarch = 6.0;
        float koef =  (rayMarch - s1) / rayMarch;
        float k_d = fabsf(sinf(koef*3.14f*isoNum));
         k_d *= powf(2.6, koef);
         col.w *= (k_d + 0.2);*/

         // pre-multiply alpha
        col.x *= col.w;
        col.y *= col.w;
        col.z *= col.w;
        // "over" operator for front-to-back blending
        sum = sum + col*(1.0f - sum.w);

        //float op = sum.w;
        //isoNum = 2;

        //float4 currColor = col*(1.0f - sum.w);
        //computation of Blinn-Phong
        //if (k_d >= 0.7) {
        //computeColor(sum, pos, eyeRay, rayMarch, time, s1, currColor, k_d, texSDF, texSDF_F);

        //}

        // exit early if opaque
        if (sum.w > 1.1)
            stop = true;
        //	break;

        t += tstep;

        //get bounding box of the primitive here
        if (t > 20) stop = true;

        pos += step;
        s1 = sdfPrim(pos, make_float3(0.5)); //interpolateSDF(time, pos, texSDF, texSDF_F);
        if (s1 <= eps) stop = true;

        if (s1 > 2.0) {
            render_Surface(normal, hit_point);
            // blend
            sum = sum + make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.4)*(1.0f - sum.w);
        }
    }

    //float3 hitP = eyeRay.o;
    //hitP += eyeRay.d*tnear;
    //float3 cl = make_float3(sum.x, sum.y, sum.z);
    //float faceN =
    // computeColor(sum, hitP, eyeRay, rayMarch, time, rayMarch, sum, 2.01, texSDF, texSDF_F);

    thePrd.radiance = make_float3(sum); //+thePrd.radiance*0.8f;
}

RT_PROGRAM void anyhitvolume_sdf()
{
    float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));
    optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

    switch (Type)
    {
    case 0: //surface
    {
        render_Surface(normal, hit_point);
        break;
    }
    case 1: //volume
    {
        render_Volume(normal, hit_point);
        break;
    }
    }
}