/*=========================================================================
Program:   VolumeRenderer
Module:    optixActor.h

=========================================================================*/
/**
* @class  optixActor
* @brief
*
* The class creates a basic Actor for Optix use. Architecture much inspired by VTK (http://vtk.org) implementation
*
*/

#ifndef optixActor_h
#define optixActor_h

#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include "data_structures/vertex_attributes.h"

#include "macros.h"
#include "optixBasicActor.h"
class optixTRIActor :public optixBasicActor
{
public:
    optixTRIActor() {};
    ~optixTRIActor() {};

protected:
    virtual void SetAccelerationProperties();
};

/*
\class vaActor
\brief standard actor, that is used with SDF objects
*/
class vaActor :public optixBasicActor
{
public:
    vaActor() {
        m_builder = "Bvh8";
    };
    ~vaActor() {};

protected:
    //virtual void SetAccelerationProperties();
    virtual void RebuildAccel();
};

#endif