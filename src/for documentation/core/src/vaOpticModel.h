#pragma once
/*=========================================================================
Program:   Auditory and Optic interop classes

=========================================================================*/

#ifndef vaOpticModel_h
#define vaOpticModel_h

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
#include "vaBasicModel.h"

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

    std::string vsSource;
    std::string fsSource;

protected:
    // GLSL shaders objects and program.
    GLuint m_glslVS;
    GLuint m_glslFS;
    GLuint m_glslProgram;

    //texture
    GLuint m_hdrTexture;
};

/**
* \class  opticalModel
* \brief Actual binding through texture of the cuda output
*
* Brings together glsl shaders and openGL output buffer
*/
class opticalModel :public vaBasicModel
{
public:
    glslRayCast glsl;

    opticalModel() {};
    ~opticalModel() {};
    virtual void Init(); /*<Inits OpenGl and all procedures*/
    virtual void Render() {
        glsl.Display();
    }
    virtual void BindBuffer(optix::Context context);
    virtual void UpdateBuffer(); /*<Writes from cuda buffer to texture. Updates Optical Buffer*/
    virtual void Reshape(int width, int height);

    void Modified() {};

protected:
    /**
    *OpenGL variable
    */
    GLuint m_pboOutputBuffer;
};

#endif