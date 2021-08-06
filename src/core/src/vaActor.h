/*=========================================================================
Program:   VolumeRenderer
Module:    optixActor.h

=========================================================================*/

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

/**
\class vaTRIActor
\brief
*
* The class creates a basic Actor for triangular mesh objects use.
* This direction of work is currently is not supported or tested
*
*/
class vaTRIActor :public vaBasicActor
{
public:
    vaTRIActor() {};
    ~vaTRIActor() {};

protected:
    virtual void SetAccelerationProperties();
};

/*
\class vaActor
\brief standard actor, that is used with SDF objects
*/
class vaActor :public vaBasicActor
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