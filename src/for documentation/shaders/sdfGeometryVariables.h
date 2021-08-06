/*
Basic headers for all SDF primitives and geometry
*/
#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include <optix_math.h>
#include "attributeInfo.h"

rtDeclareVariable(attributeInfo, info, attribute info, );
rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );
rtDeclareVariable(float, theDist, rtIntersectionDistance, );
//rtDeclareVariable(PerRayData, prd, rtPayload, ); is not accessible for this



//sets Material index to call
//0 - optical type
//1 - auditory type

rtDeclareVariable(int, MaterialIndex, , );

