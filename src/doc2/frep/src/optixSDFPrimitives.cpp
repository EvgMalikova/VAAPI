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
//SDF Box prim

void sdfMicrosturcture::InitCallableProg()
{
    optixSDFGeometry::InitProg("sdfMicrostructure", "intersection_sdfPrimLib.cu", "sdf_sphere");
    optixSDFGeometry::SetCallableProgName("sdf_sphere");
}

void sdfMicrosturcture::SetCenter1(optix::float3 center)
{
    optix::Program pr = GetCallableProg();
    pr["varCenter"]->setFloat(center.x, center.y, center.z);
    optixSDFPrimitive::SetCenter(center);
    //optixSDFGeometry::GetOutput()["varCenter"]->setFloat(center.x, center.y, center.z);
    //std::cout << pr["varCenter"]->getFloat() << std::endl;
}

void sdfMicrosturcture::SetDims(optix::float3 radius)
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

//-------------------------------------
//SDF Hand

void optixSDFHand::InitCallableProg()
{
    optixSDFGeometry::InitProg("sdfHand", "intersection_sdfPrimLib.cu", "sdf_hand");
    optixSDFGeometry::SetCallableProgName("sdf_hand");
}

void optixSDFHand::SetCenter1(optix::float3 center)
{
    optix::Program pr = GetCallableProg();
    pr["varCenter"]->setFloat(center.x, center.y, center.z);
    optixSDFPrimitive::SetCenter(center);
    for (int i = 0; i < 5; i++) {
        std::string name = "varCenter" + std::to_string(i);

        pr[name]->setFloat(center.x, center.y, center.z);
    }
    //optixSDFGeometry::GetOutput()["varCenter"]->setFloat(center.x, center.y, center.z);
    //std::cout << pr["varCenter"]->getFloat() << std::endl;
}

void optixSDFHand::SetRadius1(optix::float3 radius)
{
    //only first two are used
    optix::Program pr = GetCallableProg();
    pr["varRadius"]->setFloat(radius.x, radius.y, radius.z);
    optixSDFPrimitive::SetRadius(radius);
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
    std::string progName = "sdfPrim";// +m_rogType;
    optix::Program pr = optixSDFGeometry::GetCallableProg();
    if (pr->get() != nullptr) {
        optixSDFGeometry::GetOutput()[progName]->setProgramId(pr);
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
    std::cout << "Type range " << m_typeRange.x << ", " << m_typeRange.y << std::endl;
    primNumber = (int)(type.size());
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
        p = rad[i] * m_scale;

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
    float *pos = reinterpret_cast<float*> (centersBuffer->map());

    for (int nFr = 0; nFr < numFrames; nFr++)
    {
        for (int i = 0; i < c.size(); ++i) {
            optix::float3 p = c[i];
            float r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float r3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            int sng1 = 1;
            int sng2 = -1;
            int sng3 = 1;
            if (r2 > 0.5) sng3 = -1;
            if (r3 < 0.5) sng1 = -1;
            if (r2 + r3 - r1 > 0.5) sng2 = 1;
            //std::cout << "R1=" << r1 << std::endl;
            float v1 = r1  * (2 * nFr)*sng1; /// v1 in the range 0 to 99
            float v2 = r2  * (2 * nFr)*sng2;
            float v3 = r3  * (2 * nFr)*sng3;

            pos[index++] = p.x + v1 / 7.0;
            pos[index++] = p.y + v2 / 7.0;
            pos[index++] = p.z + v3 / 7.0;
        }
    }
    centersBuffer->unmap();
    optixSDFGeometry::GetOutput()["Positions"]->setBuffer(centersBuffer);
    std::cout << "Positions set" << std::endl;

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

        pos[index++] = p.x + m_shift.x;
        pos[index++] = p.y + m_shift.y;
        pos[index++] = p.z + m_shift.z;
    }
    centersBuffer->unmap();

    optixSDFGeometry::GetOutput()["Positions"]->setBuffer(centersBuffer);
    std::cout << "Positions set" << std::endl;
    optixSDFGeometry::GetOutput()["PNum"]->setInt(c.size());

    optixSDFGeometry::GetOutput()["TimeSound"]->setFloat(0.0);
    optixSDFGeometry::GetOutput()["numFrames"]->setInt(0);
    SetDynamic(false);
}
void sdfCPKMol::SetScale(float sc) {
    m_scale = sc;
}

void sdfCPKMol::SetShift(optix::float3 sc) {
    m_shift = sc;
}
void sdfCPKMol::SetCenter(std::vector<optix::float3> c, int frame) {
    if (frame == 1) {
        centersBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, c.size()*numFrames);
    }
    int num = c.size();
    int index = 3 * c.size()*(frame - 1);
    float *pos = reinterpret_cast<float*> (centersBuffer->map());
    for (int i = 0; i < static_cast<int>(c.size()); ++i) {
        optix::float3 p = c[i];

        pos[index++] = p.x + m_shift.x;
        pos[index++] = p.y + m_shift.y;
        pos[index++] = p.z + m_shift.z;
    }
    centersBuffer->unmap();

    optixSDFGeometry::GetOutput()["Positions"]->setBuffer(centersBuffer);
    std::cout << "Positions set" << std::endl;
    if (frame == numFrames) {
        optixSDFGeometry::GetOutput()["PNum"]->setInt(num);
        optixSDFGeometry::GetOutput()["TimeSound"]->setFloat(0);
        optixSDFGeometry::GetOutput()["numFrames"]->setInt(numFrames);
        SetDynamic(true);

        optixSDFGeometry::GetOutputDesc()->prog = optixSDFGeometry::GetIntersectionProg();
    }
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

void sdfBallSticksMol::SetCallableProg()
{
    std::string progName = "sdfPrim";// +m_rogType;
    optix::Program pr = optixSDFGeometry::GetCallableProg();
    if (pr->get() != nullptr) {
        optixSDFGeometry::GetOutput()[progName]->setProgramId(pr);
        // it->second->getId()
    }
}

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

            //TODO: process programs
            // save their names
            optixSDFGeometry::InitProg("BVInt", "intersection_sphere_bond.cu", "bondProg");
            optixSDFGeometry::InitProg("ReadData", "intersection_sphere_bond.cu", "readTimeData");
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
    SetPrimNumber(int(c.size()));
    //primNumber = c.size();
    SetParameters();
}

optix::Program sdfBallSticksMol::GetProgByName(std::string name)
{
    std::map<std::string, optix::Program>::const_iterator it = m_mapOfPrograms.find(name);
    if (it != m_mapOfPrograms.end()) {
        return it->second;
    }
    else {
        std::cout << "No program" << std::endl;
        return nullptr;
    }
}
void sdfBallSticksMol::SetParameters() {
    optix::Program pr = GetProgByName("bondProg");
    optix::Program prInter = optixSDFGeometry::GetIntersectionProg();
    RTvariable varProg;
    rtProgramDeclareVariable(prInter->get(), "boundIntersection", &varProg);
    if (pr.get() != nullptr)
        rtVariableSetObject(varProg, pr->get());

    //Read data
    optix::Program pr2 = GetProgByName("readTimeData");
    RTvariable varProg2;
    rtProgramDeclareVariable(prInter->get(), "getTimeData", &varProg2);
    if (pr.get() != nullptr)
        rtVariableSetObject(varProg2, pr2->get());

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

//------------------------------------
//Molecule BallSticks Molecule primitive
void sdfMolBallSticksMol::Initialize()
{
    //creates geometry

    std::cout << "Programs initialized" << std::endl;
    //init geometry
    // Geometry
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            optixSDFGeometry::InitProg("boundingbox_molecules", "intersection_molecules.cu", "boundingbox_molecules");
            optixSDFGeometry::InitProg("intersection_molecules", "intersection_molecules.cu", "intersection_molecules");
            //m_mapOfPrograms["boundingbox_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("boundingbox_triangle_indexed.cu"), "boundingbox_sdf_sphere");
            // m_mapOfPrograms["intersection_sdf_sphere"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("intersection_triangle_indexed.cu"), "intersection_sdf_sphere");
            optixSDFGeometry::SetBoundingBoxProgName("boundingbox_molecules");
            optixSDFGeometry::SetIntersectionProgName("intersection_molecules");

            //compile primitive program
            optixSDFGeometry::InitProg("sdfMolBondSphere", "intersection_sdfPrimLib.cu", "sdfMolBondSphere");
            optixSDFGeometry::SetCallableProgName("sdfMolBondSphere");
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

void sdfMolBallSticksMol::SetMols(std::vector<Molecule> c, int maxMolSize) {
    //fill the positions
    molsBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT2, c.size());

    int *pos = reinterpret_cast<int*> (molsBuffer->map());
    for (int i = 0, index = 0; i < static_cast<int>(c.size()); ++i) {
        optix::int2 p = optix::make_int2(c[i].bond_id[0], c[i].bond_id[1]);

        pos[index++] = (p.x);
        pos[index++] = (p.y);
    }
    molsBuffer->unmap();

    optixSDFGeometry::GetOutput()["Mols"]->setBuffer(molsBuffer);
    std::cout << "Molecules set" << std::endl;

    //total number of primities are molecules
    SetPrimNumber(int(c.size()));
}

//*------------Heterogeneous objects framework
//------------------------------------
/*------
Basic abstract class sdfHeterogeneous
------*/

void sdfHeterogeneous::SetCallableProg()
{
    std::string progName = "sdfPrim";// +m_rogType;
    optix::Program pr = optixSDFGeometry::GetCallableProg();
    if (pr->get() != nullptr) {
        optixSDFGeometry::GetOutput()[progName]->setProgramId(pr);
        optixSDFGeometry::GetIntersectionProg()[progName]->setProgramId(pr);
    }
}

void sdfHeterogeneous::CreateGeometry()
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

/* Builds the main program for intersecton*/
void sdfHeterogeneous::InitMainIntersectionProg()
{
    optixSDFGeometry::InitProg("intersection_mol", "intersection_hetero_indexed.cu", "intersection_mol");
    optixSDFGeometry::SetIntersectionProgName("intersection_mol");
}
void sdfHeterogeneous::Initialize()
{
    //creates geometry

    // std::cout << "Programs initialized" << std::endl;
    //init geometry
    // Geometry
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitMainIntersectionProg();
            InitBoundingBoxProg();

            //compile primitive program
            InitSDFPrimitiveProg();

            //constructive tree subbounding volumes
            InitConstructiveTreeOptimisationProg();

            //Read data procedure
            InitReadDataProg();
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

optix::Program sdfHeterogeneous::GetProgByName(std::string name)
{
    std::map<std::string, optix::Program>::const_iterator it = m_mapOfPrograms.find(name);
    if (it != m_mapOfPrograms.end()) {
        return it->second;
    }
    else {
        std::cout << "No program" << std::endl;
        return nullptr;
    }
}
void sdfHeterogeneous::SetParameters() {
    optix::Program pr = GetProgByName(GetConstructiveTreeOptimisationProgName());
    optix::Program prInter = optixSDFGeometry::GetIntersectionProg();
    RTvariable varProg;
    rtProgramDeclareVariable(prInter->get(), "boundIntersection", &varProg);
    if (pr.get() != nullptr)
        rtVariableSetObject(varProg, pr->get());

    //Read data
    optix::Program pr2 = GetProgByName(GetReadDataProgName());
    RTvariable varProg2;
    rtProgramDeclareVariable(prInter->get(), "getTimeData", &varProg2);
    if (pr.get() != nullptr)
        rtVariableSetObject(varProg2, pr2->get());
}

/*----------
*sdfHCPKMol
-----------*/

void sdfHCPKMol::InitBoundingBoxProg()
{
    optixSDFGeometry::InitProg("boundingbox_mol", "intersection_sphere_indexed.cu", "boundingbox_mol");
    optixSDFGeometry::SetBoundingBoxProgName("boundingbox_mol");
}

void sdfHCPKMol::InitSDFPrimitiveProg()
{
    optixSDFGeometry::InitProg("sdfAtom", "intersection_sdfPrimLib.cu", "sdf_sphere");
    optixSDFGeometry::SetCallableProgName("sdf_sphere");
}

void sdfHCPKMol::InitConstructiveTreeOptimisationProg()
{
    optixSDFGeometry::InitProg("BVInt", "intersection_hetero_indexed.cu", "bondProg");
    SetConstructiveTreeOptimisationProgName("bondProg");
}

//Read data procedure
void sdfHCPKMol::InitReadDataProg()
{
    optixSDFGeometry::InitProg("ReadData", "intersection_hetero_indexed.cu", "readTimeData");
    SetReadDataProgName("readTimeData");
}
void sdfHCPKMol::SetType(std::vector<int> type) {
    typeBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT, type.size());

    int p;
    int *pos = reinterpret_cast<int*> (typeBuffer->map());
    for (int i = 0, index = 0; i < static_cast<int>(type.size()); ++i) {
        p = type[i];

        pos[index++] = p;
        optix::float2* typeR = GetTypeRange();
        if (typeR->x > p) { typeR->x = p; }
        if (typeR->y < p) { typeR->y = p; }
    }
    typeBuffer->unmap();

    optixSDFGeometry::GetOutput()["BSType"]->setBuffer(typeBuffer);
    // std::cout << "Type range " << m_typeRange.x << ", " << m_typeRange.y << std::endl;
    SetPrimNumber((int)(type.size()));
}

void sdfHCPKMol::SetRadius(std::vector<float> rad) {
    /* radBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT, rad.size());
    void *dst = radBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
    memcpy(dst, rad.data(), sizeof(float) * rad.size());
    radBuffer->unmap();

    optixSDFGeometry::GetOutput()["BSRadius"]->setBuffer(radBuffer);*/
    radBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT, rad.size());
    float p;
    float *pos = reinterpret_cast<float*> (radBuffer->map());
    for (int i = 0, index = 0; i < static_cast<int>(rad.size()); ++i) {
        p = rad[i] * m_scale;

        pos[index++] = p;
    }
    radBuffer->unmap();

    optixSDFGeometry::GetOutput()["BSRadius"]->setBuffer(radBuffer);
    std::cout << "Radius set" << std::endl;

    SetPrimNumber((int)(rad.size()));
}

void sdfHCPKMol::SetCenter(std::vector<optix::float3> c) {
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

        pos[index++] = p.x + m_shift.x;
        pos[index++] = p.y + m_shift.y;
        pos[index++] = p.z + m_shift.z;
    }
    centersBuffer->unmap();

    optixSDFGeometry::GetOutput()["Positions"]->setBuffer(centersBuffer);
    std::cout << "Positions set" << std::endl;
    optixSDFGeometry::GetOutput()["PNum"]->setInt(c.size());

    optixSDFGeometry::GetOutput()["TimeSound"]->setFloat(0.0);
    optixSDFGeometry::GetOutput()["numFrames"]->setInt(0);
    SetDynamic(false);
}
void sdfHCPKMol::SetScale(float sc) {
    m_scale = sc;
}

void sdfHCPKMol::SetShift(optix::float3 sc) {
    m_shift = sc;
}
void sdfHCPKMol::SetCenter(std::vector<optix::float3> c, int frame) {
    if (frame == 1) {
        centersBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, c.size()*GetNumFrames());
    }
    int num = c.size();
    int index = 3 * c.size()*(frame - 1);
    float *pos = reinterpret_cast<float*> (centersBuffer->map());
    for (int i = 0; i < static_cast<int>(c.size()); ++i) {
        optix::float3 p = c[i];

        pos[index++] = p.x + m_shift.x;
        pos[index++] = p.y + m_shift.y;
        pos[index++] = p.z + m_shift.z;
    }
    centersBuffer->unmap();

    optixSDFGeometry::GetOutput()["Positions"]->setBuffer(centersBuffer);
    std::cout << "Positions set" << std::endl;
    if (frame == GetNumFrames()) {
        optixSDFGeometry::GetOutput()["PNum"]->setInt(num);
        optixSDFGeometry::GetOutput()["TimeSound"]->setFloat(0);
        optixSDFGeometry::GetOutput()["numFrames"]->setInt(GetNumFrames());
        SetDynamic(true);

        optixSDFGeometry::GetOutputDesc()->prog = optixSDFGeometry::GetIntersectionProg();
    }
}

//*------------Heterogeneous objects framework
//------------------------------------
//Microstructures primitive

void sdfHMicrostructure::InitSDFPrimitiveProg()
{
    optixSDFGeometry::InitProg("sdfMicro", "intersection_sdfPrimLib.cu", "sdf_micro");
    optixSDFGeometry::SetCallableProgName("sdf_micro");
}

/*----------
*sdfHTetra
-----------*/

void sdfHTetra::InitBoundingBoxProg()
{
    optixSDFGeometry::InitProg("boundingbox_mol", "intersection_sphere_indexed.cu", "boundingbox_mol");
    optixSDFGeometry::SetBoundingBoxProgName("boundingbox_mol");
}

void sdfHTetra::InitSDFPrimitiveProg()
{
    optixSDFGeometry::InitProg("sdfAtom", "intersection_sdfPrimLib.cu", "sdf_sphere");
    optixSDFGeometry::SetCallableProgName("sdf_sphere");
}

void sdfHTetra::InitConstructiveTreeOptimisationProg()
{
    optixSDFGeometry::InitProg("BVInt", "intersection_hetero_indexed.cu", "bondProg");
    SetConstructiveTreeOptimisationProgName("bondProg");
}

//Read data procedure
void sdfHTetra::InitReadDataProg()
{
    optixSDFGeometry::InitProg("ReadData", "intersection_hetero_indexed.cu", "readTimeData");
    SetReadDataProgName("readTimeData");
}