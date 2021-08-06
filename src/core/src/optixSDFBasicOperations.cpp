#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "optixSDFBasicOperations.h"

//------------------------------------
//SDF unary program

void optixSDFUnaryOp::SetContext(optix::Context context)
{
    vaBasicObject::SetContext(context);
    //no new geometry creation
    //the old one is redefined
    Initialize(); //init all contex related GPU buffers

                  //As far questionable. Should be added on final step for several geom Groups
                  //set acceleration properties for geometry
                  //acceleration = optixTriGeometry::GetContext()->createAcceleration(m_builder);
                  //SetAccelerationProperties();
}
void optixSDFUnaryOp::CreateGeometry()
{

    SetMainPrograms();

}

void optixSDFUnaryOp::SetCallableProg()
{

    if (m_sdf != nullptr) {
        //apply operation on top
        optix::Program primProg = m_sdf->GetCallableProg();
        optix::Program pr = optixSDFGeometry::GetCallableProg();

        if (pr->get() != nullptr) {

            pr["sdfOpPrim"]->setProgramId(primProg);// geoDesc->prog); //setOperand geoDesc->prog);//
                                                     //std::cout << "reset callable program" << std::endl;
            g1["sdfPrim"]->setProgramId(pr);
            // it->second->getId()
            //std::cout << "reset callable program" << std::endl;
            //geoDesc->prog = pr;

        }
    }


}


void optixSDFUnaryOp::Initialize()
{


    // init Geometry
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            optixSDFGeometry::InitProg("boundingbox_sdf_sphere", "intersection_sdfPrim.cu", "boundingbox_sdf_sphere");
            optixSDFGeometry::InitProg("intersection_sdf_sphere", "intersection_sdfPrim.cu", "intersection_sdf_sphere");

            //compile primitive program

            //m_mapOfPrograms["boundingbox_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("boundingbox_triangle_indexed.cu"), "boundingbox_sdf_sphere");
            //m_mapOfPrograms["intersection_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("intersection_triangle_indexed.cu"), "intersection_sdf_sphere");
            optixSDFGeometry::SetBoundingBoxProgName("boundingbox_sdf_sphere");
            optixSDFGeometry::SetIntersectionProgName("intersection_sdf_sphere");

            InitCallableProg();

            //std::cout << "names =" << optixSDFGeometry::GetBoundingBoxProgName().c_str() << std::endl;
            //std::cout << "names =" << optixSDFGeometry::GetIntersectionProgName().c_str() << std::endl;
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;

    //TODO:define parameters

}

void optixSDFUnaryOp::SetMainPrograms()
{
    try
    {
        SetCallableProg();
        AdjustCenterAndBoundingBox();
    }
    catch (optix::Exception& e)
    {
        std::cerr << e.getErrorString() << std::endl;
    }


}


//------------------------------------
//SDF Binary program

void optixSDFBinaryOp::SetContext(optix::Context context)
{
    vaBasicObject::SetContext(context);
    //no new geometry creation
    //the old one is redefined
    Initialize(); //init all contex related GPU buffers

                  //As far questionable. Should be added on final step for several geom Groups
                  //set acceleration properties for geometry
                  //acceleration = optixTriGeometry::GetContext()->createAcceleration(m_builder);
                  //SetAccelerationProperties();
}
void optixSDFBinaryOp::CreateGeometry()
{

    SetMainPrograms();

}

void optixSDFBinaryOp::SetCallableProg()
{
    if ((m_sdf1 != nullptr)&&(m_sdf2!=nullptr)) {
        //apply operation on top
        optix::Program primProg1 = m_sdf1->GetCallableProg();
        optix::Program primProg2 = m_sdf2->GetCallableProg();
            optix::Program pr = optixSDFGeometry::GetCallableProg();

        if (pr->get() != nullptr) {

            pr["sdfOpPrim"]->setProgramId(primProg1); // geoDesc1->prog);// primProg); //setOperand
            pr["sdfOpPrim2"]->setProgramId(primProg2); //geoDesc2->prog);                                             //std::cout << "reset callable program" << std::endl;

            //first operand is set to be the main geometry
            g1["sdfPrim"]->setProgramId(pr);
            // it->second->getId()
            //std::cout << "reset callable program" << std::endl;
            primProg1 = pr;
        }

       
    }
}


void optixSDFBinaryOp::Initialize()
{


    // init Geometry
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            optixSDFGeometry::InitProg("boundingbox_sdf_sphere", "intersection_sdfPrim.cu", "boundingbox_sdf_sphere");
            optixSDFGeometry::InitProg("intersection_sdf_sphere", "intersection_sdfPrim.cu", "intersection_sdf_sphere");

            //compile primitive program

            //m_mapOfPrograms["boundingbox_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("boundingbox_triangle_indexed.cu"), "boundingbox_sdf_sphere");
            //m_mapOfPrograms["intersection_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("intersection_triangle_indexed.cu"), "intersection_sdf_sphere");
            optixSDFGeometry::SetBoundingBoxProgName("boundingbox_sdf_sphere");
            optixSDFGeometry::SetIntersectionProgName("intersection_sdf_sphere");

            InitCallableProg();

            //std::cout << "names =" << optixSDFGeometry::GetBoundingBoxProgName().c_str() << std::endl;
            //std::cout << "names =" << optixSDFGeometry::GetIntersectionProgName().c_str() << std::endl;
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;

    //TODO:define parameters

}

void optixSDFBinaryOp::SetMainPrograms()
{
    try
    {
        SetCallableProg();
        AdjustCenterAndBoundingBox();
    }
    catch (optix::Exception& e)
    {
        std::cerr << e.getErrorString() << std::endl;
    }


}
