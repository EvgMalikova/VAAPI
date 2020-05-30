/*=========================================================================
Program:   VolumeRenderer
Module:    vaMapper.h

=========================================================================*/

#ifndef optixAbstractMaterial_h
#define optixAbstractMaterial_h

#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include "macros.h"
#include "vaBasicObject.h"
#include "basic_lights.h"

typedef struct MaterialDesc {
    bool auditory;
    bool dynamic;

    MaterialDesc(bool au, bool dy) { auditory = au; dynamic = dy; };
};

/**
* \class  vaAuditoryMaterial
* \brief A basic class for auditory material
*
*/
class vaAuditoryMaterial :public vaBasicObject {
public:
    virtual void Update();

    //By default there are several properties of material
    //1)optical or auditory
    //2)static or dynamic
    //TODO: add haptic material on the similar basis
    MaterialDesc GetType() { return MaterialDesc(m_isAuditory, m_isDynamic); };

    optix::Material GetOutput() { return m; };

    vaAuditoryMaterial() {
        m_isClosestHit = false;
        m_isAnyHit = false;
        m_isDynamic = false;
        m_isAuditory = false;
        m_rayType = 0;
        useScalar = false;
        m = nullptr;
    };
    ~vaAuditoryMaterial() {};

    bool isDynamic() { return m_isDynamic; };
    bool isAnyHit() { return m_isAnyHit; };
    bool isClosestHit() { return m_isClosestHit; };
    bool isAuditory() { return m_isAuditory; };
    bool isScalar() { return useScalar; }
    /**
    *Defines material as dynamic
    */
    void SetDynamicTypeOn() {
        m_isDynamic = true;
        //m_isAuditory = true;
    }
    void SetDynamicTypeOff() {
        m_isDynamic = false;
    }
    /**
    *Defines material as auditory
    */
    void SetAuditoryTypeOn() {
        //m_isDynamic = true;
        m_isAuditory = true;
    }
    //Sets material to be standard or optical
    void SetAuditoryTypeOff() {
        m_isAuditory = false;
    }

    //Helping programs
    //Handle programs names and set them
    void SetClosestHit(bool type) { m_isClosestHit = type; };
    void SetAnyHit(bool type) { m_isAnyHit = type; };
    void SetScalarMode(bool t) { useScalar = t; };

    std::string GetClosestHitProgName() { return closesthit_prog; };
    std::string GetAnyHitProgName() {
        return anyhit_prog;
    };
    std::string GetDynamicProgName() { return dynamic_prog; };

    void SetClosestHitProgName(std::string name); /*<Sets closest hit program name for further reference with GetClosestHitProgName()*/

    void SetAnyHitProgName(std::string name); /*<Sets any hit program name for further reference*/
    void SetDynamicProgName(std::string name);/*<Sets dynamic highlight program name*/

    std::string GetProgName(int i) { return names[i]; };

private:
    int m_rayType; //currently not used, but later for shadow rays
    bool useScalar;
    //returned output optix material
    optix::Material m;
    //stack of programs
    std::map<std::string, optix::Program> m_mapOfPrograms;
    std::vector<std::string> names; //all additional program names, that are not standard ones

    bool m_isClosestHit;
    bool m_isAnyHit;
    bool m_isDynamic;
    bool m_isAuditory;

    /**
    *There are only three main programs for material, defined with reserved names
    */
    std::string anyhit_prog; /*<Any hit optix program name */
    std::string closesthit_prog; /*<Closest hit optix program name */
    std::string dynamic_prog; /*<Name for callable procedure defining some dynamic changes to material*/

protected:
    optix::Program GetProgByName(std::string name);
    void SetClosestHitProg(optix::Material m);/*<Any hit optix program name */
    void SetAnyHitProg(optix::Material m);
    void SetDynamicProg(optix::Material m);

    void InitProg(std::string prog/*<name of program in cuda file*/, std::string file/*<name of cuda file*/, std::string name /*<assigned name of program for further reference*/);

    virtual void Initialize() {};
    void ApplyScalarMode()
    {
        if (m.get() != nullptr)
        {
            m["useScalar"]->setInt(int(useScalar));
        }
    };
    //set cuda variables to program
    virtual void SetMaterialParameters() {};
    virtual void InitLights() {};

    //array of lights for material. A basic definition
    std::vector<BasicLight> m_lights;
    void SetLights();
};

/**
* \class  vaMapper
* \brief A basic mapper that suggest application of one visual and one auditory material
* to one geometry instance
*
* The codes inspired by vtkMapper class. Returns optix::GeometryInstance with assigned material.
* Each material suggests definition of optical model , that treats geometry defined object as a sufrace
* or takes geometry and hetegoreneously defined attributes and maps them to optical and/or auditory properties
* for more details see ..heterogeneous objects modelling

* The class takes optix geometry as input, assigns materials and returns
*optix GeometryInstance. Class also handles switching between optical and auditory material
*to perform visual and auditory rendering accordingly
*
*/

class vaMapper :public vaBasicObject {
public:

    virtual void Update();

    void SetInput(optix::Geometry g) { geo = g; }
    optix::GeometryInstance GetOutput() { return gi; };

    void SetDescInput(sdfGeo*g) {
        geoDesc->geo = g->geo;
        geoDesc->prog = g->prog;
        geo = geoDesc->geo;
    };
    void SetScalarModeOn();
    vaMapper() { geoDesc = new sdfGeo(); };
    ~vaMapper() {
        delete geoDesc;
    };

    void AddMaterial(optix::Material m, MaterialDesc desc);
    optix::Material GetMaterial(int i);
    MaterialDesc GetMaterialDesc(int i) {
        return m_desc[i];
    };

    void SetOpticalModel();
    void SetAuditoryModel();

    //plays dynamics

    void SetTime(float time) {
        if (geoDesc != nullptr) {
            geoDesc->SetTime(time);
            //  std::cout << "there is a dynamic scene" << std::endl;
        }
    }
    void PrintInfo() {
        std::cout << " is Dynamic " << m_desc[0].dynamic << std::endl;
    }
private:
    //description of materials
    std::vector<MaterialDesc> m_desc;

    //applied materials
    std::vector<optix::Material> mat;

    //output ref
    optix::GeometryInstance gi;
    //input ref
    optix::Geometry geo;
    sdfGeo* geoDesc;

protected:
    //can be used to set custom parameters for specified material i
    virtual void SetMaterialParameters(int i) {};

    virtual void Modified() {};
};
#endif