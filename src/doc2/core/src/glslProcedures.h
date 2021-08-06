#pragma once
/*=========================================================================
Program:   Auditory and Optic interop classes

=========================================================================*/

#ifndef glsl_h
#define glsl_h

#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include "macros.h"
#include <GL/glew.h>

#include "shaders/renderer/per_ray_data.h"
#include "predefined.h"

/**
* \class  glslRayCast
* \brief The class responsible for glsl shaders
*
* The shaders can include post processing procedures and other staff. This class provides basic facilities
* for whitted style ray tracing
*/

class glslRayCast
{
public:
    glslRayCast();
    ~glslRayCast() {};

    void initTexture();
    void initGLSL();
    void Display();
    void ActivateTexture() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_hdrTexture);
    }

    void SetVs(std::string s) {
        vsSource = s;
    }
    void SetFs(std::string s) {
        fsSource = s;
    }
protected:
    // GLSL shaders objects and program.
    GLuint m_glslVS;
    GLuint m_glslFS;
    GLuint m_glslProgram;

    //texture
    GLuint m_hdrTexture;

    std::string vsSource;
    std::string fsSource;
    /* inits default GLSL variables values */
    virtual void SetGLSLVars();
};

class glslAntiAlizedRayCast : public glslRayCast {
public:
    glslAntiAlizedRayCast();
    ~glslAntiAlizedRayCast() {};

    // Tonemapper group:
    float         m_gamma;
    optix::float3 m_colorBalance;
    float         m_whitePoint;
    float         m_burnHighlights;
    float         m_crushBlacks;
    float         m_saturation;
    float         m_brightness;
protected:
    void SetDefaults()
    {
        m_gamma = 2.2f;
        m_colorBalance = optix::make_float3(1.0f, 1.0f, 1.0f);
        m_whitePoint = 1.0f;
        m_burnHighlights = 0.8f;
        m_crushBlacks = 0.2f;
        m_saturation = 1.2f;
        m_brightness = 0.7f;
    }
    virtual void SetGLSLVars();
};
#endif