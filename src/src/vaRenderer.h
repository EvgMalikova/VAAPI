/*=========================================================================
Program:   VolumeRenderer
Module:    vaRenderer.h

=========================================================================*/

#ifndef vaRenderer_h
#define vaRenderer_h

#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include "data_structures/vertex_attributes.h"
#include "macros.h"

#include "inc/PinholeCamera.h"

//#include <GL/glew.h>

#include "vaBaseWidget.h"
#include "optixBasicRenderer.h"
#include "optixBasicActor.h"

/*
*\class vaRenderer
*\brief  A start point for interactive rendering.
Current class implements ready to use ray-tracing withoug anti-alising
with all prepared camera and widget operation procedures

Operates Pinhole camera and related to it widgets. For defining your own camera and low level widgets
look at vaBasicRenderer

Handles device and interop staff
*/
class vaRenderer :public vaBasicRenderer
{
public:

    vaBasicRenderer& get() { return *this; };

    /*customly updates camera without updating widget
    */
    virtual void UpdateParam();

    vaRenderer() : vaBasicRenderer(false)
    {
        Setm_stackSize(1024);
        Setm_devicesEncoding(3210);
        m_sceneEpsilonFactor = 30500;  // Factor on 1e-7 used to offset ray origins along the path to reduce self intersections.
    };
    ~vaRenderer() {};

    virtual void SetContext(optix::Context&context) {
        vaBasicObject::SetContext(context);
        SetInterop(true);
        SetMultiscaleParam(0.0);
    }
    opxSetMacro(m_stackSize, unsigned int);
    opxGetMacro(m_stackSize, unsigned int);

    opxSetMacro(m_devicesEncoding, unsigned int);
    opxGetMacro(m_devicesEncoding, unsigned int);

    bool IsValid() { return m_isValid; }

    virtual bool Render();
    virtual void Display() {
        opticM->Render();
    };
    void Reshape(int width, int height);

    void SetValid(bool t) { m_isValid = true; };

    //todo: rewrite through class of basic camera
    void SetCamera(std::shared_ptr<PinholeCamera> pinholeCamera) {
        m_pinholeCamera = pinholeCamera;
    }

    //todo: rewrite through class of basic camera
    void SetWidget(std::shared_ptr<vaBaseWidget> v) {
        m_widget = v;
        AddActor(m_widget->GetActor());
    }

    virtual void Update();

private:
    int m_sceneEpsilonFactor;
    unsigned int m_stackSize;

    std::shared_ptr<PinholeCamera> m_pinholeCamera;
    std::shared_ptr<vaBaseWidget> m_widget;
    // Application command line parameters.
    unsigned int m_devicesEncoding;

    //checks for exception: TODO extend for the case of all objects in API
    bool        m_isValid;

    //Default parameters
    const float defaultCameraPosition[3] = { 0.0f, 0.0f, 1.0f };
    const float defaultCameraU[3] = { 1.0f, 0.0f, 0.0f };
    const float defaultCameraV[3] = { 0.0f, 1.0f, 0.0f };
    const float defaultCameraW[3] = { 0.0f, 0.0f, -1.0f };

protected:

    //-----------------------------
    virtual void InitRenderer(); //entry point for all programs and rendering stuff, both optical and auditory

    virtual void Modified() {};

    void SetCameraDefaultParameters();

    virtual bool UpdateCamera();
    void UpdateWidget();
};
#endif