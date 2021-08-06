#include "optixMaterial.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

void Lambertian::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitProg("closesthit", "closesthit.cu", "closesthit");
            SetClosestHitProgName("closesthit"); //sets closesthit to true;
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}
//--------------------
//vaBasicMaterial
//---------------------

void vaComplexVolumeMaterial::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitProg("anyhit_sdf_complex", "materials_closesthit.cu", "anyhit_sdf_complex");
            SetAnyHitProgName("anyhit_sdf_complex"); //sets closesthit to true;
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}

void vaComplexVolumeMaterial::SetMaterialParameters()
{
    vaBasicMaterial::SetMaterialParameters();
    //TODO: set SDF function
    //optix::Material m = GetMaterial(i);
   // GetOutput()["Type"]->setInt(int(m_type));

    //set primitive prog
    //the program for two variables
  /*  for (int i = 0; i < m_prog.size(); i++)
    {
        std::string name = "sdfPrim" + std::to_string(i);
        if (m_prog[i].get() != nullptr)

            GetOutput()[name]->setProgramId(m_prog[i]);
    }*/
    if (m_prog.get() != nullptr)

        GetOutput()["sdfPrim2"]->setProgramId(m_prog);
}

//--------------------
//vaBasicMaterial
//---------------------

void vaBasicMaterial::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitProg("closesthit_sdf", "materials_closesthit.cu", "closesthit_sdf");
            SetClosestHitProgName("closesthit_sdf"); //sets closesthit to true;
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}
void vaBasicMaterial::InitLights()
{
    BasicLight l;
    l.pos = optix::make_float3(50, 0, 0);
    l.color = optix::make_float3(1);
    m_lights.push_back(l);
    l.pos = optix::make_float3(0, 60, 0);
    l.color = optix::make_float3(0.5);
    m_lights.push_back(l);
}

void vaBasicMaterial::SetMaterialParameters()
{
    //optix::Material m = GetMaterial(i);
    GetOutput()["ambient_light_color"]->setFloat(1.0, 1.0, 0.0);
    if (m_ColorMap.get() != nullptr)
        GetOutput()["tFunction"]->setProgramId(m_ColorMap);
    //TODO: setupLights
}

//vaEAVolume
//---------------------

void vaEAVolume::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitProg("anyhitvolume_sdf", "materials_volumehit.cu", "any_hitvolume");
            SetClosestHitProgName("any_hitvolume"); //sets closesthit to true;
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}
void vaEAVolume::AddLight(BasicLight* l)
{
    m_lights.push_back(*l);
}
void vaEAVolume::SetSDFProg(optix::Program sdfProg)
{
    m_prog = sdfProg;
}
void vaEAVolume::SetMaterialParameters()
{
    //optix::Material m = GetMaterial(i);
    GetOutput()["Type"]->setInt(int(m_type));

    //set primitive prog
    if (m_prog.get() != nullptr)
        GetOutput()["sdfPrim"]->setProgramId(m_prog);
    //m["albedo"]->setFloat(1.0f, 0.5f, 0.0f);

    //TODO get callable prog with defined parameters from primitive
    // optix::Program pr = optixSDFGeometry::GetCallableProg();
    for (int i = 0; i < sampler.size(); i++) {
        if (i < 3) { //we can set only 3 samplers
            std::string samp_string = "tex" + std::to_string(i);
            GetOutput()[samp_string]->setTextureSampler(sampler[i]);
        }
        int num = std::min(3, int(sampler.size()));
        GetOutput()["numTexDefined"]->setInt(num);
        if (num > 1)
            SetDynamicTypeOn();
    }
}

//----------------------------------
void vaVolumeSDFComplex::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitProg("volumehit_complex_sdf", "materials_volumehit.cu", "volumehit_complex_sdf");
            SetAnyHitProgName("volumehit_complex_sdf"); //sets closesthit to true;
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}
void vaVolumeSDFComplex::SetMaterialParameters()
{
    //vaEAVolume::SetMaterialParameters();
}

//----------------------------------
void vaVolumeSDFHetero::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            switch (m_postprocess) {
            case 0:
                InitProg("volume_hetero_close", "materials_modelling.cu", "any_hitvolume");
                SetClosestHitProgName("any_hitvolume"); //sets closesthit to true;

                break;
            case 1:
                InitProg("volume_hetero_any", "materials_modelling.cu", "any_hitvolume");
                SetAnyHitProgName("any_hitvolume"); //sets closesthit to true;

                break;
            }
            CompileFunction();
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}

optix::Program vaVolumeSDFHetero::GetEvalProg()
{
    return GetProgByName("evalMaterial");
}
optix::Program vaVolumeSDFHetero::GetColorProg()
{
    return GetProgByName("colorMaterial");
}
std::string vaVolumeSDFHetero::GetEvalProgramName()
{
    switch (m_hType)
    {
    case sdfHeterogeneous::ObjectType::GENERAL:
        return "evalDefault";
        break;
    case sdfHeterogeneous::ObjectType::DIM_0D:
        return "eval0";
        break;
    case sdfHeterogeneous::ObjectType::DIM_1D:
        return "eval1";
        break;
    case sdfHeterogeneous::ObjectType::MULTISCALE:
        return "eval3";
        break;
    case sdfHeterogeneous::ObjectType::CELL:
        return "eval5";
        break;
    case sdfHeterogeneous::ObjectType::DIM_4D:
        return "eval4";
        break;
    }
}

void vaVolumeSDFHetero::CompileFunction()
{
    std::string nameProg = GetEvalProgramName();
    //TODO compile function depending on progName
    InitProg(nameProg, "materials_modelling.cu", "evalMaterial");
    //compile color prog
    InitProg("GetColorBlend", "materials_modelling.cu", "colorMaterial");
}

void vaVolumeSDFHetero::SetMaterialParameters()
{
    //set primitive prog
    std::string nameProg = sdfHeterogeneous::GetPrimProgramName(m_hType);
    //Set it to default as far
    //nameProg = sdfHeterogeneous::GetPrimProgramName(sdfHeterogeneous::ObjectType::GENERAL);

    optix::Program pr2 = GetProgByName("evalMaterial");

    //setEval
    optix::Program prInter = GetProgByName("any_hitvolume");
    /* RTvariable varProg2;
     rtProgramDeclareVariable(prInter->get(), "evalF", &varProg2);
     if (pr2.get() != nullptr)
         rtVariableSetObject(varProg2, pr2->get());
 */
    GetOutput()["evalF"]->setProgramId(pr2);
    if (m_prog.get() != nullptr) {
        //  GetOutput()[nameProg]->setProgramId(m_prog);
        pr2[nameProg]->setProgramId(m_prog);
    }
}

//----------------------------------
void vaVolumeSDFHeteroMultiscale::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            switch (m_postprocess) {
            case 0:
                InitProg("volume_hetero_close", "materials_modelling_multiscale.cu", "any_hitvolume");
                SetClosestHitProgName("any_hitvolume"); //sets closesthit to true;

                break;
            case 1:
                InitProg("volume_hetero_any", "materials_modelling_multiscale.cu", "any_hitvolume");
                SetAnyHitProgName("any_hitvolume"); //sets closesthit to true;

                break;
            }
            CompileFunction();
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}

void vaVolumeSDFHeteroMultiscale::CompileFunction()
{
    std::string nameProg = GetEvalProgramName();
    //TODO compile function depending on progName
    InitProg(nameProg, "materials_modelling_multiscale.cu", "evalMaterial");
    //compile color prog
    if (GetHeteroObjType() == sdfHeterogeneous::ObjectType::CELL) {
        InitProg("GetColorBlendCell", "materials_modelling_multiscale.cu", "colorMaterial");
    }
    else
        InitProg("GetColorBlend", "materials_modelling_multiscale.cu", "colorMaterial");
}

//------------------------------------
//SDF Texture primitive

void vaEAVolume::SetTexture(optix::TextureSampler samp)
{
    sampler.push_back(samp);
}

//------------------------------------------------------
//second sdf material
void SDFMaterial2::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitProg("closesthit_sdf", "materials_closesthit.cu", "closesthit_sdf");
            SetClosestHitProgName("closesthit_sdf"); //sets closesthit to true;

            //second material will be added to stack names
            InitProg("closesthit_sdf2", "materials_closesthit.cu", "closesthit_sdf2");
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}

void SDFMaterial2::Update()
{
    // if (GetOutput()->get() != nullptr) {
    Initialize();
    GetOutput() = vaBasicObject::GetContext()->createMaterial();
    //}
    SetClosestHitProg(GetOutput());
    //TODO: look at stack names, check for not closest hi main prog and add second material manually
    /*
    optix::Material m2 = vaBasicObject::GetContext()->createMaterial();
    SetClosestHitProg(1, m2);
    AddMaterial(m2);*/
}

//--------------------
//--------------SDF Volume

void SDFVolumeMaterial::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitProg("volume_any_hit", "materials_sdf_material.cu", "volume_any_hit");
            SetAnyHitProgName("volume_any_hit"); //sets anyhit to true;
            InitProg("highlight", "materials_sdf_material.cu", "highlight");
            SetDynamicProgName("highlight");
            //m_mapOfPrograms["closesthit"] = vaBasicObject::GetContext()->createProgramFromPTXFile(ptxPath("closesthit.cu"), "closesthit");
            //m_mapOfPrograms["volume_any_hit"] = vaBasicObject::GetContext()->createProgramFromPTXFile(ptxPath("sdf_material.cu"), "volume_any_hit");
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}

//--------------------
//--------------SDF Molecule Volume

void vaMolVolume::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitProg("volumehit_sdf", "materials_closesthit.cu", "volumehit_sdf");
            //SetAnyHitProgName("volumehit_sdf"); //sets anyhit to true;
            SetClosestHitProgName("volumehit_sdf"); //sets anyhit to true;

             // InitProg("highlight", "materials_sdf_material.cu", "highlight");
            // SetDynamicProgName("highlight");
             //m_mapOfPrograms["closesthit"] = vaBasicObject::GetContext()->createProgramFromPTXFile(ptxPath("closesthit.cu"), "closesthit");
             //m_mapOfPrograms["volume_any_hit"] = vaBasicObject::GetContext()->createProgramFromPTXFile(ptxPath("sdf_material.cu"), "volume_any_hit");
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}

//--------------SDF Molecule Volume

void vaMolVolume2::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitProg("volumehit_sdf2", "materials_closesthit.cu", "volumehit_sdf2");
            //SetAnyHitProgName("volumehit_sdf"); //sets anyhit to true;
            SetClosestHitProgName("volumehit_sdf2"); //sets anyhit to true;

                                                    // InitProg("highlight", "materials_sdf_material.cu", "highlight");
                                                    // SetDynamicProgName("highlight");
                                                    //m_mapOfPrograms["closesthit"] = vaBasicObject::GetContext()->createProgramFromPTXFile(ptxPath("closesthit.cu"), "closesthit");
                                                    //m_mapOfPrograms["volume_any_hit"] = vaBasicObject::GetContext()->createProgramFromPTXFile(ptxPath("sdf_material.cu"), "volume_any_hit");
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}
void vaMolVolume2::SetMaterialParameters()
{
    if (m_prog.get() != nullptr)
        GetOutput()["sdfPrim4"]->setProgramId(m_prog);
}
//--------------------
//--------------SDF Auditory Volume

void SDFAudioVolumeMaterial::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitProg("auditory_volume_any_hit", "materials_sdf_material.cu", "volume_any_hit");
            SetAnyHitProgName("volume_any_hit"); //sets anyhit to true;
            SetAuditoryTypeOn();
            //InitProg("highlight", "sdf_material.cu", "highlight");
            //SetDynamicProgName("highlight");
            //m_mapOfPrograms["closesthit"] = vaBasicObject::GetContext()->createProgramFromPTXFile(ptxPath("closesthit.cu"), "closesthit");
            //m_mapOfPrograms["volume_any_hit"] = vaBasicObject::GetContext()->createProgramFromPTXFile(ptxPath("sdf_material.cu"), "volume_any_hit");
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}

//--------------------
//--------------SDFAudioRayTraceMaterial

void SDFAudioRayTraceMaterial::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitProg("auditory_raytrace_hit", "materials_sdf_material.cu", "auditory_raytrace_hit");
            SetClosestHitProgName("auditory_raytrace_hit"); //sets anyhit to true;
            SetAuditoryTypeOn();

            std::cout << "Auditory PROG " << this->GetClosestHitProgName() << std::endl;
            //InitProg("highlight", "sdf_material.cu", "highlight");
            //SetDynamicProgName("highlight");
            //m_mapOfPrograms["closesthit"] = vaBasicObject::GetContext()->createProgramFromPTXFile(ptxPath("closesthit.cu"), "closesthit");
            //m_mapOfPrograms["volume_any_hit"] = vaBasicObject::GetContext()->createProgramFromPTXFile(ptxPath("sdf_material.cu"), "volume_any_hit");
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}

//--------------------
//--------------Dynamic texture Volume

void DynamicTextureVolumeMaterial::Initialize() {
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitProg("volume_texture_hit", "materials_sdf_material.cu", "volume_any_hit");
            SetAnyHitProgName("volume_any_hit"); //sets anyhit to true;

            //InitProg("transfer_function", "sdf_material.cu", "transfer_function");
            //SetDynamicProgName("transfer_function");
            //m_mapOfPrograms["closesthit"] = vaBasicObject::GetContext()->createProgramFromPTXFile(ptxPath("closesthit.cu"), "closesthit");
            //m_mapOfPrograms["volume_any_hit"] = vaBasicObject::GetContext()->createProgramFromPTXFile(ptxPath("sdf_material.cu"), "volume_any_hit");
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}