/*=========================================================================
Program:   VolumeRenderer
Module:    optixSDFPrimitives.h

=========================================================================*/
/**
* @class  optixSDFPrimitive
* @brief  A basic class for all sdf primitives
*
* The class creates a basic triangular plane for Optix use. Codes from Optix advanced samples.
* Architecture much inspired by VTK (http://vtk.org) implementation
*
*/

#ifndef optixSDFBasicPrimitives_h
#define optixSDFBasicPrimitives_h

#include "optixSDFGeometry.h"

class optixSDFPrimitive : public optixSDFGeometry {
public:

    optixSDFPrimitive() { };
    ~optixSDFPrimitive() {};

    virtual void SetCenter1(optix::float3 center) {};
    virtual void SetRadius1(optix::float3 radius) {};

    //Sets mainly bounding box length
    void SetRadius(optix::float3 rad) {
        radius = rad;
        optixSDFGeometry::GetOutput()["varRadius"]->setFloat(radius.x, radius.y, radius.z);
    }
    optix::float3 GetRadius() { return radius; }

    void SetCenter(optix::float3 c) {
        center = c;
        optixSDFGeometry::GetOutput()["varCenter"]->setFloat(center.x, center.y, center.z);
    }
    optix::float3 GetCenter() { return center; }

    //virtual void Update() { CreateGeometry(); };
    virtual optix::Geometry GetOutput() {
        return optixSDFGeometry::GetOutput();
    }

    //Implemented through description
    //But still might be needed for space-time blending
    /*
    virtual void SetTime(float time) {
    optix::Program pr = optixSDFGeometry::GetCallableProg();
    pr["TimeSound"]->setFloat(time);
    }
    virtual float GetTime() {
    optix::Program pr = optixSDFGeometry::GetCallableProg();
    return pr["TimeSound"]->getFloat();
    }*/
protected:

    optix::float3 center;
    optix::float3 radius;

    //Porcedure for geometry creation
    virtual void CreateGeometry();
    //virtual void GenerateGeometry();
    virtual void Initialize();

    virtual void SetCallableProg();

    //redefined in other classes
    virtual void InitCallableProg() {};
};

#endif