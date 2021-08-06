#include "optixBasicRenderer.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

//for renderer class
#include <sutil.h>
#include <optixu/optixpp_namespace.h>

void vaBasicRenderer::InitProg(std::string prog, std::string file, std::string name)
{
    m_mapOfPrograms[name] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath(file), prog);
}
//-------------------------------
// Visual stimuli ray tracing
//--------------------------------
void vaBasicRenderer::SetRayGenerationProg()
{
    InitProg("raygeneration", "raygeneration.cu", "raygeneration");
    SetRayGenerationProgName("raygeneration");
}
void vaBasicRenderer::SetExceptionProg()
{
    InitProg("exception", "exception.cu", "exception");
    SetExceptionProgName("exception");
}
void vaBasicRenderer::SetMissProg()
{
    InitProg("miss_environment_constant", "miss.cu", "miss");
    SetMissProgName("miss");
}

void vaBasicRenderer::ActivateOpticalModel()
{
    for (int i = 0; i < GetNumOfActors(); i++) {
        optixBasicActor* a = GetActor(i);
        //a->PrintInfo();  - works
        a->SetOpticalModel();
    }
}
//-------------------------------
// Setting up Renderer
//--------------------------------
void vaBasicRenderer::InitRenderer()
{
    try
    {
        if (isAuditory()) {
            vaBasicObject::GetContext()->setEntryPointCount(2); // 0 = render
            vaBasicObject::GetContext()->setRayTypeCount(2);    // 0 = radiance
        }
        else {
            vaBasicObject::GetContext()->setEntryPointCount(1); // 0 = render
            vaBasicObject::GetContext()->setRayTypeCount(1);    // 0 = radiance
        }
        //vaBasicObject::GetContext()->setStackSize(m_stackSize);
        //std::cout << "stackSize = " << m_stackSize << std::endl;

        //Bind optical buffer
        opticM->BindBuffer(vaBasicObject::GetContext());
        opticM->SetBufferSize();
        setOpticBuffer();

        //Similarly bind Auditory Buffer and set Variables

        audioM->BindBuffer(vaBasicObject::GetContext());
        audioM->SetBufferSize();
        setAudioBuffer();

        SetTime(0);

        //initialise all programs
        SetPrograms();
    }
    catch (optix::Exception& e)
    {
        std::cerr << "Error in optixBasicRenderer " << e.getErrorString() << std::endl;
    }
}
//-------------------------------
// Auditory stimuli ray tracing
//--------------------------------
void vaBasicRenderer::ActivateAuditoryModel()
{
    for (int i = 0; i < GetNumOfActors(); i++) {
        optixBasicActor* a = GetActor(i);
        //a->PrintInfo();  - works
        a->SetAuditoryModel();
    }
}
//for auditory
//This can be overwritten by widget for auditory ray-casting
void vaBasicRenderer::SetAuditoryRayGenerationProg()
{
    if (!m_AudioWidget)
    {
        InitProg("auditory_raygeneration", "raygeneration.cu", "auditory_raygeneration");
        SetAuditoryRayGenerationProgName("auditory_raygeneration");
        std::cout << "Defaul AUDIO Program set " << std::endl;
    }
}

void vaBasicRenderer::SetAuditoryRayGenerationFromWidget(std::string name, optix::Program prog)
{
    m_mapOfPrograms[name] = prog;
    SetAuditoryRayGenerationProgName(name);
    m_AudioWidget = true;
    std::cout << "AUDIO Program set " << name << std::endl;
}
void vaBasicRenderer::SetAuditoryMapModel(auditoryMapper*m)
{
    audioM->SetAuditoryMapper(m);
}
void vaBasicRenderer::SetAuditoryExceptionProg()
{
    InitProg("auditory_exception", "exception.cu", "auditory_exception");
    SetAuditoryExceptionProgName("auditory_exception");
}
void vaBasicRenderer::SetAuditoryMissProg()
{
    InitProg("auditory_miss_environment_constant", "miss.cu", "auditory_miss");
    SetAuditoryMissProgName("auditory_miss");
}

void vaBasicRenderer::InitializePrograms() {
    // Renderer
    SetRayGenerationProg();
    SetExceptionProg();
    SetMissProg();
    if (m_isAuditory)
    {
        SetAuditoryRayGenerationProg();
        SetAuditoryExceptionProg();
        SetAuditoryMissProg();
    }
}
void vaBasicRenderer::SetPrograms()
{
    std::map<std::string, optix::Program>::const_iterator it = m_mapOfPrograms.find(raygeneration_program);
    if (it != m_mapOfPrograms.end()) {
        vaBasicObject::GetContext()->setRayGenerationProgram(OPTIC_RAYCASTING, it->second); // entrypoint
    }
    it = m_mapOfPrograms.find(exception_program);
    if (it != m_mapOfPrograms.end())
    {
        vaBasicObject::GetContext()->setExceptionProgram(OPTIC_RAYCASTING, it->second); // entrypoint
    }
    it = m_mapOfPrograms.find(miss_program);
    if (it != m_mapOfPrograms.end()) {
        vaBasicObject::GetContext()->setMissProgram(OPTIC_RAYCASTING, it->second); // raytype
    }
    //set up other context
    if (isAuditory())
    {
        std::map<std::string, optix::Program>::const_iterator it = m_mapOfPrograms.find(auditory_raygeneration_program);
        if (it != m_mapOfPrograms.end()) {
            vaBasicObject::GetContext()->setRayGenerationProgram(AUDITORY_RAYCASTING, it->second); // entrypoint
        }
        it = m_mapOfPrograms.find(auditory_exception_program);
        if (it != m_mapOfPrograms.end()) {
            vaBasicObject::GetContext()->setExceptionProgram(AUDITORY_RAYCASTING, it->second); // entrypoint
        }
        it = m_mapOfPrograms.find(auditory_miss_program);
        if (it != m_mapOfPrograms.end()) {
            vaBasicObject::GetContext()->setMissProgram(AUDITORY_RAYCASTING, it->second); // raytype
        }
    }
}

void vaBasicRenderer::InitAcceleration()
{
    //set acceleration
    m_rootAcceleration = vaBasicObject::GetContext()->createAcceleration(m_builder); // No need to set acceleration properties on the top level Acceleration.

    m_rootGroup = vaBasicObject::GetContext()->createGroup(); // The scene's root group nodes becomes the sysTopObject.
    m_rootGroup->setAcceleration(m_rootAcceleration);

    vaBasicObject::GetContext()["sysTopObject"]->set(m_rootGroup); // This is where the rtTrace calls start the BVH traversal. (Same for radiance and shadow rays.)
}

void vaBasicRenderer::AddActor(optixBasicActor* act)
{
    m_act.push_back(act);

    unsigned int count;
    // Add the transform node placeing the plane to the scene's root Group node.
    count = m_rootGroup->getChildCount();
    m_rootGroup->setChildCount(count + 1);
    m_rootGroup->setChild(count, act->GetOutput());
}

void vaBasicRenderer::LaunchAuditoryContext()
{
    vaBasicObject::GetContext()->launch(AUDITORY_RAYCASTING, audioM->GetWidth(), audioM->GetHeight());
}

bool vaBasicRenderer::PlayAnimation(float play_time)
{
    m_timeSound += 0.1;
    std::cout << "Playing " << m_timeSound << "," << play_time << std::endl;
    vaBasicObject::GetContext()["TimeSound"]->setFloat(m_timeSound);
    if (m_timeSound < play_time) {
        return false;
    }
    else true;
}

void vaBasicRenderer::SetUpRenderer()
{
    opticM->Init();
    audioM->Init();
    //TODO: should be restructured and moved to Update()
    //so before all parameters like is auditory should be set
    InitializePrograms(); //bind programs to stack

    InitRenderer(); //call programs

                    //from basic renderer
    InitAcceleration();
}

void vaBasicRenderer::LaunchOpticContext()
{
    int width = opticM->GetWidth();
    int height = opticM->GetHeight();
    vaBasicObject::GetContext()->launch(OPTIC_RAYCASTING, width, height);
    //  std::cout << "width in launch " << opticM->GetWidth()<<m_width << std::endl;
      //no launching context auditory here
}