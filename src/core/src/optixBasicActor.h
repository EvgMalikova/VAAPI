/*=========================================================================
Program:   VolumeRenderer
Module:    optixActor.h

=========================================================================*/

#ifndef optixBasicActor_h
#define optixBasicActor_h

#include <cstring>
#include <iostream>
#include <sstream>

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include "macros.h"
#include "optixAbstractMaterial.h"

/**
* \class  vaBasicActor
* \brief  basic abstract actor class
*
* The class creates a basic Actor for Optix use. Architecture much inspired by VTK (http://vtk.org) implementation
*
*/
class vaBasicActor :public vaBasicObject
{
public:
    vaBasicActor() {
        vaBasicObject::vaBasicObject();

        //TODO: BVH acceleration true only for triangular
        //and array of geometrical objects,
        //for example, molecular structures

        //For FREP construction tree of SDFs
        //Check for number of nodes in geometry tree
        //and switch of if there is only one object
        m_builder = std::string("Trbvh");
        m_triangulated = false;
    };
    ~vaBasicActor() {
        m_mapper.clear();
    };
    virtual void SetContext(optix::Context &context)
    {
        vaBasicObject::SetContext(context);
    }

    void AddMapper(std::shared_ptr<vaMapper> map) {
        //vaMapper* m = map.get();
        m_mapper.push_back(map);
        geom.push_back(std::move(map->GetOutput()));
    };

    void AddMapper2(vaMapper* map) {
        m_mapper.push_back(std::shared_ptr<vaMapper>(map));
        geom.push_back(map->GetOutput());
    };
    //void SetInput(optix::GeometryInstance gi) { geom.push_back(gi); };

    void SetAuditoryModel()
    {
        for (int i = 0; i < m_mapper.size(); i++)
        {
            m_mapper[i]->SetAuditoryModel();
        }
    }

    void SetOpticalModel()
    {
        for (int i = 0; i < m_mapper.size(); i++)
        {
            m_mapper[i]->SetOpticalModel();
        }
    }
    void Update();
    optix::GeometryGroup GetOutput() { return gg; };

    //for playing animation in dynamic scenes
    void SetTime(float time)
    {
        for (int i = 0; i < m_mapper.size(); i++)
            m_mapper[i]->SetTime(time);
    }
    void PrintInfo() {
        m_mapper[0]->PrintInfo(); //is not available for some reason
        //if(m_mapper[0])
        //std::cout << "info" << m_mapper.size()<< std::endl;// something is wrong in particular with mappers
    }

    optix::Acceleration GetAcceleration() { return acceleration; }
    void SetAccelerationType(std::string type) { m_builder = type; }

private:

    std::vector<optix::GeometryInstance> geom;
    std::vector<std::shared_ptr<vaMapper>> m_mapper;

    optix::Acceleration acceleration;
    bool m_triangulated;

protected:

    std::string m_builder;
    optix::GeometryGroup gg;
    virtual void SetAccelerationProperties() {};
    virtual void RebuildAccel() {};
};
#endif