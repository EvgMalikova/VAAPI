#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <time.h>       /* time */
#include "optixSDFPrimitives.h"

//-------------------------------------
//SDF Sphere prim

void optixSDFSphere::InitCallableProg()
{
    optixSDFGeometry::InitProg("sdSphere", "intersection_sdfPrimLib.cu", "sdf_sphere");
    optixSDFGeometry::SetCallableProgName("sdf_sphere");
}

void optixSDFSphere::SetCenter1(optix::float3 center)
{
    optix::Program pr = GetCallableProg();
    pr["varCenter"]->setFloat(center.x, center.y, center.z);
    optixSDFPrimitive::SetCenter(center);
    //optixSDFGeometry::GetOutput()["varCenter"]->setFloat(center.x, center.y, center.z);
    //std::cout << pr["varCenter"]->getFloat() << std::endl;
}

void optixSDFSphere::SetRadius1(optix::float3 radius)
{
    optix::Program pr = GetCallableProg();
    pr["varRadius"]->setFloat(radius.x, radius.y, radius.z);
    optixSDFPrimitive::SetRadius(radius);

    //std::cout << pr["varRadius"]->getFloat() << std::endl;
}

//-------------------------------------
//SDF Box prim

void optixSDFBox::InitCallableProg()
{
    optixSDFGeometry::InitProg("sdfBox", "intersection_sdfPrimLib.cu", "sdf_sphere");
    optixSDFGeometry::SetCallableProgName("sdf_sphere");
}

void optixSDFBox::SetCenter1(optix::float3 center)
{
    optix::Program pr = GetCallableProg();
    pr["varCenter"]->setFloat(center.x, center.y, center.z);
    optixSDFPrimitive::SetCenter(center);
    //optixSDFGeometry::GetOutput()["varCenter"]->setFloat(center.x, center.y, center.z);
    //std::cout << pr["varCenter"]->getFloat() << std::endl;
}

void optixSDFBox::SetDims(optix::float3 radius)
{
    optix::Program pr = GetCallableProg();
    pr["varRadius"]->setFloat(radius.x, radius.y, radius.z);
    optixSDFPrimitive::SetRadius(radius);

    //std::cout << pr["varRadius"]->getFloat() << std::endl;
}

//-------------------------------------
//SDF Torus prim

void optixSDFTorus::InitCallableProg()
{
    optixSDFGeometry::InitProg("sdfTorus", "intersection_sdfPrimLib.cu", "sdf_sphere");
    optixSDFGeometry::SetCallableProgName("sdf_sphere");
}

void optixSDFTorus::SetCenter1(optix::float3 center)
{
    optix::Program pr = GetCallableProg();
    pr["varCenter"]->setFloat(center.x, center.y, center.z);
    optixSDFPrimitive::SetCenter(center);
    //optixSDFGeometry::GetOutput()["varCenter"]->setFloat(center.x, center.y, center.z);
    //std::cout << pr["varCenter"]->getFloat() << std::endl;
}

void optixSDFTorus::SetRadius1(optix::float3 radius)
{
    //only first two are used
    optix::Program pr = GetCallableProg();
    pr["varT"]->setFloat(radius.x, radius.y);
    //TODO: corectly define boundaries
    optixSDFPrimitive::SetRadius(optix::make_float3(radius.x + radius.y, radius.x + radius.y, radius.x + radius.y));

    //std::cout << pr["varRadius"]->getFloat() << std::endl;
}

//------------------------------------
//SDF Texture primitive

void sdfTexture::InitCallableProg()
{
    optixSDFGeometry::InitProg("sdfField", "intersection_sdfPrimLib.cu", "sdfField");
    optixSDFGeometry::SetCallableProgName("sdfField");
}

void sdfTexture::SetTexture(optix::TextureSampler samp, sdfParams* param)
{
    optix::float3 cent = optix::make_float3(0);
    float rad = float(param->texSize);

    sampler.push_back(samp);
    m_param.push_back(sdfParams(param));
    SetRadius(optix::make_float3(1.0));
    SetCenter(cent);
}

void sdfTexture::SetParameters() {
    //TODO curently here
    optix::Program pr = optixSDFGeometry::GetCallableProg();
    pr["numTexDefined"]->setInt(1);
    for (int i = 0; i < 2; i++) {
        if (i < sampler.size()) { //we can set only 3 samplers
            std::string samp_string = "texSDF" + std::to_string(i);
            std::string shift_string = "shift" + std::to_string(i);
            std::string size_string = "size" + std::to_string(i);

            pr[samp_string]->setTextureSampler(sampler[i]);

            pr[shift_string]->setFloat(m_param[i].lvShift);
            pr[size_string]->setFloat(m_param[i].texSize);
        }
        else //fill blank
        { //TODO fix with dynamic texture sampler
            //Try to use Buffer here
            std::string samp_string = "texSDF" + std::to_string(i);
            std::string shift_string = "shift" + std::to_string(i);
            std::string size_string = "size" + std::to_string(i);

            pr[samp_string]->setTextureSampler(sampler[0]);

            pr[shift_string]->setFloat(m_param[0].lvShift);
            pr[size_string]->setFloat(m_param[0].texSize);
        }

        //std::min
    }
    int num = std::min(3, int(sampler.size()));
    pr["numTexDefined"]->setInt(num);
    if (num > 1) SetDynamic(true);
    pr["timeSound"]->setFloat(0.0);
    GetOutputDesc()->prog = pr;
}
//------------------------------------
//Molecule primitive

void sdfCPKMol::SetCallableProg()
{
    optix::Program pr = optixSDFGeometry::GetCallableProg();
    if (pr->get() != nullptr) {
        optixSDFGeometry::GetOutput()["sdfPrim"]->setProgramId(pr);
        // it->second->getId()
    }
}

void sdfCPKMol::CreateGeometry()
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
    optixSDFGeometry::GetOutput()->setPrimitiveCount(primNumber);
}

void sdfCPKMol::Initialize()
{
    //creates geometry

    std::cout << "Programs initialized" << std::endl;
    //init geometry
    // Geometry
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            optixSDFGeometry::InitProg("boundingbox_mol", "intersection_sphere_indexed.cu", "boundingbox_mol");
            optixSDFGeometry::InitProg("intersection_mol", "intersection_sphere_indexed.cu", "intersection_mol");
            //m_mapOfPrograms["boundingbox_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("boundingbox_triangle_indexed.cu"), "boundingbox_sdf_sphere");
            // m_mapOfPrograms["intersection_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("intersection_triangle_indexed.cu"), "intersection_sdf_sphere");
            optixSDFGeometry::SetBoundingBoxProgName("boundingbox_mol");
            optixSDFGeometry::SetIntersectionProgName("intersection_mol");

            //compile primitive program
            optixSDFGeometry::InitProg("sdfSphere", "intersection_sdfPrimLib.cu", "sdf_sphere");
            optixSDFGeometry::SetCallableProgName("sdf_sphere");
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;

    // Set default parameters
    //NO default parameters
    // SetRadius(0.6);
    // SetCenter(optix::make_float3(0.0));
}
void sdfCPKMol::SetType(std::vector<int> type) {
    typeBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT, type.size());

    int p;
    int *pos = reinterpret_cast<int*> (typeBuffer->map());
    for (int i = 0, index = 0; i < static_cast<int>(type.size()); ++i) {
        p = type[i];

        pos[index++] = p;

        if (m_typeRange.x > p) { m_typeRange.x = p; }
        if (m_typeRange.y < p) { m_typeRange.y = p; }
    }
    typeBuffer->unmap();

    optixSDFGeometry::GetOutput()["BSType"]->setBuffer(typeBuffer);
    //std::cout << "Type set" << std::endl;
    //primNumber = (int)(rad.size());
}

void sdfCPKMol::SetRadius(std::vector<float> rad) {
    /* radBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT, rad.size());
    void *dst = radBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
    memcpy(dst, rad.data(), sizeof(float) * rad.size());
    radBuffer->unmap();

    optixSDFGeometry::GetOutput()["BSRadius"]->setBuffer(radBuffer);*/
    radBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT, rad.size());
    float p;
    float *pos = reinterpret_cast<float*> (radBuffer->map());
    for (int i = 0, index = 0; i < static_cast<int>(rad.size()); ++i) {
        p = rad[i];

        pos[index++] = p;
    }
    radBuffer->unmap();

    optixSDFGeometry::GetOutput()["BSRadius"]->setBuffer(radBuffer);
    std::cout << "Radius set" << std::endl;
    primNumber = (int)(rad.size());
}

void sdfCPKMol::SimulateDynamic(int numFrames, std::vector<optix::float3> c)
{
    int num = c.size(); //number in one frame
    centersBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, numFrames*num);
    int index = 0;
    for (int nFr = 0; nFr < numFrames; nFr++)
    {
        float *pos = reinterpret_cast<float*> (centersBuffer->map());
        for (int i = 0; i < c.size(); ++i) {
            optix::float3 p = c[i];
            float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float r3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

            float v1 = r1 / 1000 * (i - 1) / 2; /// v1 in the range 0 to 99
            float v2 = r2 / 1000 * (i - 1) / 2;
            float v3 = r3 / 1000 * (i - 1) / 2;
            pos[index++] = p.x + v1 / 1000.0;
            pos[index++] = p.y + v2 / 1000.0;
            pos[index++] = p.z + v3 / 1000.0;
        }
        centersBuffer->unmap();

        optixSDFGeometry::GetOutput()["Positions"]->setBuffer(centersBuffer);
        std::cout << "Positions set" << std::endl;
    }
    optixSDFGeometry::GetOutput()["PNum"]->setInt(num);
    optixSDFGeometry::GetOutput()["TimeSound"]->setFloat(0);
    optixSDFGeometry::GetOutput()["numFrames"]->setInt(numFrames);
    SetDynamic(true);
    this->numFrames = numFrames;
    optixSDFGeometry::GetOutputDesc()->prog = optixSDFGeometry::GetIntersectionProg();
}
void sdfCPKMol::SetCenter(std::vector<optix::float3> c) {
    /*
    centersBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, c.size() / 3);
    void*dst = centersBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
    memcpy(dst, c.data(), sizeof(optix::float3) * c.size() / 3);
    centersBuffer->unmap();*/

    //fill the positions
    centersBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, c.size());

    float *pos = reinterpret_cast<float*> (centersBuffer->map());
    for (int i = 0, index = 0; i < static_cast<int>(c.size()); ++i) {
        optix::float3 p = c[i];

        pos[index++] = p.x;
        pos[index++] = p.y;
        pos[index++] = p.z;
    }
    centersBuffer->unmap();

    optixSDFGeometry::GetOutput()["Positions"]->setBuffer(centersBuffer);
    std::cout << "Positions set" << std::endl;
    optixSDFGeometry::GetOutput()["PNum"]->setInt(c.size());

    optixSDFGeometry::GetOutput()["TimeSound"]->setFloat(0.0);
    optixSDFGeometry::GetOutput()["numFrames"]->setInt(0);
    SetDynamic(false);
}

//------------------------------------
//Dynamic Molecule primitive

void sdfCPKDynMol::Initialize()
{
    //creates geometry

    std::cout << "Programs initialized" << std::endl;
    //init geometry
    // Geometry
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            optixSDFGeometry::InitProg("boundingbox_dyn_mol", "intersection_sphere_dynamic.cu", "boundingbox_dyn_mol");
            optixSDFGeometry::InitProg("intersection_dyn_mol", "intersection_sphere_dynamic.cu", "intersection_dyn_mol");
            //m_mapOfPrograms["boundingbox_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("boundingbox_triangle_indexed.cu"), "boundingbox_sdf_sphere");
            // m_mapOfPrograms["intersection_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("intersection_triangle_indexed.cu"), "intersection_sdf_sphere");
            optixSDFGeometry::SetBoundingBoxProgName("boundingbox_dyn_mol");
            optixSDFGeometry::SetIntersectionProgName("intersection_dyn_mol");

            //compile primitive program
            optixSDFGeometry::InitProg("sdfDynSphere", "intersection_sdfPrimLib.cu", "sdfDynSphere");
            optixSDFGeometry::SetCallableProgName("sdfDynSphere");
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;

    // Set default parameters
    //NO default parameters
    // SetRadius(0.6);
    // SetCenter(optix::make_float3(0.0));
}

void sdfCPKDynMol::SetCenter2(std::vector<optix::float3> c) {
    /*
    centersBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, c.size() / 3);
    void*dst = centersBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
    memcpy(dst, c.data(), sizeof(optix::float3) * c.size() / 3);
    centersBuffer->unmap();*/

    //fill the positions
    centersBuffer2 = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, c.size());

    float *pos = reinterpret_cast<float*> (centersBuffer2->map());
    for (int i = 0, index = 0; i < static_cast<int>(c.size()); ++i) {
        optix::float3 p = c[i];

        pos[index++] = p.x;
        pos[index++] = p.y;
        pos[index++] = p.z;
    }
    centersBuffer2->unmap();

    optixSDFGeometry::GetOutput()["Positions2"]->setBuffer(centersBuffer2);
    std::cout << "Positions2 set" << std::endl;
    SetParameters();
}

void sdfCPKDynMol::SetParameters() {
    //TODO curently here
    optix::Program pr = optixSDFGeometry::GetCallableProg();

    SetDynamic(true);

    pr["TimeSound"]->setFloat(0.0);
    //std::min

    GetOutputDesc()->prog = pr;

    //SetDynamic(true);
    //optix::Program pr = optixSDFGeometry::GetCallableProg();
    //pr["timeSound"]->setFloat(0.0);
    //GetOutputDesc()->prog = pr;
}

//------------------------------------
//BallSticks Molecule primitive

void sdfBallSticksMol::Initialize()
{
    //creates geometry

    std::cout << "Programs initialized" << std::endl;
    //init geometry
    // Geometry
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            optixSDFGeometry::InitProg("boundingbox_bond_mol", "intersection_sphere_bond.cu", "boundingbox_bond_mol");
            optixSDFGeometry::InitProg("intersection_bond_mol", "intersection_sphere_bond.cu", "intersection_bond_mol");
            //m_mapOfPrograms["boundingbox_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("boundingbox_triangle_indexed.cu"), "boundingbox_sdf_sphere");
            // m_mapOfPrograms["intersection_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("intersection_triangle_indexed.cu"), "intersection_sdf_sphere");
            optixSDFGeometry::SetBoundingBoxProgName("boundingbox_bond_mol");
            optixSDFGeometry::SetIntersectionProgName("intersection_bond_mol");

            //compile primitive program
            optixSDFGeometry::InitProg("sdfBondSphere", "intersection_sdfPrimLib.cu", "sdfBondSphere");
            optixSDFGeometry::SetCallableProgName("sdfBondSphere");
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;

    // Set default parameters
    //NO default parameters
    // SetRadius(0.6);
    // SetCenter(optix::make_float3(0.0));
}

void sdfBallSticksMol::SetBonds(std::vector<optix::int2> c) {
    //fill the positions
    bondsBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT2, c.size());

    int *pos = reinterpret_cast<int*> (bondsBuffer->map());
    for (int i = 0, index = 0; i < static_cast<int>(c.size()); ++i) {
        optix::int2 p = c[i];

        pos[index++] = (p.x);
        pos[index++] = (p.y);
    }
    bondsBuffer->unmap();

    optixSDFGeometry::GetOutput()["Bonds"]->setBuffer(bondsBuffer);
    std::cout << "Bonds set" << std::endl;
    SetParameters();
}

void sdfBallSticksMol::SetParameters() {
    //TODO curently here
     /*optix::Program pr = optixSDFGeometry::GetCallableProg();
    pr["TimeSound"]->setFloat(0.0);
    if (numFrames > 0) {
        //dynamic
    }
    else {
        pr["numFrames"]->setInt(0);
    }

    SetDynamic(true);

    pr["TimeSound"]->setFloat(0.0);
    //std::min

    GetOutputDesc()->prog = pr;

    //SetDynamic(true);
    //optix::Program pr = optixSDFGeometry::GetCallableProg();
    //pr["timeSound"]->setFloat(0.0);
    //GetOutputDesc()->prog = pr;*/
}