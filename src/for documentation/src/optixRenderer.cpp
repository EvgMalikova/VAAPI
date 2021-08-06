#include "optixRenderer.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

//for renderer class
#include <sutil.h>
#include "shaders/app_config.h"
#include <optixu/optixpp_namespace.h>

//**-------------------------------
//*vaRenderer
//*
//------------------------------------
void vaRenderer::InitRenderer()
{
    vaBasicRenderer::InitRenderer();
    try
    {
        vaBasicObject::GetContext()->setStackSize(m_stackSize);
        SetCameraDefaultParameters();
    }
    catch (optix::Exception& e)
    {
        std::cerr << "Error in vaRenderer " << e.getErrorString() << std::endl;
    }
}
void vaRenderer::SetCameraDefaultParameters()
{
    // Default initialization. Will be overwritten on the first frame.
    vaBasicObject::GetContext()[DefaultCamPos]->setFloat(defaultCameraPosition[0], defaultCameraPosition[1], defaultCameraPosition[2]);
    vaBasicObject::GetContext()[DefaultCamU]->setFloat(defaultCameraU[0], defaultCameraU[1], defaultCameraU[2]);
    vaBasicObject::GetContext()[DefaultCamV]->setFloat(defaultCameraV[0], defaultCameraV[1], defaultCameraV[2]);
    vaBasicObject::GetContext()[DefaultCamW]->setFloat(defaultCameraW[0], defaultCameraW[1], defaultCameraW[2]);
}

void vaRenderer::Update()
{
    vaBasicObject::GetContext()->validate();
    vaBasicObject::GetContext()->launch(vaBasicRenderer::OPTIC_RAYCASTING, 0, 0);
}

void vaRenderer::SetContext(optix::Context &context)
{
    vaBasicObject::SetContext(context);

    //----

//Call other protected members stuff
//    opticM.SetDim(GetWidth(), GetHeight());
}

void vaRenderer::UpdateWidget()
{
    m_widget->Show();

    m_widget->UpdateHandlePosition();
}

void vaRenderer::UpdateCamera()
{
    optix::float3 cameraPosition;
    optix::float3 cameraU;
    optix::float3 cameraV;
    optix::float3 cameraW;

    optix::float3 center;
    float dist;

    bool cameraChanged = m_pinholeCamera->getFrustum(cameraPosition, cameraU, cameraV, cameraW);
    if (cameraChanged)
    {
        vaBasicObject::GetContext()["sysCameraPosition"]->setFloat(cameraPosition);
        vaBasicObject::GetContext()["sysCameraU"]->setFloat(cameraU);
        vaBasicObject::GetContext()["sysCameraV"]->setFloat(cameraV);
        vaBasicObject::GetContext()["sysCameraW"]->setFloat(cameraW);

        // vaBasicObject::GetContext()["renderWidget"]->setInt(0);
        m_pinholeCamera->getFocusDistanceAndCenter(center, dist);
        m_widget->SetCamParameters(cameraPosition, cameraU, cameraV, cameraW, center, dist);
        //m_widget->Hide();
        m_widget->setDims(opticM->GetWidth(), opticM->GetHeight());
    }
}

bool vaRenderer::Render()
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

        opticM->UpdateBuffer();

        repaint = true; // Indicate that there is a new image.
    }
    catch (optix::Exception& e)
    {
        std::cerr << e.getErrorString() << std::endl;
    }
    return repaint;
}

void vaRenderer::Reshape(int width, int height)
{
    if ((width != 0 && height != 0) && // Zero sized interop buffers are not allowed in OptiX.
        (opticM->GetWidth() != width || opticM->GetHeight() != height))
    {
        opticM->SetDim(width, height);

        glViewport(0, 0, opticM->GetWidth(), opticM->GetHeight());
        try
        {
            opticM->SetBufferSize();
            //m_bufferOutput->setSize(width, height); // RGBA32F buffer.
            opticM->Reshape(width, height);
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }

        m_pinholeCamera->setViewport(width, height);
    }
    //std::cout << opticM->GetWidth() << std::endl;
}