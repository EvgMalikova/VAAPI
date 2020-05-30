#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "optixSDFBasicPrimitives.h"

//------------------------------------
//SDF sphere primitive
void optixSDFPrimitive::CreateGeometry()
{
    //questionable choise if we have to read from the file
    //in case of molecular surfaces becomes simillar to meshes
    //with defining buffers
    //as far one sphere
    //no need to create triangles
    //directly go to primitive creation
    //look for similarities in TriObject

    optix::Geometry geometry = optixSDFGeometry::GetOutput();

    optixSDFGeometry::SetMainPrograms();
    geometry->setPrimitiveCount(1);
}

void optixSDFPrimitive::Initialize()
{
    std::cout << "Geometry initialized" << std::endl;
    //init geometry
    // Geometry
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            optixSDFGeometry::InitProg("boundingbox_sdf_sphere", "intersection_sdfPrim.cu", "boundingbox_sdf_sphere");
            optixSDFGeometry::InitProg("intersection_sdf_sphere", "intersection_sdfPrim.cu", "intersection_sdf_sphere");

            //m_mapOfPrograms["boundingbox_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("boundingbox_triangle_indexed.cu"), "boundingbox_sdf_sphere");
            //m_mapOfPrograms["intersection_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("intersection_triangle_indexed.cu"), "intersection_sdf_sphere");
            optixSDFGeometry::SetBoundingBoxProgName("boundingbox_sdf_sphere");
            optixSDFGeometry::SetIntersectionProgName("intersection_sdf_sphere");

            //compile primitive program
            std::cout << "Geometry initialized" << std::endl;
            InitCallableProg();

            std::cout << "Program inited" << std::endl;

            //std::cout << "names =" << optixSDFGeometry::GetBoundingBoxProgName().c_str() << std::endl;
            //std::cout << "names =" << optixSDFGeometry::GetIntersectionProgName().c_str() << std::endl;
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;

    // Set default parameters

    SetRadius(optix::make_float3(0.6));
    SetCenter(optix::make_float3(0.0));
}

void optixSDFPrimitive::SetCallableProg()
{
    optix::Program pr = optixSDFGeometry::GetCallableProg();
    if (pr->get() != nullptr) {
        optixSDFGeometry::GetOutput()["sdfPrim"]->setProgramId(pr);
        // it->second->getId()
    }
}