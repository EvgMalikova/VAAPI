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

rtDeclareVariable(attributeInfo2, infoH, attribute infoH, );
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

//typedef rtCallableProgramId<float(float3, float3)> callT;
rtDeclareVariable(callT, sdfPrimBack, , );

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
    float4 sum = make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here

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

    thePrd.result = sum;
    // thePrd.radiance = make_float3(sum); //+thePrd.radiance*0.8f;
}
__device__ void render_Surface2(float3 normal, float3 hit_point)
{
    float Ka = 0.5;
    float Kd = 0.5;
    float Ks = 0.2;
    float4 col = make_float4(0, 1, 0, 1);// translucent_grays(0.5, 0.1, 0);

    float tstep = 0.01;
    float3 pos = hit_point;// eyeRay.o + eyeRay.d*tnear;
    float3 step = theRay.direction*tstep;

    float4 sum = make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here
    BasicLight lights2[2];
    lights2[0].color = optix::make_float3(1.0);
    lights2[0].pos = optix::make_float3(10.0);

    lights2[1].color = optix::make_float3(1.0);
    lights2[1].pos = optix::make_float3(0, 0, 10.0);

    float s1 = sdfPrim(pos, make_float3(thePrd.maxDist));
    pos += step;
    float s2 = sdfPrim(pos, make_float3(thePrd.maxDist));
    float i = 0;
    float trp = 0.01;
    float max = thePrd.maxDist * 2 + 0.4; //bounding box size
    while (i < max) //s2 < 0.01)
    {
        if (s2 < 0.01) {
            //col *= Ka;
           //col.w = 0.5; //s1 is very small
            float3 color = Ka *  make_float3(col);// ambient_light_color;

                                                  //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

            for (int i = 0; i < 2; ++i)
            {
                BasicLight light = lights2[i];
                float3 L = optix::normalize(light.pos - hit_point);
                float nDl = optix::dot(thePrd.normal, L);

                //if (nDl > 0)
                //    color += Kd * nDl * light.color; // make_float3(1.0);//

                float phong_exp = 0.1;
                if (nDl > 0) {
                    color += Kd * nDl * light.color;

                    /*  optix::float3 H = optix::normalize(L - theRay.direction);
                      float nDh = optix::dot(normal, H);
                      if (nDh > 0)
                          color += Ks * light.color * pow(nDh, phong_exp);
                  */
                }
            }
            //initial blend
            col = make_float4(color);
            col.w = trp;
            col.x *= col.w;
            col.y *= col.w;
            col.z *= col.w;
            // "over" operator for front-to-back blending
            sum = sum + col*(1.0f - sum.w);
        }
        i += tstep;
        s1 = s2;
        pos += step;

        if (sum.w >= 1.0) i = max + 1;
        else
            s2 = sdfPrim(pos, make_float3(thePrd.maxDist)); //interpolateSDF(time, pos, texSDF, texSDF_F);
           // if (abs(s1) <= eps)
    }
    //or it should be
    // thePrd.radiance = make_float3(sum);
    thePrd.radiance += make_float3(sum);
}
__device__ float fract(float x)
{
    return x - floor(x);
}
/* discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3 */
/*__device__  float3 random3(float3 c) {
    float j = 4096.0*sin(dot(c, make_float3(17.0, 59.4, 15.0)));
    float3 r;
    r.z = fract(512.0*j);
    j *= 0.125;
    r.x = fract(512.0*j);
    j *= 0.125;
    r.y = fract(512.0*j);
    return r - 0.5;
}
*/
__device__ float noise3D(float3 p)
{
    return fract(sin(dot(p, make_float3(12.9898, 78.233, 126.7235))) * 43758.5453);
}

__device__ float evalNoise(float3 p)
{
    float r = 1.0;
    float3 f = floor(p);
    float3 x = make_float3(fract(p.x), fract(p.y), fract(p.z));
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            for (int k = -1; k <= 1; k++)
            {
                float3 q = make_float3(float(i), float(j), float(k));
                float3 v = q + make_float3(noise3D((q + f)*1.11), noise3D((q + f)*1.14), noise3D((q + f)*1.17)) - x;
                float d = dot(v, v);
                r = fminf(r, d);
            }
        }
    }
    return sqrt(r);
}
__device__ float ComputeSDFCol(float3 pos)
{
    float radc = 5.0;
    float scale = 1.8;
    float3 cent1 = make_float3(radc, 0, 0);
    float3 cent2 = make_float3(-radc, 0, 0);
    float3 cent3 = make_float3(0, radc, 0);
    float3 cent4 = make_float3(0, -radc, 0);
    float d1 = length(pos - cent1) - radc / scale;
    float d2 = length(pos - cent2) - radc / scale;
    float d3 = length(pos - cent3) - radc / scale;
    float d4 = length(pos - cent4) - radc / scale;

    float d5 = min(d1, d2);
    d5 = min(d5, d3);
    d5 = min(d5, d4);
    d5 -= scale;
    d5 = -d5;

    float f = fmaxf(d5, d4);
    f = fmaxf(f, d3);
    f = fmaxf(f, d2);
    f = fmaxf(f, d1);

    return f;
}
__device__ float3 ComputeColor(float3 pos, int& VolInt, float&trp)
{
    //Compute color function
    //----------------
    float radc = 5.0;
    float scale = 1.8;
    float3 cent1 = make_float3(radc, 0, 0);
    float3 cent2 = make_float3(-radc, 0, 0);
    float3 cent3 = make_float3(0, radc, 0);
    float3 cent4 = make_float3(0, -radc, 0);
    float d1 = length(pos - cent1) - radc / scale;
    float d2 = length(pos - cent2) - radc / scale;
    float d3 = length(pos - cent3) - radc / scale;
    float d4 = length(pos - cent4) - radc / scale;

    float d5 = min(d1, d2);
    d5 = min(d5, d3);
    d5 = min(d5, d4);
    d5 -= scale * 1.5;
    d5 = -d5;

    float3 col1 = make_float3(1, 0, 0);
    float3 col2 = make_float3(0, 0, 1);
    float3 col3 = make_float3(1, 1, 0);
    float3 col4 = make_float3(0, 1, 1);
    float3 col5 = make_float3(0, 1, 0);
    //------------
    //interpolation with noise
   /* VolInt = 1;//ray-segment

                   //f = worley3D(vec3(p*0.25, 1.0)*f);;
                   //if (d4 < scale)
    {
        //VolInt = 0; //vol sampling for this region
        float f = 0;
        if (d4 <= 0) {
            f = evalNoise(pos);
            float3 p2 = pos*0.25;
            f = evalNoise(p2)*f;
            VolInt = 0;
        }
        else {
            if (d4 <= scale) {
                float3 p = pos - d4*thePrd.normal;
                f = evalNoise(p);
                float3 p2 = pos*0.25;
                f = evalNoise(p2)*f;
                VolInt = 0;
            }
            else f = 1 / 5;
            //trp *= f;
        }
        //trp += f;
        col4 = make_float3(5 * f);
        col4 *= make_float3(0.0, 1.0, 1.0)*f;
        //col4 *= expf(1.0f - col4);
    }
    */

    //-------------
    float3 colorm2;
    /* if (d1 <= scale) return col1;
     if (d2 <= scale) return col2;
     if (d3 <= scale) return col3;
     if (d4 <= scale) return col4;
     if (d5 <= scale) return col5;*/

    float d_l = abs(d1) + abs(d2);
    colorm2 = (d1 / d_l)*col2 + (d2 / d_l)*col1;
    d_l = abs(d3) + abs(d4);
    colorm2 += (d3 / d_l)*col4 + (d4 / d_l)*col3;
    d_l = abs(d1) + abs(d3);
    colorm2 += (d3 / d_l)*col1 + (d1 / d_l)*col3;
    d_l = abs(d3) + abs(d2);
    colorm2 += (d3 / d_l)*col2 + (d2 / d_l)*col3;
    d_l = abs(d1) + abs(d4);
    colorm2 += (d1 / d_l)*col4 + (d4 / d_l)*col1;
    d_l = abs(d2) + abs(d4);
    colorm2 += (d2 / d_l)*col4 + (d4 / d_l)*col2;

    //-----
    d_l = abs(d2) + abs(d5);
    colorm2 += (d2 / d_l)*col5 + (d5 / d_l)*col2;
    d_l = abs(d1) + abs(d5);
    colorm2 += (d1 / d_l)*col5 + (d5 / d_l)*col1;
    d_l = abs(d3) + abs(d5);
    colorm2 += (d3 / d_l)*col5 + (d5 / d_l)*col3;
    d_l = abs(d4) + abs(d5);
    colorm2 += (d4 / d_l)*col5 + (d5 / d_l)*col4;

    colorm2 /= 10.0;
    return colorm2;
}
__device__ void render_Surface3(float3 normal, float3 hit_point)
{
    float Ka = 0.5;
    float Kd = 0.9;
    float Ks = 0.9;
    float4 col = make_float4(0, 0, 0, 1);// translucent_grays(0.5, 0.1, 0);

    BasicLight lights2[2];
    lights2[0].color = optix::make_float3(1.0);
    lights2[0].pos = optix::make_float3(10.0);

    lights2[1].color = optix::make_float3(1.0);
    lights2[1].pos = optix::make_float3(0, 0, 10.0);

    float tstep = 0.01;
    float3 pos = hit_point;// eyeRay.o + eyeRay.d*tnear;
    float3 step = theRay.direction*tstep;

    float4 sum = thePrd.result;// make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here

    float trp = 0.01;
    float trp0 = 0.02;
    float s1 = sdfPrimBack(pos, make_float3(1.1));
    if (abs(s1) > tstep)
        step = theRay.direction*abs(s1);

    //pos += step;
    //float s2 = sdfPrimBack(pos, make_float3(1.1));
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
                float3 colorm2 = ComputeColor(pos, VolInt, trp);
                VolInt = 1;
                //------------------

                float3 color = Ka *  colorm2;// ambient_light_color;
                float3 color2 = Ka *  colorm2;                                          //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;
                if (i < 0.02)//smooth boundary
                {
                    for (int i = 0; i < 2; ++i)
                    {
                        BasicLight light = lights2[i];
                        float3 L = optix::normalize(light.pos - thePrd.last_hit_point);
                        float nDl = optix::dot(thePrd.normal, L);

                        //if (nDl > 0)
                        //    color += Kd * nDl * light.color; // make_float3(1.0);//

                        float phong_exp = 0.2;
                        if (nDl > 0) {
                            color += Kd * nDl * light.color;

                            optix::float3 H = optix::normalize(L - theRay.direction);
                            float nDh = optix::dot(normal, H);
                            if (nDh > 0)
                                color += Ks * light.color * pow(nDh, phong_exp);
                        }
                    }
                    //---------------

                    col = make_float4(color.x, color.y, color.z, trp*Ka);
                }
                else
                    col = make_float4(color2.x, color2.y, color2.z, trp*Ka);

                if (VolInt > 0) {
                    //Beerâ€“Lambert law
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
            s1 = sdfPrimBack(pos, make_float3(thePrd.maxDist)); //interpolateSDF(time, pos, texSDF, texSDF_F);

                                                                //s2 = sdfPrimBack(pos, make_float3(1.1)); //interpolateSDF(time, pos, texSDF, texSDF_F);
                                                                // if (abs(s1) <= eps)
    }
    thePrd.result = sum;
}
__device__ void render_Surface4(float3 normal, float3 hit_point)
{
    float Ka = 0.5;
    float Kd = 0.5;
    float Ks = 0.2;

    float tstep = 0.01;
    float3 pos = hit_point;// eyeRay.o + eyeRay.d*tnear;
    float3 step = theRay.direction*tstep;

    float4 sum = make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here

    float trp = 0.1;
    float s1 = sdfPrim(pos, make_float3(1.1));
    if (abs(s1) > tstep)
        step = theRay.direction*abs(s1);

    pos += step;
    float s2 = sdfPrim(pos, make_float3(1.1));
    float i = 0;
    float max = 2.2 + 0.4; //bounding box size
    float4 sumcol = make_float4(0.0);
    float tracedDist = 0;

    float4 col1 = translucent_grays(0.5, 0.01, 0);
    float3 color = Ka *  make_float3(col1);// ambient_light_color;

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

            optix::float3 H = optix::normalize(L - theRay.direction);
            float nDh = optix::dot(normal, H);
            if (nDh > 0)
                color += Ks * light.color * pow(nDh, phong_exp);
        }
    }

    while (i < max) //s2 < 0.01)
    {
        if (s2 < 0.0) {
            if (abs(s1) > tstep) //sum transparency
            {
                /*//col *= Ka;
                //col.w = 0.5; //s1 is very small

                //initial blend
               // float4 col = make_float4(color);

                //   col.w = trp*abs(s1) * 10;
                //Beer–Lambert law
                col.w = exp(-(trp)*abs(s1) * 1000);

                col.x *= col.w;
                col.y *= col.w;
                col.z *= col.w;

                //col = col*col.w;

               // "over" operator for front-to-back blending
                sum = sum + col*(1.0f - sum.w);*/
                tracedDist += abs(s1);
            }
        }
        else {
            float4 col = make_float4(color);
            col.w = exp(-trp*tracedDist * 100);

            col.x *= 1 - col.w;
            col.y *= 1 - col.w;
            col.z *= 1 - col.w;

            //col = col*col.w;

            // "over" operator for front-to-back blending
            sum = sum + col*(1.0f - sum.w);
            tracedDist = 0;
        }
        i += abs(s1);
        s1 = s2;
        if (abs(s1) > tstep)
            step = theRay.direction*abs(s1);

        pos += step;
        if (sum.w >= 1.0) i = max + 1;
        else
            s2 = sdfPrimBack(pos, make_float3(thePrd.maxDist)); //interpolateSDF(time, pos, texSDF, texSDF_F);

        //s2 = sdfPrim(pos, make_float3(1.1)); //interpolateSDF(time, pos, texSDF, texSDF_F);
                                             // if (abs(s1) <= eps)
    }
    thePrd.radiance = make_float3(sum);
}

__device__ void render_Surface21(float3 inp_normal, float3 inp_p)
{
    float Ka = 0.5;
    float Kd = 0.5;
    float Ks = 0.2;
    float4 col = make_float4(0, 0, 0, 1);// translucent_grays(0.5, 0.1, 0);

    float tstep = 0.01;
    float3 pos = thePrd.last_hit_point;// eyeRay.o + eyeRay.d*tnear;
    float3 step = theRay.direction*tstep;

    float4 sum = thePrd.result;// make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here
    BasicLight lights2[2];
    lights2[0].color = optix::make_float3(1.0);
    lights2[0].pos = optix::make_float3(10.0);

    lights2[1].color = optix::make_float3(1.0);
    lights2[1].pos = optix::make_float3(0, 0, 10.0);

    float s1 = sdfPrimBack(pos, make_float3(thePrd.maxDist));
    pos += step;
    float s2 = sdfPrimBack(pos, make_float3(thePrd.maxDist));
    float i = 0;
    float trp = 0.01;
    float max = thePrd.maxDist * 2 + 0.4; //bounding box size
    float3 color = Ka *  make_float3(0, 1, 0);// ambient_light_color;
    float3 color2 = Ka *  make_float3(0, 1, 0);                                          //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

    for (int i = 0; i < 2; ++i)
    {
        BasicLight light = lights2[i];
        float3 L = optix::normalize(light.pos - thePrd.last_hit_point);
        float nDl = optix::dot(thePrd.normal, L);

        //if (nDl > 0)
        //    color += Kd * nDl * light.color; // make_float3(1.0);//

        float phong_exp = 0.1;
        if (nDl > 0) {
            color += Kd * nDl * light.color;
        }
    }

    while (i < max) //s2 < 0.01)
    {
        if (s2 < 0.01) {
            //col *= Ka;
            //col.w = 0.5; //s1 is very small

            //initial blend
            if (col.x == 0)
                col = make_float4(color);
            else
                col = make_float4(color2);
            col.w = trp;
            col.x *= col.w;
            col.y *= col.w;
            col.z *= col.w;

            float t = sum.w;
            // "over" operator for front-to-back blending
            sum = sum + col*(1.0f - t);
            sum.w = t*(1.0 - trp);
        }
        i += tstep;
        s1 = s2;
        pos += step;

        if (sum.w >= 1.0) i = max + 1;
        else
            s2 = sdfPrimBack(pos, make_float3(thePrd.maxDist)); //interpolateSDF(time, pos, texSDF, texSDF_F);
                                                            // if (abs(s1) <= eps)
    }
    //or it should be
    // thePrd.radiance = make_float3(sum);
   // thePrd.radiance += make_float3(sum);
    thePrd.result = sum;
}

RT_PROGRAM void anyhitvolume_sdf()
{
    float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));
    optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;
    thePrd.maxDist = info.maxDist;
    thePrd.normal = normal;
    thePrd.last_hit_point = hit_point;
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
    /*case 2: //volume
    {
        render_Transp(normal, hit_point);
        break;
    }*/
    }
    // For comparison
   //render_Surface21(info.normal, info.hit_point);
//    render_Surface3(info.normal, info.hit_point);
    //render_Volume(normal, hit_point); //for leap motion - don't remember
}

RT_PROGRAM void volumehit_complex_sdf()
{
    float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));
    optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;
    //thePrd.normal = normal;
    //thePrd.last_hit_point = hit_point;
    //thePrd.maxDist = info.maxDist;
    /*
           thePrd.last_hit_point = hit_point - theRay.direction*0.4;
        thePrd.wo = theRay.direction;
        thePrd.maxDist = info.maxDist;

        //render_Surface21(info.normal, info.hit_point);
        rtIgnoreIntersection();

        thePrd.renderType = 2;
        //size compared to other rays
        optix::Ray ray = optix::make_Ray(hit_point + theRay.direction *1.2, theRay.direction, 0, 0.0f, RT_DEFAULT_MAX);
        rtTrace(sysTopObject, ray, thePrd);
    */

    if (thePrd.cur_prim < MAX_PRIM_ALONG_RAY)
    { //push intersections
        thePrd.cur_prim++;
        cellPrimDesc cell;
        cell.intersectionDist = theIntersectionDistance;
        cell.type = 0; //main
        cell.normal = normal;
        cell.color = make_float4(0, 1, 0, 0.005);
        cell.maxDist = info.maxDist;
        thePrd.cellPrimitives[thePrd.cur_prim - 1] = cell;

        //check this one after as it may lead to error in any hit
        thePrd.renderType = 2;
        rtIgnoreIntersection();
    }
    else {
        rtTerminateRay();
    }
}