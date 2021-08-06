#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "optixSDFOperations.h"
#include "optixSDFGeometry.h"

//https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm

//------------------
//Rounding op
//----------------

void SDFRoundingOp::InitCallableProg()
{
    optixSDFGeometry::InitProg("opRound", "intersection_sdfPrimLib.cu", "opRound");
    optixSDFGeometry::SetCallableProgName("opRound");
}
void SDFRoundingOp::AdjustCenterAndBoundingBox()
{
    //adjust bounding box
    optix::float3 rad;
    GetInput()["varRadius"]->getFloat(rad.x, rad.y, rad.z);
    rad.x += 0.02;
    rad.y += 0.02;
    rad.z += 0.02;
    GetInput()["varRadius"]->setFloat(rad.x, rad.y, rad.z);
}

//------------------
//Elongate op
//----------------

void SDFElongateOp::InitCallableProg()
{
    optixSDFGeometry::InitProg("opElongate", "intersection_sdfPrimLib.cu", "opElongate");
    optixSDFGeometry::SetCallableProgName("opElongate");
}
void SDFElongateOp::AdjustCenterAndBoundingBox()
{
    //adjust bounding box
    optix::float3 rad;
    GetInput()["varRadius"]->getFloat(rad.x, rad.y, rad.z);
    rad += m_h;

    GetInput()["varRadius"]->setFloat(rad.x, rad.y, rad.z);
}

//------------------
//SDFBlendUnionOp
//----------------

void SDFBlendUnionOp::InitCallableProg()
{
    optixSDFGeometry::InitProg("opSmoothUnion", "intersection_sdfPrimLib.cu", "opSmoothUnion");
    optixSDFGeometry::SetCallableProgName("opSmoothUnion");
}
void SDFBlendUnionOp::AdjustCenterAndBoundingBox()
{
    //adjust bounding box
    optix::float3 rad;
    GetInput1()["varRadius"]->getFloat(rad.x, rad.y, rad.z);
    optix::float3 rad2;
    GetInput2()["varRadius"]->getFloat(rad2.x, rad2.y, rad2.z);

    rad += rad2;
    GetInput1()["varRadius"]->setFloat(rad.x, rad.y, rad.z);

    //optix::float3 center;

    GetInput1()["varCenter"]->getFloat(rad.x, rad.y, rad.z);
    //optix::float3 center2;
    GetInput2()["varCenter"]->getFloat(rad2.x, rad2.y, rad2.z);

    rad += rad2;
    rad /= 2.0;
    GetInput1()["varCenter"]->setFloat(rad.x, rad.y, rad.z);
}

//------------------
//SDFBlendIntersectionOp
//----------------

void SDFBlendIntersectionOp::InitCallableProg()
{
    optixSDFGeometry::InitProg("opSmoothIntersection", "intersection_sdfPrimLib.cu", "opSmoothIntersection");
    optixSDFGeometry::SetCallableProgName("opSmoothIntersection");
}
void SDFBlendIntersectionOp::AdjustCenterAndBoundingBox()
{
    //adjust bounding box
    optix::float3 rad;
    GetInput1()["varRadius"]->getFloat(rad.x, rad.y, rad.z);
    optix::float3 rad2;
    GetInput2()["varRadius"]->getFloat(rad2.x, rad2.y, rad2.z);

    rad += rad2;
    GetInput1()["varRadius"]->setFloat(rad.x, rad.y, rad.z);

    //optix::float3 center;

    GetInput1()["varCenter"]->getFloat(rad.x, rad.y, rad.z);
    //optix::float3 center2;
    GetInput2()["varCenter"]->getFloat(rad2.x, rad2.y, rad2.z);

    rad += rad2;
    rad /= 2.0;
    GetInput1()["varCenter"]->setFloat(rad.x, rad.y, rad.z);
}

//------------------
//SDFBlendSubtractionOp
//----------------

void SDFBlendSubtractionOp::InitCallableProg()
{
    optixSDFGeometry::InitProg("opSmoothSubtraction", "intersection_sdfPrimLib.cu", "opSmoothSubtraction");
    optixSDFGeometry::SetCallableProgName("opSmoothSubtraction");
}

void SDFBlendSubtractionOp::AdjustCenterAndBoundingBox()
{
    //adjust bounding box
    optix::float3 rad;
    GetInput1()["varRadius"]->getFloat(rad.x, rad.y, rad.z);
    optix::float3 rad2;
    GetInput2()["varRadius"]->getFloat(rad2.x, rad2.y, rad2.z);

    rad += rad2;
    GetInput1()["varRadius"]->setFloat(rad.x, rad.y, rad.z);

    //optix::float3 center;

    GetInput1()["varCenter"]->getFloat(rad.x, rad.y, rad.z);
    //optix::float3 center2;
    GetInput2()["varCenter"]->getFloat(rad2.x, rad2.y, rad2.z);

    rad += rad2;
    rad /= 2.0;
    GetInput1()["varCenter"]->setFloat(rad.x, rad.y, rad.z);
}

//------------------
//SDFSubtractionOp
//----------------

void SDFSubtractionOp::InitCallableProg()
{
    optixSDFGeometry::InitProg("opSubtraction", "intersection_sdfPrimLib.cu", "opSubtraction");
    optixSDFGeometry::SetCallableProgName("opSubtraction");
}

void SDFSubtractionOp::AdjustCenterAndBoundingBox()
{
    //adjust bounding box
    optix::float3 rad;
    GetInput1()["varRadius"]->getFloat(rad.x, rad.y, rad.z);
    optix::float3 rad2;
    GetInput2()["varRadius"]->getFloat(rad2.x, rad2.y, rad2.z);

    rad += rad2;
    GetInput1()["varRadius"]->setFloat(rad.x, rad.y, rad.z);

    //optix::float3 center;

    GetInput1()["varCenter"]->getFloat(rad.x, rad.y, rad.z);
    //optix::float3 center2;
    GetInput2()["varCenter"]->getFloat(rad2.x, rad2.y, rad2.z);

    rad += rad2;
    rad /= 2.0;
    GetInput1()["varCenter"]->setFloat(rad.x, rad.y, rad.z);
}