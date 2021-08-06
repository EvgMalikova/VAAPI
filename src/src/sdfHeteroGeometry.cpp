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
#include "sdfHeteroGeometry.h"

//*------------Heterogeneous objects framework
//------------------------------------
/*------
Basic abstract class sdfHeterogeneous
------*/
std::string sdfHeterogeneous::GetPrimProgramName(ObjectType type)
{
    switch (type) {
    case ObjectType::GENERAL:
        return "sdfPrimDefault";
        break;
    case ObjectType::DIM_0D:
        return "sdfPrim0";
        break;
    case ObjectType::DIM_1D:
        return "sdfPrim1";
        break;
    case ObjectType::MULTISCALE:
        return "sdfPrim3";
        break;

    case ObjectType::DIM_4D:
        return "sdfPrim4";
        break;
    }
}

std::string sdfHeterogeneous::GetPrimProgName()
{
    ObjectType type = GetPrimType();
    return GetPrimProgramName(type);
}
void sdfHeterogeneous::SetCallableProg()
{
    std::string progName = GetPrimProgName();// "sdfPrim";// +m_rogType;
    optix::Program pr = optixSDFGeometry::GetCallableProg();
    if (pr->get() != nullptr) {
        optixSDFGeometry::GetOutput()[progName]->setProgramId(pr);
        // optixSDFGeometry::GetIntersectionProg()[progName]->setProgramId(pr);
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

void sdfHeterogeneous::SetShift(optix::float3 sc) {
    m_shift = sc;
}

void sdfHeterogeneous::SetCenter(std::vector<optix::float3> c, int frame) {
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
void sdfHeterogeneous::SetCenter(std::vector<optix::float3> c) {
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
    if (pr2.get() != nullptr)
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
void sdfHCPKMol::SetScale(float sc) {
    m_scale = sc;
}

//*------------Heterogeneous objects framework
//------------------------------------
//Microstructures primitive

void sdfHMicrostructure::InitSDFPrimitiveProg()
{
    optixSDFGeometry::InitProg("sdfMicro", "intersection_sdfPrimLib.cu", "sdf_micro");
    optixSDFGeometry::SetCallableProgName("sdf_micro");
}

void sdfHMicro::InitSDFPrimitiveProg()
{
    optixSDFGeometry::InitProg("sdfMicrostructure2", "intersection_sdfPrimLib.cu", "sdf_micro");
    optixSDFGeometry::SetCallableProgName("sdf_micro");
}

/*----------
*sdfHeterogeneous0D
-----------*/
/* Builds the main program for intersecton*/
void sdfHeterogeneous0D::InitMainIntersectionProg()
{
    optixSDFGeometry::InitProg("intersection_mol", "intersection_hetero0D.cu", "intersection_mol");
    optixSDFGeometry::SetIntersectionProgName("intersection_mol");
}
std::string sdfHeterogeneous0D::GetHeteroName()
{
    sdfHeterogeneous::ObjectType type = GetPrimType();;
    switch (type) {
    case ObjectType::GENERAL:
        return "HeterogeneousGeneral";
        break;
    case ObjectType::DIM_0D:
        return "Heterogeneous0D";
        break;
    case ObjectType::DIM_1D:
        return "Heterogeneous1D";
        break;
    case ObjectType::DIM_4D:
        return "Heterogeneous4D";
        break;
    }
}

void sdfHeterogeneous0D::Initialize()
{
    //creates geometry
    std::string name = GetHeteroName();

    std::cout << "Programs " << name << " initialized" << std::endl;
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
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}

void sdfHeterogeneous0D::InitBoundingBoxProg()
{
    optixSDFGeometry::InitProg("primitive_bounds", "intersection_hetero0D.cu", "boundingbox_mol");
    optixSDFGeometry::SetBoundingBoxProgName("boundingbox_mol");
}

void sdfHeterogeneous0D::InitSDFPrimitiveProg()
{
    optixSDFGeometry::InitProg("sdfSphere2", "intersection_sdfPrimLib.cu", "sdf_sphere");
    optixSDFGeometry::SetCallableProgName("sdf_sphere");
}

/* Reading functions*/
void sdfHeterogeneous0D::SetType(std::vector<int> type) {
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

void sdfHeterogeneous0D::SetRadius(std::vector<float> rad) {
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
void sdfHeterogeneous0D::SetScale(float sc) {
    m_scale = sc;
}
/*----------
*sdfHeterogeneous1D
-----------*/
void sdfHeterogeneous1D::InitMainIntersectionProg()
{
    optixSDFGeometry::InitProg("intersection_mol", "intersection_hetero1D.cu", "intersection_mol");
    optixSDFGeometry::SetIntersectionProgName("intersection_mol");
}

void sdfHeterogeneous1D::InitBoundingBoxProg()
{
    optixSDFGeometry::InitProg("boundingbox_bond_mol", "intersection_hetero1D.cu", "boundingbox_mol");
    optixSDFGeometry::SetBoundingBoxProgName("boundingbox_mol");
}

void sdfHeterogeneous1D::SetBonds(std::vector<optix::int2> c) {
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
/*----------
*sdfHBondsSticks
-----------*/
void sdfHBondsSticks::InitSDFPrimitiveProg()
{
    optixSDFGeometry::InitProg("sdfBondSphereBlob", "intersection_sdfPrimLib.cu", "bond");
    optixSDFGeometry::SetCallableProgName("bond");

    float blob = 0.2;
    GetCallableProg()["varBlob"]->setFloat(blob);

    /*increase boundary for intersection as well*/

    GetOutput()["blendAdd"]->setFloat(blob / 6);
    //optixSDFGeometry::InitProg("sdfBondSphereBlob", "intersection_sdfPrimLib.cu", "simple");
}

void sdfHCrazyBonds::InitSDFPrimitiveProg()
{
    optixSDFGeometry::InitProg("sdfBondSphere", "intersection_sdfPrimLib.cu", "bond");
    optixSDFGeometry::SetCallableProgName("bond");

    float blob = 0.1;
    GetCallableProg()["varBlob"]->setFloat(blob);

    /*increase boundary for intersection as well*/

    GetOutput()["blendAdd"]->setFloat(0.3);
    //optixSDFGeometry::InitProg("sdfBondSphereBlob", "intersection_sdfPrimLib.cu", "simple");
}

/*----------
*sdfMolecule
-----------*/

void sdfMoleculeBallSticks::InitMainIntersectionProg()
{
    optixSDFGeometry::InitProg("intersection_molecules", "intersection_molecules.cu", "intersection_mol");
    optixSDFGeometry::SetIntersectionProgName("intersection_mol");
}

void sdfMoleculeBallSticks::InitBoundingBoxProg()
{
    optixSDFGeometry::InitProg("boundingbox_molecules", "intersection_molecules.cu", "boundingbox_mol");
    optixSDFGeometry::SetBoundingBoxProgName("boundingbox_mol");
}

void sdfMoleculeBallSticks::InitSDFPrimitiveProg()
{
    optixSDFGeometry::InitProg("sdfMolBondSphere", "intersection_sdfPrimLib.cu", "sdfMolBondSphere");
    optixSDFGeometry::SetCallableProgName("sdfMolBondSphere");
}

void sdfMoleculeBallSticks::Initialize()
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

void sdfMoleculeBallSticks::SetMols(std::vector<Molecule> c, int maxMolSize) {
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

/*----------
*sdfHeterogeneous4D
-----------*/

void sdfHeterogeneous4D::InitSDFPrimitiveProg()
{
    optixSDFGeometry::InitProg("sdfTetra", "intersection_sdfPrimLib.cu", "bond");
    optixSDFGeometry::SetCallableProgName("bond");

    //GetCallableProg()["varBlob"]->setFloat(0.4);

    //optixSDFGeometry::InitProg("sdfBondSphereBlob", "intersection_sdfPrimLib.cu", "simple");
}
void sdfHeterogeneous4D::InitMainIntersectionProg()
{
    optixSDFGeometry::InitProg("intersection_mol", "intersection_hetero4D.cu", "intersection_mol");
    optixSDFGeometry::SetIntersectionProgName("intersection_mol");
}

void sdfHeterogeneous4D::InitBoundingBoxProg()
{
    optixSDFGeometry::InitProg("primitive_bounds", "intersection_hetero4D.cu", "boundingbox_mol");
    optixSDFGeometry::SetBoundingBoxProgName("boundingbox_mol");
}

void sdfHeterogeneous4D::SetTetras(std::vector<optix::int4> c) {
    //fill the positions
    tetBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT4, c.size());

    int *pos = reinterpret_cast<int*> (tetBuffer->map());
    for (int i = 0, index = 0; i < static_cast<int>(c.size()); ++i) {
        optix::int4 p = c[i];

        pos[index++] = (p.x);
        pos[index++] = (p.y);
        pos[index++] = (p.z);
        pos[index++] = (p.w);
    }
    tetBuffer->unmap();

    optixSDFGeometry::GetOutput()["Tets"]->setBuffer(tetBuffer);
    std::cout << "Tetrahedras set" << std::endl;
    SetPrimNumber(int(c.size()));
    //primNumber = c.size();
    SetParameters();
}

/*----------
*sdfHBallSticksMol
-----------*/
/* Builds the main program for intersecton*/
void sdfHBallSticksMol::InitMainIntersectionProg()
{
    optixSDFGeometry::InitProg("intersection_mol", "intersection_hetero1D.cu", "intersection_mol");
    optixSDFGeometry::SetIntersectionProgName("intersection_mol");
}

void sdfHBallSticksMol::Initialize()
{
    //creates geometry

    std::cout << "Programs initialized" << std::endl;
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
void sdfHBallSticksMol::SetBonds(std::vector<optix::int2> c) {
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

void sdfHBallSticksMol::InitBoundingBoxProg()
{
    optixSDFGeometry::InitProg("boundingbox_bond_mol", "intersection_hetero1D.cu", "boundingbox_mol");
    optixSDFGeometry::SetBoundingBoxProgName("boundingbox_mol");
}

void sdfHBallSticksMol::InitSDFPrimitiveProg()
{
    optixSDFGeometry::InitProg("sdfBondBlob", "intersection_sdfPrimLib.cu", "sdf_sphere");
    optixSDFGeometry::SetCallableProgName("sdf_sphere");

    optixSDFGeometry::InitProg("sdfBondSphereBlob", "intersection_sdfPrimLib.cu", "simple");
}

void sdfHBallSticksMol::SetCallableProg()
{
    std::string progName = "sdfPrim1";// +m_rogType;
    optix::Program pr = GetCallableProgSimple();
    if (pr->get() != nullptr) {
        optixSDFGeometry::GetOutput()[progName]->setProgramId(pr);
        //     optixSDFGeometry::GetIntersectionProg()[progName]->setProgramId(pr);
    }
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
