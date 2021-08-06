/*
* Stores universal attribute data that should be shared between
* - geometry intersection
* - material

* This
*/

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <optixu/optixu_vector_types.h>

typedef struct attributeInfo
{
    //used for shading
    float3 hit_point; /*<hit point of intersection with primitive*/

    float3 normal; /*<computed normal*/
    float3 useScalar; /*<indicator to eighter use type date form primInfo for mapping or
                      defined color and etc.*/
                      //used by transfer functions to map to scalars
    float2 primInfo; /*<stores intersection distance and type of object, intersected
                     is used by transfer function to map type to color value or frequency or etc.*/
    float3 xyz;
    int type;
	
};

#endif // ATTRIBUTE_H
