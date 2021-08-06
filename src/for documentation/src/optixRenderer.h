/*=========================================================================
Program:   VolumeRenderer
Module:    vaRenderer.h

=========================================================================*/
/**
* @class  vaRenderer
* @brief
*
* Architecture much inspired by VTK Renderer(http://vtk.org) implementation
*An example of interactive renderer with non-progressive rendering approach
*
*/

#ifndef optixRenderer_h
#define optixRenderer_h

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
*\class
*\brief Rendering class
*/
class vaRenderer :public vaBasicRenderer
{
public:

    void SetInterop(bool inter) {
        opticM->SetInterop(inter);
    };
    vaRenderer() {
        Setm_stackSize(1024);
        SetInterop(true);
        Setm_devicesEncoding(3210);
    };
    ~vaRenderer() {};

    virtual void SetContext(optix::Context &context);

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
    void SetCamera(PinholeCamera* pinholeCamera) {
        m_pinholeCamera = pinholeCamera;
    }

    //todo: rewrite through class of basic camera
    void SetWidget(vaBaseWidget* v) {
        m_widget = v;
        AddActor(m_widget->GetActor());
    }

    virtual void Update();

private:

    unsigned int m_stackSize;

    PinholeCamera* m_pinholeCamera;
    vaBaseWidget* m_widget;
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

    void UpdateCamera();
    void UpdateWidget();
};
#endif