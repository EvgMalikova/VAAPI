/*=========================================================================
Program:   VolumeRenderer
Module:    optixSDFGeometry.h

=========================================================================*/

#ifndef optixSDFGeometry_h
#define optixSDFGeometry_h

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <map>
#include <memory>
#include "vaBasicObject.h"
#include "macros.h"

/** \class optixSDFGeometry
*\brief Abstract class for SDF Geometry.
* A basic class. All SDF geometry inherits it's functions
*
* All SDF geometry inherits it's functions, that includes:
*
* 1)SDF dynamic textures
*
* 2)FRep modelling primitives and operations that construct dynamically Construction Tree
* see http://hyperfun.org for details
*
* 3)Large objects, like molecular structures, that consist of lot's of objects and use
* BVH for rendering
*
* ...
*
* may be something else

* Architecture much inspired by VTK (http://vtk.org) implementation.
* Main operations similar to most of objects are
* much similar to VTK classes structure:
*
*virtual void SetContext(optix::Context &context);

*virtual void Update();

*virtual optix::Geometry GetOutput()

*virtual sdfGeo* GetOutputDesc()

*
* See [Tutorial](doc/tutorialSDF.md) for more info.
*
*/

class optixSDFGeometry : public vaBasicObject
{
public:
    /**
    * Constructor and destructor
    */
    optixSDFGeometry();
    ~optixSDFGeometry() {};

    virtual void SetContext(optix::Context &context); /**<Sets optix context that will store all generated optix geometry*/
    virtual void Update();/**<Runs the process of computation of optix geometry and setting up all necessary cuda programs and geometry parameters*/
    virtual optix::Geometry GetOutput() { return geo; }; /**<Returns ready to use output, that is optix geometry*/
    virtual sdfGeo* GetOutputDesc() { return geometryDesc.get(); }; /**<Returns description, a more detailed info, that is optix geometry and description wheter it is dynamic, auditory and etc.*/

    //std::string GetGeometryName() { return geometryName; }

    std::string GetBoundingBoxProgName() { return bounding_box; }
    std::string GetIntersectionProgName() { return intersection_program; }
    std::string GetCallableProgName() { return callable_program; }

    void SetBoundingBoxProgName(std::string bb) { bounding_box = bb; }
    void SetIntersectionProgName(std::string inter) { intersection_program = inter; }
    void SetCallableProgName(std::string c) { callable_program = c; }
    optix::Program GetCallableProg();
    void SetCallableProgManually(optix::Program pr);

    void SetMaterialType(int type);
    int GetMaterialType();

    void SetDynamic(bool d) { m_Dynamic = d; };
    bool isDynamic() { return m_Dynamic; };

    //virtual void SetTime(float time) {};
    //virtual float GetTime() { return 0; }
private:
    optix::Geometry geo;
    std::shared_ptr<sdfGeo> geometryDesc;

    std::map<std::string, optix::Program> m_mapOfPrograms;
    std::string geometryName;

    bool m_Dynamic;

    /**stored names of minimul 3 following programs
    *bounding_box
    *intersection_program
    *callable_program, that is a sdf primitive or upper object.

    *those names are reserved and can't be changed
    */
    std::string bounding_box;
    std::string intersection_program;
    std::string callable_program;

protected:
    virtual void Modified() {};
    virtual void CreateGeometry() {}; /**<TODO: potentialy read file with points here*/
    virtual void Initialize() {}; /**<read cuda procedure for SDF generation, the process is linked to name*/
    template<class T> optix::Buffer optixSDFGeometry::InitializeInputBuffer(T Attributes, std::vector<T> attributes, optix::RTbuffermapflag mode);

    void SetIntersectionProg();
    void SetBoundingBoxProg();
    void InitProg(std::string prog, std::string file, std::string name);

    /**
    *sets all main programs
    */
    virtual void SetMainPrograms();
    /**
    *Is called for setting callable prog
    */
    virtual void SetCallableProg() {};

    /**
    *for setting custom cuda variables values
    */
    virtual void SetParameters() {};

    //only exceptional use
    optix::Program GetIntersectionProg()
    {
        std::map<std::string, optix::Program>::iterator it = m_mapOfPrograms.find(GetIntersectionProgName());
        if (it != m_mapOfPrograms.end()) {
            return it->second;
        }
        else return nullptr;
    }
};

#endif