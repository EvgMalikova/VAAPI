#include "optixGeometry.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

void optixTriGeometry::Initialize() {
    std::cout << "Geometry initialized" << std::endl;
    //init geometry
    // Geometry
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            m_mapOfPrograms["boundingbox_triangle_indexed"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("boundingbox_triangle_indexed.cu"), "boundingbox_triangle_indexed");
            m_mapOfPrograms["intersection_triangle_indexed"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("intersection_triangle_indexed.cu"), "intersection_triangle_indexed");
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}
// This part is always identical in the generated geometry creation routines.
void optixTriGeometry::CreateGeometry(std::vector<VertexAttributes> const& attributes, std::vector<unsigned int> const& indices)
{
    optix::Geometry geometry(nullptr);

    try
    {
        geometry = vaBasicObject::GetContext()->createGeometry();

        optix::Buffer attributesBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER);
        attributesBuffer->setElementSize(sizeof(VertexAttributes));
        attributesBuffer->setSize(attributes.size());

        void *dst = attributesBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
        memcpy(dst, attributes.data(), sizeof(VertexAttributes) * attributes.size());
        attributesBuffer->unmap();

        optix::Buffer indicesBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT3, indices.size() / 3);
        dst = indicesBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
        memcpy(dst, indices.data(), sizeof(optix::uint3) * indices.size() / 3);
        indicesBuffer->unmap();

        std::map<std::string, optix::Program>::const_iterator it = m_mapOfPrograms.find("boundingbox_triangle_indexed");
        MY_ASSERT(it != m_mapOfPrograms.end());
        geometry->setBoundingBoxProgram(it->second);

        it = m_mapOfPrograms.find("intersection_triangle_indexed");
        MY_ASSERT(it != m_mapOfPrograms.end());
        geometry->setIntersectionProgram(it->second);

        geometry["attributesBuffer"]->setBuffer(attributesBuffer);
        geometry["indicesBuffer"]->setBuffer(indicesBuffer);
        geometry->setPrimitiveCount((unsigned int)(indices.size()) / 3);
    }
    catch (optix::Exception& e)
    {
        std::cerr << e.getErrorString() << std::endl;
    }

    //assign the handle

    geo = geometry;
    std::cout << "createTriangular geometry" << std::endl;
}

void optixGeomBasic::SetContext(optix::Context &context)
{
    vaBasicObject::SetContext(context);
    optixTriGeometry::Initialize(); //init all contex related GPU buffers

                                    //As far questionable. Should be added on final step for several geom Groups
                                    //set acceleration properties for geometry
                                    //acceleration = optixTriGeometry::GetContext()->createAcceleration(m_builder);
                                    //SetAccelerationProperties();
}

void optixGeomBasic::Update()
{
    std::vector<VertexAttributes> attributes;
    std::vector<unsigned int> indices;

    GenerateGeometry(attributes, indices);

    //std::cout << "createPlane(" << upAxis << "): Vertices = " << attributes.size() << ", Triangles = " << indices.size() / 3 << std::endl;

    //call standard triangular geometry creation function
    optixTriGeometry::CreateGeometry(attributes, indices);
}

//basic clas for geometry creation
//it can be reading triangular mesh from file as well
void optixPlane::GenerateGeometry(std::vector<VertexAttributes>& attributes, std::vector<unsigned int>& indices)
{
    MY_ASSERT(1 <= tessU && 1 <= tessV);

    const float uTile = 2.0f / float(tessU);
    const float vTile = 2.0f / float(tessV);

    optix::float3 corner;

    VertexAttributes attrib;

    switch (upAxis)
    {
    case 0: // Positive x-axis is the geometry normal, create geometry on the yz-plane.
        corner = optix::make_float3(0.0f, -1.0f, 1.0f); // Lower front corner of the plane. texcoord (0.0f, 0.0f).

        attrib.tangent = optix::make_float3(0.0f, 0.0f, -1.0f);
        attrib.normal = optix::make_float3(1.0f, 0.0f, 0.0f);

        for (int j = 0; j <= tessV; ++j)
        {
            const float v = float(j) * vTile;

            for (int i = 0; i <= tessU; ++i)
            {
                const float u = float(i) * uTile;

                attrib.vertex = corner + optix::make_float3(0.0f, v, -u);
                attrib.texcoord = optix::make_float3(u * 0.5f, v * 0.5f, 0.0f);

                attributes.push_back(attrib);
            }
        }
        break;

    case 1: // Positive y-axis is the geometry normal, create geometry on the xz-plane.
        corner = optix::make_float3(-1.0f, 0.0f, 1.0f); // left front corner of the plane. texcoord (0.0f, 0.0f).

        attrib.tangent = optix::make_float3(1.0f, 0.0f, 0.0f);
        attrib.normal = optix::make_float3(0.0f, 1.0f, 0.0f);

        for (int j = 0; j <= tessV; ++j)
        {
            const float v = float(j) * vTile;

            for (int i = 0; i <= tessU; ++i)
            {
                const float u = float(i) * uTile;

                attrib.vertex = corner + optix::make_float3(u, 0.0f, -v);
                attrib.texcoord = optix::make_float3(u * 0.5f, v * 0.5f, 0.0f);

                attributes.push_back(attrib);
            }
        }
        break;

    case 2: // Positive z-axis is the geometry normal, create geometry on the xy-plane.
        corner = optix::make_float3(-1.0f, -1.0f, 0.0f); // Lower left corner of the plane. texcoord (0.0f, 0.0f).

        attrib.tangent = optix::make_float3(1.0f, 0.0f, 0.0f);
        attrib.normal = optix::make_float3(0.0f, 0.0f, 1.0f);

        for (int j = 0; j <= tessV; ++j)
        {
            const float v = float(j) * vTile;

            for (int i = 0; i <= tessU; ++i)
            {
                const float u = float(i) * uTile;

                attrib.vertex = corner + optix::make_float3(u, v, 0.0f);
                attrib.texcoord = optix::make_float3(u * 0.5f, v * 0.5f, 0.0f);

                attributes.push_back(attrib);
            }
        }
        break;
    }

    const unsigned int stride = tessU + 1;
    for (int j = 0; j < tessV; ++j)
    {
        for (int i = 0; i < tessU; ++i)
        {
            indices.push_back(j      * stride + i);
            indices.push_back(j      * stride + i + 1);
            indices.push_back((j + 1) * stride + i + 1);

            indices.push_back((j + 1) * stride + i + 1);
            indices.push_back((j + 1) * stride + i);
            indices.push_back(j      * stride + i);
        }
    }

    std::cout << "createPlane(" << upAxis << "): Vertices = " << attributes.size() << ", Triangles = " << indices.size() / 3 << std::endl;
}