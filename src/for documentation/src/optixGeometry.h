/*=========================================================================
Program:   VolumeRenderer
Module:    optixGeometry.h

=========================================================================*/
/**
* @class  optixGeometry
* @brief
*
* The class creates a basic triangular plane for Optix use. Codes from Optix advanced samples.
* Architecture much inspired by VTK (http://vtk.org) implementation
*
*/

#ifndef optixGeometry_h
#define optixGeometry_h

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
#include "inc/MyAssert.h"
#include "vaBasicObject.h"
#include "macros.h"

//-----------------------------------
//--------------------Functions to automatically define parameters types
//https://www.quora.com/How-do-I-check-if-the-template-type-T-is-of-the-integer-type-and-give-the-error-if-not
//https://stackoverflow.com/questions/580922/identifying-primitive-types-in-templates

/**
* \class optixTriGeometry
* \brief Abstract class for Triangulated Geometry
*
*The visualisation of triangulated meshes is a direction that is not highly developed
* by this API. However, this class implements basic features for such objects integration
* within API and further developmet
*------------------------*/

class optixTriGeometry : public vaBasicObject
{
public:
    optixTriGeometry() {
        vaBasicObject::vaBasicObject();
    };
    ~optixTriGeometry() {};

    virtual void Update() {};
    virtual optix::Geometry GetOutput() { return geo; };

private:
    optix::Geometry geo;
    std::map<std::string, optix::Program> m_mapOfPrograms;

protected:
    virtual void Modified() {};
    void CreateGeometry(std::vector<VertexAttributes> const& attributes, std::vector<unsigned int> const& indices);
    void Initialize();
};

/*------------------
*
*Basic class for Triangulated Geometry
*
*------------------------*/
class optixGeomBasic : public optixTriGeometry {
public:

    optixGeomBasic() {
        optixTriGeometry::optixTriGeometry();
    };

    virtual void SetContext(optix::Context &context);

    ~optixGeomBasic() {};

    virtual void Update();
    virtual optix::Geometry GetOutput() {
        return optixTriGeometry::GetOutput();
    }

protected:

    //Porcedure for geometry creation
    //void CreateGeometry();
    virtual void GenerateGeometry(std::vector<VertexAttributes>& attributes, std::vector<unsigned int>& indices) {};
    //void SetAccelerationProperties();
    //acceleration structure parameters
};

/*------------------
*
*Example class for plane
*Can be any triangulated surface read from the file
*
*------------------------*/

class optixPlane : public optixGeomBasic {
public:

    optixPlane() { optixGeomBasic::optixGeomBasic(); };
    ~optixPlane() {};

    //@{
    /**
    * Specify the resolution of the plane along the first axes.
    */
    opxSetMacro(tessU, int);
    opxGetMacro(tessU, int);
    //@}

    //@{
    /**
    * Specify the resolution of the plane along the second axes.
    */
    opxSetMacro(tessV, int);
    opxGetMacro(tessV, int);
    //@}

    //@{
    /**
    * Specify the up.
    */
    opxSetMacro(upAxis, int);
    opxGetMacro(upAxis, int);
    //@}

    //virtual void Update() { CreateGeometry(); };
    virtual optix::Geometry GetOutput() {
        return optixTriGeometry::GetOutput();
    }
protected:

    int tessU;
    int tessV;
    int upAxis;

    //Porcedure for geometry creation
    virtual void GenerateGeometry(std::vector<VertexAttributes>& attributes, std::vector<unsigned int>& indices);
};
#endif