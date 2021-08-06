/*
* Stores universal attribute data that should be shared between
* - geometry intersection
* - material

* This
*/

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

//#include <optixu/optixu_vector_types.h>
#include "primDesc.h"

typedef struct attributeInfo
{
    //used for shading
    float3 hit_point; /*<hit point of intersection with primitive*/
    float ao;
    float3 normal; /*<computed normal*/
    float4 useScalar; /*<indicator to eighter use type date form primInfo for mapping or
                      defined color and etc.*/
                      //used by transfer functions to map to scalars
    float2 primInfo; /*<stores intersection distance and type of object, intersected
                     is used by transfer function to map type to color value or frequency or etc.*/
    float3 xyz;
    int type;
    int2 types; //for color interpolation
    float maxDist;
    float3 pos[5];
    float rad[5];
    //primParamDesc desc;
};

typedef struct attributeInfo2
{
    //used for shading
    float3 hit_point; /*<hit point of intersection with primitive*/

    float3 normal; /*<computed normal*/
    float4 useScalar; /*<indicator to eighter use type date form primInfo for mapping or*/
    float tmin;
    float maxDist;

    primParamDesc desc;
};

#endif // ATTRIBUTE_H
