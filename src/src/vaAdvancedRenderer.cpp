#include "vaAdvancedRenderer.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

//for renderer class
#include <sutil.h>
#include "shaders/app_config.h"
#include <optixu/optixpp_namespace.h>

//-------------------------------
// Visual stimuli ray tracing
//Redefined from vaBasicRenderer
//--------------------------------
//**-------------------------------
//*vaAdvancedRenderer
//*
//------------------------------------
void  vaAdvancedRenderer::InitDefaultModels()
{
    opticM = std::unique_ptr<antiAlizedModel>(new antiAlizedModel());
    audioM = std::unique_ptr<auditoryModel>(new auditoryModel());
    std::cout << "NEW INITED" << std::endl;
}

bool vaAdvancedRenderer::Render()
{
    bool repaint = false;

    try
    {
        switch (GetMode()) {
        case RenderModes::INTERACTIVE_WIDGET:
            UpdateWidget();
            break;
        case RenderModes::INTERACTIVE_CAMERA:
            UpdateCamera();
            break;
        }

        LaunchOpticContext();
        //TODO: return back when antialising will be restored
        //if (m_presentNext)
        {
            opticM->UpdateBuffer();

            repaint = true; // Indicate that there is a new image.

            m_presentNext = m_present;
        }
        //TIMER
  /*      double seconds = m_timer.getTime();
        if (m_presentAtSecond < seconds)
        {
            m_presentAtSecond = ceil(seconds);

            const double fps = double(m_iterationIndex) / seconds;

            std::ostringstream stream;
            stream.precision(3); // Precision is # digits in fraction part.
                                 // m_iterationIndex has already been incremented for the last rendered frame, so it is the actual framecount here.
            stream << std::fixed << m_iterationIndex << " / " << seconds << " = " << fps << " fps";
            std::cout << stream.str() << std::endl;

            m_presentNext = true; // Present at least every second.
        }
        */
    }
    catch (optix::Exception& e)
    {
        std::cerr << e.getErrorString() << std::endl;
    }
    return repaint;
}

void vaAdvancedRenderer::restartAccumulation()
{
    m_iterationIndex = 0;
    m_presentNext = true;
    m_presentAtSecond = 1.0;

    //    m_timer.restart();
}

void vaAdvancedRenderer::SetRayGenerationProg()
{
    InitProg("raygeneration", "antialise_raygeneration.cu", "antialise_raygeneration");
    SetRayGenerationProgName("antialise_raygeneration");
}
void vaAdvancedRenderer::SetExceptionProg()
{
    InitProg("exception", "antialise_exception.cu", "antialise_exception");
    SetExceptionProgName("antialise_exception");
}

bool vaAdvancedRenderer::UpdateCamera()
{
    bool cameraChanged = vaRenderer::UpdateCamera();
    if (cameraChanged)
    {
        restartAccumulation();
    }
    return cameraChanged;
}

void vaAdvancedRenderer::LaunchOpticContext()
{
    // Continue manual accumulation rendering if there is no limit (m_frames == 0) or the number of frames has not been reached.
    if (0 == m_frames || m_iterationIndex < m_frames)
    {
        GetContext()["sysIterationIndex"]->setInt(m_iterationIndex); // Iteration index is zero-based!
        vaRenderer::LaunchOpticContext();
        m_iterationIndex++;
        // std::cout << "increased " << GetContext()["sysIterationIndex"]->getInt() << std::endl;
    }
}
void vaAdvancedRenderer::SetMissProg()
{
    InitProg("miss_environment_constant", "antialise_miss.cu", "antialise_miss");
    SetMissProgName("antialise_miss");
}

/*void vaAdvancedRenderer::GetMissProg()
{
    InitProg("miss_environment_constant", "antialise_miss.cu", "antialise_miss");
    SetMissProgName("antialise_miss");
}*/

void vaAdvancedRenderer::InitRenderer()
{
    vaRenderer::InitRenderer();

    // Add context-global variables here.
    GetContext()["sysPathLengths"]->setInt(m_minPathLength, m_maxPathLength);
    GetContext()["sysIterationIndex"]->setInt(0); // With manual accumulation, 0 fills the buffer, accumulation starts at 1. On the VCA this variable is unused!
}