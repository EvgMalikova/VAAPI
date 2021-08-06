#include "optixSDFGeometry.h"

#include <algorithm>
#include <cstring>

#include <iostream>
#include <sstream>



//---------------------------------------
//------------SDF
/*
template <typename T>
struct Param {
{ RTformat RTTypeForBuffer; //RT_FORMAT_USER
                    //RT_FORMAT_UNSIGNED_INT3
 T val;

}

SDFFunction::SDFFunction()
{

}
optix::Buffer SDFFunction::

*/

template<class T>RTformat getFormat()
{
    return RT_FORMAT_USER;
};
template<>RTformat getFormat<int>()
{
    return RT_FORMAT_INT;
};


optixSDFGeometry::optixSDFGeometry()
{
vaBasicObject::vaBasicObject();

bounding_box = "bounding_box";
intersection_program = "intersection_program";
callable_program = "callable_program";

geometryDesc = std::shared_ptr<sdfGeo>(new sdfGeo());
//matrix = optix::Matrix4x4::identity();
/*
//TODO look for codes operating matrix here in
//https://github.com/knightcrawler25/Optix-PathTracer/blob/master/src/sutil/Camera.cpp
const Matrix4x4 frame = Matrix4x4::fromBasis(
normalize( m_camera_u ),
normalize( m_camera_v ),
normalize( -camera_w ),
m_camera_lookat);
const Matrix4x4 frame_inv = frame.inverse();
*/
};

template<class T> optix::Buffer optixSDFGeometry::InitializeInputBuffer(T Attributes, std::vector<T> attributes, optix::RTbuffermapflag mode)
{
    optix::Buffer attributesBuffer;
    if (getFormat<T>() == RT_FORMAT_USER)
    {
        attributesBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER);
        attributesBuffer->setElementSize(sizeof(Attributes));
        attributesBuffer->setSize(attributes.size());

        void *dst = attributesBuffer->map(0, mode);
        memcpy(dst, attributes.data(), sizeof(Attributes) * attributes.size());
        attributesBuffer->unmap();
        std::cout << "user type of data" << std::endl;
    }
    else
    {
        //TODO: handle more complecated data like indices those are of 3
        /*
        optix::Buffer indicesBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT3, indices.size() / 3);
        dst = indicesBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
        memcpy(dst, indices.data(), sizeof(optix::uint3) * indices.size() / 3);
        indicesBuffer->unmap();
        */
        attributesBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, getFormat<T>(), attributes.size());
        memcpy(dst, attributes.data(), sizeof(Attributes) * attributes.size());
        attributesBuffer->unmap();

    }
    /*
    //TODO: buffers are attached to geometry
    geometry["attributesBuffer"]->setBuffer(attributesBuffer);
    geometry["indicesBuffer"]->setBuffer(indicesBuffer);
    geometry->setPrimitiveCount((unsigned int)(indices.size()) / 3);

    //and still we have to write buffer in cuda file like

    rtBuffer<VertexAttributes> attributesBuffer;
    rtBuffer<uint3>            indicesBuffer;


    */
    return attributesBuffer;
}


void optixSDFGeometry::SetBoundingBoxProg()
{
    std::map<std::string, optix::Program>::const_iterator it = m_mapOfPrograms.find(GetBoundingBoxProgName());
    if (it != m_mapOfPrograms.end()) {
        geo->setBoundingBoxProgram(it->second);
    }
  
}

void optixSDFGeometry::SetCallableProgManually(optix::Program pr)
 {
    
    std::map<std::string, optix::Program>::iterator it = m_mapOfPrograms.find(GetCallableProgName());
    if (it != m_mapOfPrograms.end()) {
        it->second = pr;

    }
}

void optixSDFGeometry::SetIntersectionProg()
{
    std::map<std::string, optix::Program>::const_iterator it = m_mapOfPrograms.find(GetIntersectionProgName());
    if (it != m_mapOfPrograms.end()) {
        geo->setIntersectionProgram(it->second);
    }
}
optix::Program optixSDFGeometry::GetCallableProg()
{
    std::map<std::string, optix::Program>::const_iterator it = m_mapOfPrograms.find(GetCallableProgName());
    if (it != m_mapOfPrograms.end()) {
        return it->second;
    }
    else return nullptr;
}


void optixSDFGeometry::InitProg(std::string prog, std::string file, std::string name)
{
    m_mapOfPrograms[name] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath(file), prog);

}

void optixSDFGeometry::SetMaterialType(int type)
{

    optixSDFGeometry::GetOutput()["MaterialIndex"]->setInt(type);

}

int optixSDFGeometry::GetMaterialType()
{

    return optixSDFGeometry::GetOutput()["MaterialIndex"]->getInt();

}
void optixSDFGeometry::Update()
{
    // std::vector<VertexAttributes> attributes;
    //std::vector<unsigned int> indices;

    // GenerateGeometry();

    //std::cout << "createPlane(" << upAxis << "): Vertices = " << attributes.size() << ", Triangles = " << indices.size() / 3 << std::endl;

    //call standard triangular geometry creation function
    std::cout << "start creating geometry " << std::endl;
    CreateGeometry();
    std::cout << "finish creating geometry " << std::endl;
    SetParameters();
}


void optixSDFGeometry::SetContext(optix::Context &context)
{
    vaBasicObject::SetContext(context);
    geo = vaBasicObject::GetContext()->createGeometry();
    geometryDesc->geo = geo;
    /*
    inline Geometry ContextObj::createGeometry()
  {
    RTgeometry geometry;
    checkError( rtGeometryCreate( m_context, &geometry ) );
    return Geometry::take(geometry);
  }
  typedef struct sdfRTgeometry_api : RTgeometry_api {
  int prog_id=0;
  };
  typedef struct RTgeometry_api           * RTgeometry;
    */
    Initialize(); //init all contex related GPU buffers

                  //As far questionable. Should be added on final step for several geom Groups
                  //set acceleration properties for geometry
                  //acceleration = optixTriGeometry::GetContext()->createAcceleration(m_builder);
                  //SetAccelerationProperties();
}

void optixSDFGeometry::SetMainPrograms()
{
    try
    {
        SetBoundingBoxProg();
        std::cout << "bounding box set" << std::endl;
        SetIntersectionProg();
        std::cout << "intersection set" << std::endl;
        //TODO: link to name 
        //Set variable

        //initiate callable prog
        SetCallableProg();
		//geometryDesc->prog=GetCallableProg();

    }
    catch (optix::Exception& e)
    {
        std::cerr << e.getErrorString() << std::endl;
    }
}

