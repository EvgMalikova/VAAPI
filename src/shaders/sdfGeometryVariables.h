/*
Basic headers for all SDF primitives and geometry
*/
#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include <optix_math.h>
#include "attributeInfo.h"
//#include "../per_ray_data.h"

rtDeclareVariable(attributeInfo, info, attribute info, );

rtDeclareVariable(attributeInfo2, infoH, attribute infoH, );

rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );
rtDeclareVariable(float, theDist, rtIntersectionDistance, );
//rtDeclareVariable(PerRayData, prd, rtPayload, ); is not accessible for this
//rtDeclareVariable(PerRayData, thePrd, rtPayload, );

//sets Material index to call
//0 - optical type
//1 - auditory type

rtDeclareVariable(int, MaterialIndex, , );
