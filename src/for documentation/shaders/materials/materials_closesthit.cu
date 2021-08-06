/*
*/

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "../renderer/per_ray_data.h"
#include "../basic_lights.h"
#include "../attributeInfo.h"
//#include "per_ray_data.h"

//rtBuffer<float3> TFBuffer;

// Context global variables provided by the renderer system.
rtDeclareVariable(rtObject, sysTopObject, , );

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

//rtDeclareVariable(optix::float3, varTexCoord,  attribute TEXCOORD, );

// This closest hit program only uses the geometric normal and the shading normal attributes.
// OptiX will remove all code from the intersection programs for unused attributes automatically.

// Note that the matching between attribute outputs from the intersection program and
// the inputs in the closesthit and anyhit programs is done with the type (here float3) and
// the user defined attribute semantic (e.g. here NORMAL).
// The actual variable name doesn't need to match but it's recommended for clarity.

RT_PROGRAM void closesthit()
{
    // Transform the (unnormalized) object space normals into world space.
    float3 geoNormal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varGeoNormal));
    float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));// varNormal));

    // Check if the ray hit the geometry on the frontface or the backface.
    // The geometric normal is always defined on the front face of the geometry.
    // In this implementation the coordinate systems are right-handed and the frontface triangle winding is counter-clockwise (matching OpenGL).

    // If theRay.direction and geometric normal are in the same hemisphere we're looking at a backface.
    if (0.0f < optix::dot(theRay.direction, geoNormal))
    {
        // Flip the shading normal to the backface, because only that is used below.
        // (See later examples for more intricate handling of the frontface condition.)
        normal = -normal;
    }

    // Visualize the resulting world space normal on the surface we're looking on.
    // Transform the normal components from [-1.0f, 1.0f] to the range [0.0f, 1.0f] to get colors for negative values.
    thePrd.radiance = normal * 0.5f + 0.5f;
}

RT_PROGRAM void closesthit_sdf()
{
    /**/
    float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, info.normal));

    float Ka = 0.5;
    float Kd = 0.5;
    float Ks = 0.2;
    //todo implement primInfo type to color and a switch for mapping to color
    float3 color = Ka *  ambient_light_color;
    if (useScalar == 1) { //otherwise primInfo is not assigned a value
        if (info.type == 0)
        {
            color = info.useScalar;
        }
        else {
            int t = info.type;// __float_as_int(info.primInfo.y);
            color = tFunction(t);// transfer_function(t);
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