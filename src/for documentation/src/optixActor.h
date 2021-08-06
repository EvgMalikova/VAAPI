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

#include "shaders/vertex_attributes.h"

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

class optixSdfActor :public optixBasicActor
{
public:
    optixSdfActor() {
        m_builder="Bvh8";
    };
    ~optixSdfActor() {};


protected:
    //virtual void SetAccelerationProperties();
    virtual void RebuildAccel();


};
#endif