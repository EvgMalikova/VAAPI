/*
*/

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "../renderer/per_ray_data.h"
#include "../basic_lights.h"
#include "../attributeInfo.h"
#include "../renderer/rt_function.h"
#include "../renderer/random_number_generators.h"
//#include "per_ray_data.h"

//rtBuffer<float3> TFBuffer;

// Context global variables provided by the renderer system.
rtDeclareVariable(rtObject, sysTopObject, , );
rtDeclareVariable(float, MultiscaleParam, , );
// Semantic variables.
rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );
rtDeclareVariable(float, theIntersectionDistance, rtIntersectionDistance, );

rtDeclareVariable(PerRayData, thePrd, rtPayload, );

// Attributes.

//for sdf
rtDeclareVariable(float3, ambient_light_color, , );
rtDeclareVariable(attributeInfo, info, attribute info, );
rtDeclareVariable(int, useScalar, , );

//for triangulated objects
//TODO:probably now it is broken
rtDeclareVariable(float3, varGeoNormal, attribute GeoNormal, );

//array of lights
rtBuffer<BasicLight> lights;

typedef rtCallableProgramId<float3(int)> callTF;
rtDeclareVariable(callTF, tFunction, , );

typedef rtCallableProgramId<float(float3, float3)> callTBackSDF;
rtDeclareVariable(callTBackSDF, sdfPrimBack, , );
typedef rtCallableProgramId<float(float3, float3, float, float)> callT;
rtDeclareVariable(callT, sdfPrim2, , );

typedef rtCallableProgramId<float(float3, float3, float3, float, float)> callT4;
rtDeclareVariable(callT4, sdfPrim4, , );

rtDeclareVariable(float, sysSceneEpsilon, , );
//rtDeclareVariable(optix::float3, varTexCoord,  attribute TEXCOORD, );

// This closest hit program only uses the geometric normal and the shading normal attributes.
// OptiX will remove all code from the intersection programs for unused attributes automatically.

// Note that the matching between attribute outputs from the intersection program and
// the inputs in the closesthit and anyhit programs is done with the type (here float3) and
// the user defined attribute semantic (e.g. here NORMAL).
// The actual variable name doesn't need to match but it's recommended for clarity.

// Helper functions for sampling a cosine weighted hemisphere distrobution as needed for the Lambert shading model.

//TODO: transmit as callable functions

RT_FUNCTION void render_Surface3(float3 normal, float3 hit_point, float4 color_inp, float max, float3 a, float3 b, float r1, float r2)
{
    float Ka = 0.2;
    float Kd = 0.5;
    float Ks = 0.2;
    float4 col = make_float4(0, 0, 0, 1);// translucent_grays(0.5, 0.1, 0);

    float tstep = 0.005;
    float3 pos = hit_point;// eyeRay.o + eyeRay.d*tnear;
    float3 step = theRay.direction*tstep;

    float4 sum = thePrd.result;// make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here

    if (sum.w >= 1.0)
    {
        return;
    }

    float trp = 0.1;
    float s1 = sdfPrim4(pos, a, b, r1, r2);
    if (abs(s1) > tstep)
        step = theRay.direction*abs(s1);

    pos += step;
    float s2 = sdfPrim4(pos, a, b, r1, r2);
    float i = 0;
    //bounding box size
    float4 sumcol = make_float4(0.0);
    float tracedDist = 0;

    //float4 col1 = translucent_grays(0.5, 0.01, 0);

    float3 color = Ka *  make_float3(color_inp.x, color_inp.y, color_inp.z);// ambient_light_color;
    float3 color2 = color;                                          //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

    for (int i = 0; i < lights.size(); ++i)
    {
        BasicLight light = lights[i];
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
        if (s2 < 0.0) {
            if (abs(s1) > tstep) //sum transparency
            {
                //col *= Ka;
                //col.w = 0.5; //s1 is very small
                // float3 color = Ka *  make_float3(0, 1, 0);// ambient_light_color;

                //initial blend
                if (i == 0)
                    // if (ll < 5) {
                    trp = 0.001 + abs(s1) / 10;
                //     ll++;
                // }
                // else {
                //     trp = 0.001;
                //     ll = 0;
                // }
                if (i == 0)
                    col = make_float4(color.x, color.y, color.z, trp*Ka);

                else
                    col = make_float4(color2.x, color2.y, color2.z, trp*Ka);
                //else
               // col = make_float4(color2.x, color2.y, color2.z, trp*Ka);
                //initial blend
                // float4 col = make_float4(color);
                /*  float f = exp(-trp*abs(s1) * 0.01);
                float T = sum.w;
                sum = sum + sum.w*col*(1.0f - f);
                sum.w = T*f;*/
                /*col.w = trp*abs(s1) * 10;
                col.x *= col.w;
                col.y *= col.w;
                col.z *= col.w;*/

                // "over" operator for front-to-back blending

                //Beer–Lambert law
                float F = exp(-trp*abs(s1) * 200);
                col = col*(1.0 - F);
                sum = sum + col*(1.0f - sum.w);

                tracedDist += abs(s1);
            }
            else {
                //Beer–Lambert law
                //col = make_float4(color);
                // col = col*exp(-trp*abs(s1) / 100);
                tracedDist = 0;
            }
        }

        s1 = s2;
        if (abs(s1) > tstep) {
            step = theRay.direction*abs(s1);
            i += abs(s1);
        }
        else
        {
            step = theRay.direction*abs(tstep);
            i += abs(tstep);
        }
        pos += step;
        if (sum.w >= 1.0) i = max + 1;
        else
            s2 = sdfPrim4(pos, a, b, r1, r2); //interpolateSDF(time, pos, texSDF, texSDF_F);

                                                                //s2 = sdfPrimBack(pos, make_float3(1.1)); //interpolateSDF(time, pos, texSDF, texSDF_F);
                                                                // if (abs(s1) <= eps)
    }
    thePrd.result = sum;
}

RT_FUNCTION void alignVector(float3 const& axis, float3& w)
{
    // Align w with axis.
    const float s = copysign(1.0f, axis.z);
    w.z *= s;
    const float3 h = make_float3(axis.x, axis.y, axis.z + s);
    const float  k = optix::dot(w, h) / (1.0f + fabsf(axis.z));
    w = k * h - w;
}

RT_FUNCTION void unitSquareToCosineHemisphere(const float2 sample, float3 const& axis, float3& w, float& pdf)
{
    // Choose a point on the local hemisphere coordinates about +z.
    const float theta = 2.0f * M_PIf * sample.x;
    const float r = sqrtf(sample.y);
    w.x = r * cosf(theta);
    w.y = r * sinf(theta);
    w.z = 1.0f - w.x * w.x - w.y * w.y;
    w.z = (0.0f < w.z) ? sqrtf(w.z) : 0.0f;

    pdf = w.z * M_1_PIf;

    // Align with axis.
    alignVector(axis, w);

    //mix with normal for self-occlusion
    const float rad = 1.0 - 1.0 / 32;
    w = optix::normalize(axis + w*rad);
}

/*vaBasicMaterial program*/
RT_PROGRAM void closesthit_sdf()
{
    float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));

    float Ka = 0.3;
    float Kd = 0.2;
    float Ks = 0.2;
    //todo implement primInfo type to color and a switch for mapping to color
    float3 col = Ka *  ambient_light_color;
    float4 color = make_float4(col.x, col.y, col.z, 1.0);

    if (useScalar == 1) { //otherwise primInfo is not assigned a value
        if (info.type == 0)
        {
            color = info.useScalar;
        }
        else {
            int t = info.type;// __float_as_int(info.primInfo.y);
            col = tFunction(t);// transfer_function(t);
            color = make_float4(col.x, col.y, col.z, 1.0);
        }
        color *= Ka;
    }/**/
    //if(useScalar==2)
    //{
    // color = info.color;// transfer_function(t);
   //  color *= Ka;
    //}

    optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

    for (int i = 0; i < lights.size(); ++i)
    {
        BasicLight light = lights[i];
        float3 L = optix::normalize(light.pos - hit_point);// varHit);
        float nDl = optix::dot(normal, L);

        //if (nDl > 0)
        //    color += Kd * nDl * light.color; // make_float3(1.0);//
        float4 lcol = make_float4(light.color.x, light.color.y, light.color.z, 1.0);
        float phong_exp = 0.5 *(1 - MultiscaleParam);
        if (nDl > 0) {
            color += Kd * nDl * lcol;
        }
        /* float3 H = optix::normalize(L - thePrd.wi);
         float nDh = optix::dot(normal, H);
         if (nDh > 0)
             color += Ks*(1 - MultiscaleParam) * light.color * pow(nDh, phong_exp);
     */
    }

    //blending of color

    thePrd.result = thePrd.result + color*(1.0f - thePrd.result.w);
    //thePrd.radiance = make_float3(color);
}

RT_PROGRAM void volumehit_sdf2()
{
    if (thePrd.cur_prim < MAX_PRIM_ALONG_RAY || thePrd.result.w >= 1.0)
    {
        //compute normal and color
        float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));
        float Ka = 0.2;

        //todo implement primInfo type to color and a switch for mapping to color
        float3 col = Ka *  ambient_light_color;
        float4 color = make_float4(col.x, col.y, col.z, 1.0);

        if (useScalar == 1) { //otherwise primInfo is not assigned a value
            if (info.type == 0)
            {
                color = info.useScalar;
            }
            else {
                int t = info.type;// __float_as_int(info.primInfo.y);
                col = tFunction(t);// transfer_function(t);
                color = make_float4(col.x, col.y, col.z, 1.0);
            }
            color *= Ka;
        }

        //todo:
        //avarage distance
        //avarage color for blending

        optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

        render_Surface3(normal, hit_point, color, info.maxDist, info.pos[0], info.pos[1], info.rad[0], info.rad[1]);
        // thePrd.result += make_float4(0.1, 0, 0, 0.1);

       //thePrd.renderType = 1;

       //if (thePrd.cur_prim < MAX_PRIM_ALONG_RAY)
        { //push intersections
            thePrd.cur_prim++;
            // thePrd.primitives[thePrd.cur_prim - 1] = make_float2(info.type, theIntersectionDistance);

             //fill second array
             //float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));
             //optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;
            cellPrimDesc cell;
            cell.intersectionDist = theIntersectionDistance;
            cell.type = 1; //not main
            cell.normal = normal;
            // cell.internalTypes = info.types;
            cell.maxDist = info.maxDist;
            //fake it as far
            /*cell.center = info.pos[0];// hit_point + theRay.direction*0.5;
            cell.center2 = info.pos[1];
            cell.rad1 = info.rad[0];
            cell.rad2 = info.rad[1];*/
            cell.color = make_float4(info.useScalar.x, info.useScalar.y, info.useScalar.z, 0.01);

            //cell.segLength=info.
            //cell.bmin= info.
            thePrd.cellPrimitives[thePrd.cur_prim - 1] = cell;
        }
        float skip = fmaxf(info.rad[0], info.rad[1]) * 2;
        skip = fmaxf(skip, info.maxDist) * 2;

        optix::Ray ray = optix::make_Ray(hit_point + theRay.direction * (skip + 0.01), theRay.direction, 0, sysSceneEpsilon, 10);

        // Start tracing ray from the camera and further
        rtTrace(sysTopObject, ray, thePrd);
    }
}

/*Multiscale molecule material
Initial assumptions:
//BVH doesn't overlap and we can perform volume
ray-casting without sorting

//Material doen't change significantly on ray-segment
//Thus it can be averaged and interpolated between certain
//ray-in  and ray-out values and
//regions of homogeneous material can be transmitted
*/
RT_PROGRAM void volumehit_sdf()
{
    //if (thePrd.cur_prim < 3)
    {
        float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));

        float Ka = 0.3;// + MultiscaleParam / 2;
        float Kd = 0.2;
        float Ks = 0.2;
        //todo implement primInfo type to color and a switch for mapping to color
        float3 col = Ka *  ambient_light_color;
        float4 color = make_float4(col.x, col.y, col.z, 1.0);

        if (useScalar == 1) { //otherwise primInfo is not assigned a value
            if (info.type == 0)
            {
                color = info.useScalar;
            }
            else {
                int t = info.type;// __float_as_int(info.primInfo.y);
                col = tFunction(t);// transfer_function(t);
                color = make_float4(col.x, col.y, col.z, 1.0);
            }
            color *= Ka;
        }/**/
         //if(useScalar==2)
         //{
         // color = info.color;// transfer_function(t);
         //  color *= Ka;
         //}

        optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

        for (int i = 0; i < lights.size(); ++i)
        {
            BasicLight light = lights[i];
            float3 L = optix::normalize(light.pos - hit_point);// varHit);
            float nDl = optix::dot(normal, L);

            //if (nDl > 0)
            //    color += Kd * nDl * light.color; // make_float3(1.0);//
            float4 lcol = make_float4(light.color.x, light.color.y, light.color.z, 1.0);
            float phong_exp = 0.5;//*(1 - MultiscaleParam);
            if (nDl > 0) {
                color += Kd * nDl * lcol;
            }
            /* float3 H = optix::normalize(L - thePrd.wi);
             float nDh = optix::dot(normal, H);
             if (nDh > 0)
                 color += Ks*(1 - MultiscaleParam) * lcol * pow(nDh, phong_exp);
              */
        }
        if (thePrd.cur_prim < 1) //first element
        {
            thePrd.radiance += make_float3(color.x*color.w, color.y*color.w, color.z*color.w);
        }
        thePrd.cur_prim++;
        //color.x *= color.w;
        //color.y *= color.w;
        //color.z *= color.w;

        thePrd.result += color *(1 - thePrd.result.w);
        //thePrd.result.w /= thePrd.cur_prim;
        // Create ray
        //rtIgnoreIntersection();

        //we consider that molecules don't intersect within bounding spheres
        //that type of data is just not possible
        optix::Ray ray = optix::make_Ray(hit_point + theRay.direction * info.maxDist, theRay.direction, 0, 0.0f, RT_DEFAULT_MAX);

        //remove from here
        thePrd.renderType = 1;

        // Start tracing ray from the camera and further
        rtTrace(sysTopObject, ray, thePrd);
    }
}
//
RT_PROGRAM void closesthit_sdf2()
{
    float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));
    // Visualize the resulting world space normal on the surface we're looking on.
    // Transform the normal components from [-1.0f, 1.0f] to the range [0.0f, 1.0f] to get colors for negative values.
    thePrd.radiance *= 2.2f;

    //TODO: call two callable programs
    //1 - for optical model
    //2 - for auditory model
}

__device__ void render_Surface2(float3 normal, float3 hit_point, float4 color)
{
    float Ka = 0.5;
    float Kd = 0.5;
    float Ks = 0.2;
    float tstep = 0.01;
    //scart tracing to the initial
    float3 pos = thePrd.last_hit_point;
    float3 step = theRay.direction*tstep;

    float max = optix::length(hit_point - thePrd.last_hit_point);
    float4 sum = thePrd.result;// make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here
    float totalD = max;
    //TODO still need to implement
    //start tracing the bounds itself
    float s1 = sdfPrimBack(pos, make_float3(thePrd.maxDist));
    pos += step;
    float s2 = sdfPrimBack(pos, make_float3(thePrd.maxDist));
    float i = 0;
    float trp = 0.01;
    float4 col;

    float3 color2 = Ka *  make_float3(0, 1, 0);// ambient_light_color;

    col = make_float4(color2);
    col += make_float4(0.5, 0, 0, 0);
    col.w = trp;
    col.x *= col.w;
    col.y *= col.w;
    col.z *= col.w;                                      //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

    while (i < max) //s2 < 0.01)
    {
        if (s2 < 0.01) {
            //col *= Ka;
            //col.w = 0.5; //s1 is very small
            //float3 color = Ka *  make_float3(col);// ambient_light_color;

            //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

            //initial blend

            // "over" operator for front-to-back blending
            sum = sum + col*(1.0f - sum.w);

            //save the last output point
            thePrd.last_hit_point = pos;
        }
        i += tstep;
        s1 = s2;
        pos += step;

        if (sum.w >= 1.0) i = max + 1;
        else
            s2 = sdfPrimBack(pos, make_float3(thePrd.maxDist));
        // if (abs(s1) <= eps)
    }

    trp = 0.06;
    //----------------------
    pos = hit_point;// eyeRay.o + eyeRay.d*tnear;

    s1 = sdfPrim2(pos - info.pos[0], pos - info.pos[1], info.rad[0], info.rad[1]);
    float s3 = sdfPrimBack(pos, make_float3(8.1));
    pos += step;
    s2 = sdfPrim2(pos - info.pos[0], pos - info.pos[1], info.rad[0], info.rad[1]);
    i = 0;

    max = fmax(info.rad[0], info.rad[1]);
    max = max * 2 + 0.4; //bounding box size
    totalD += max;
    while (i < max) //s2 < 0.01)
    {
        if (s2 < 0.01) {
            if (s3 < 0.0001)
            {
                //col *= Ka;
                //col.w = 0.5; //s1 is very small
                //float3 color = Ka *  make_float3(col);// ambient_light_color;

                                                      //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

                //initial blend
                col = color;
                col.w = trp;
                col.x *= col.w;
                col.y *= col.w;
                col.z *= col.w;
                // "over" operator for front-to-back blending
                sum = sum + col*(1.0f - sum.w);

                //save the last output point
                thePrd.last_hit_point = pos;
            }
        }
        i += tstep;
        s1 = s2;
        s3 = sdfPrimBack(pos, make_float3(8.1));
        pos += step;

        if (sum.w >= 1.0) i = max + 1;
        else
            s2 = sdfPrim2(pos - info.pos[0], pos - info.pos[1], info.rad[0], info.rad[1]);
        // if (abs(s1) <= eps)
    }
    //or it should be
    // thePrd.radiance = make_float3(sum);
    //thePrd.radiance += make_float3(sum);

    thePrd.maxDist -= totalD;
    thePrd.result = sum;
}

//For molecule with bonds
RT_PROGRAM void anyhit_sdf_complex()
{
    float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));
    optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

    if (thePrd.cur_prim < MAX_PRIM_ALONG_RAY)
    {
        thePrd.cur_prim++;
        //float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));

           //optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;
        cellPrimDesc cell;
        cell.intersectionDist = theIntersectionDistance;
        cell.type = 1; //not main
        cell.normal = normal;
        //        cell.internalTypes = info.types;
        cell.maxDist = info.maxDist;
        //fake it as far
       /* cell.center = info.pos[0];// hit_point + theRay.direction*0.5;
        cell.center2 = info.pos[1];
        cell.rad1 = info.rad[0];
        cell.rad2 = info.rad[1];*/
        cell.color = make_float4(info.useScalar.x, info.useScalar.y, info.useScalar.z, 0.01);

        //cell.segLength=info.
        //cell.bmin= info.
        thePrd.cellPrimitives[thePrd.cur_prim - 1] = cell;

        if (thePrd.renderType == 0)
            thePrd.renderType = 1;

        rtIgnoreIntersection();
    }

    else { //we have traced all primitives
        rtTerminateRay();
    }
}

RT_PROGRAM void anyhit_sdf_complex_old()
{
    if (thePrd.maxDist > 0) //some error here
    {
        if (thePrd.renderType > 0)
        {
            float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));

            float Ka = 0.3;// + MultiscaleParam / 2;
            float Kd = 0.2;
            float Ks = 0.2;
            //todo implement primInfo type to color and a switch for mapping to color
            float3 col = Ka *  ambient_light_color;
            float4 color = make_float4(col.x, col.y, col.z, 1.0);

            if (useScalar == 1) { //otherwise primInfo is not assigned a value
                if (info.type == 0)
                {
                    color = info.useScalar;
                }
                else {
                    int t = info.type;// __float_as_int(info.primInfo.y);
                    col = tFunction(t);// transfer_function(t);
                    color = make_float4(col.x, col.y, col.z, 1.0);
                }
                color *= Ka;
            }

            optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

            for (int i = 0; i < lights.size(); ++i)
            {
                BasicLight light = lights[i];
                float3 L = optix::normalize(light.pos - hit_point);// varHit);
                float nDl = optix::dot(normal, L);

                //if (nDl > 0)
                //    color += Kd * nDl * light.color; // make_float3(1.0);//
                float4 lcol = make_float4(light.color.x, light.color.y, light.color.z, 1.0);
                float phong_exp = 0.5 *(1 - MultiscaleParam);
                if (nDl > 0) {
                    color += Kd * nDl * lcol;
                }
            }

            //perform rendering
           // thePrd.result += make_float4(1, 0, 0, 0.5);

            thePrd.renderType = 1;
            render_Surface2(normal, hit_point, color);
            //thePrd.radiance += make_float3(color);
        }

        rtIgnoreIntersection();
    }
    else rtTerminateRay();
}