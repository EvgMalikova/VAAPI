#include "optixAbstractMaterial.h"

void vaAuditoryMaterial::SetLights()
{
    optix::Buffer attributesBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER);
    attributesBuffer->setElementSize(sizeof(BasicLight));
    attributesBuffer->setSize(m_lights.size());

    void *dst = attributesBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
    memcpy(dst, m_lights.data(), sizeof(BasicLight) * m_lights.size());
    attributesBuffer->unmap();

    GetOutput()["lights"]->setBuffer(attributesBuffer);
}

void vaAuditoryMaterial::SetClosestHitProgName(std::string name)
{
    closesthit_prog = name;
    m_isClosestHit = true;
}

void vaAuditoryMaterial::SetAnyHitProgName(std::string name)
{
    anyhit_prog = name;
    m_isAnyHit = true;
}

void vaAuditoryMaterial::SetDynamicProgName(std::string name)
{
    dynamic_prog = name;
    m_isDynamic = true;
}

void vaAuditoryMaterial::InitProg(std::string prog, std::string file, std::string name)
{
    m_mapOfPrograms[name] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath(file), prog);
    names.push_back(name);
}

optix::Program vaAuditoryMaterial::GetProgByName(std::string name)
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

void vaAuditoryMaterial::SetClosestHitProg(optix::Material m)
{
    std::map<std::string, optix::Program>::const_iterator it;
    it = m_mapOfPrograms.find(GetClosestHitProgName());
    if (it != m_mapOfPrograms.end());
    {
        if ((m.get() != nullptr) && ((it->second).get() != nullptr))
            m->setClosestHitProgram(m_rayType, it->second);
    }
}

void vaAuditoryMaterial::SetDynamicProg(optix::Material m)
{
    std::map<std::string, optix::Program>::const_iterator it;
    it = m_mapOfPrograms.find(GetDynamicProgName());
    if (it != m_mapOfPrograms.end());
    m["dynamic"]->setProgramId(it->second);
    //m->setCl(0, it->second);
}

void vaAuditoryMaterial::SetAnyHitProg(optix::Material m)
{
    //m->setClosestHitProgram(0, vaBasicObject::GetContext()->createProgramFromPTXFile(ptx, program));
    std::map<std::string, optix::Program>::const_iterator it;
    it = m_mapOfPrograms.find(GetAnyHitProgName());
    if (it != m_mapOfPrograms.end());
    m->setAnyHitProgram(m_rayType, it->second);
    //mat->setAnyHitProgram(0u, any_hit);
}

void vaAuditoryMaterial::Update()
{
    //compute all cuda connections

    //if (m->get() != nullptr) { //doesn't work
    Initialize(); //sets all variables

    m = vaBasicObject::GetContext()->createMaterial();

    //create lights for this material
    InitLights();
    // }
    //m->setClosestHitProgram(0, vaBasicObject::GetContext()->createProgramFromPTXFile(ptx, program));
    if (m_isClosestHit)
        SetClosestHitProg(m);

    if (m_isDynamic)
        SetDynamicProg(m);
    if (m_isAnyHit)
        SetAnyHitProg(m);

    //Set scalar or color mode
    ApplyScalarMode();

    //Set custom material variables
    SetMaterialParameters();

    //SetsBuffer
    /*int size = 10;
    optix::Buffer tf = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, size);
    optix::float3 p;
    float *pos = reinterpret_cast<float*> (tf->map());
    for (int i = 0, index = 0; i < static_cast<int>(size); ++i) {
        p = optix::make_float3(0);

        pos[index++] = p.x;
        pos[index++] = p.y;
        pos[index++] = p.z;
    }
    tf->unmap();

    m["TFBuffer"]->setBuffer(tf);*/

    //Set lights if defined in scene
    SetLights();
}

void vaMapper::Update() {
    try {
        //material creation
        //Initialize();
        //ComputeMaterial();

        //all materials should be set here
        //------
        gi = vaBasicObject::GetContext()->createGeometryInstance();
        gi->setGeometry(geo);
        gi->setMaterialCount(mat.size());
        for (int i = 0; i < mat.size(); i++) {
            //SetMaterialParameters(i);

            gi->setMaterial(i, mat[i]);
            std::cout << "added material " << i << std::endl;
        }
        //TODO:
        //Set some parameters for material here, previously defined with SetMacros
    }
    catch (optix::Exception& e)
    {
        std::cerr << e.getErrorString() << std::endl;
    }
} //this allows to set some material, from exturnal files

void vaMapper::SetOpticalModel()
{
    for (int i = 0; i < mat.size(); i++) {
        //SetMaterialParameters(i);
        if (m_desc[i].auditory == false)
        {
            //gi->setMaterial(i, mat[i]);
            geo["MaterialIndex"]->setInt(i);
            //std::cout << "set optical material " << i << std::endl;
        }
    }
}
void vaMapper::SetAuditoryModel()
{
    for (int i = 0; i < mat.size(); i++) {
        //std::cout << "param before " << m_desc[i].auditory<<","<< m_desc[i].dynamic<<std::endl;
        //SetMaterialParameters(i);
        if (m_desc[i].auditory == true)
        {
            //gi->setMaterial(i, mat[i]);
            geo["MaterialIndex"]->setInt(i);
        }
    }
}

void vaMapper::AddMaterial(optix::Material m, MaterialDesc desc) //instead of setInput
{
    mat.push_back(m);
    m_desc.push_back(desc);
}

void vaMapper::SetScalarModeOn()
{
    for (int i = 0; i < mat.size(); i++) {
        //std::cout << "param before " << m_desc[i].auditory<<","<< m_desc[i].dynamic<<std::endl;
        //SetMaterialParameters(i);
        if (m_desc[i].auditory == false)
        {
            //gi->setMaterial(i, mat[i]);
            mat[i]["useScalar"]->setInt(1);
        }
    }
}

optix::Material vaMapper::GetMaterial(int i)
{
    return mat[i];
}