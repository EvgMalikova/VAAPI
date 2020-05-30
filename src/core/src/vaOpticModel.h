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
#include "glslProcedures.h"

/**
* \class  opticalModel
* \brief Actual binding through texture of the cuda output
*
* Brings together glsl shaders and openGL output buffer
*/
class opticalModel :public vaBasicModel
{
public:

    opticalModel() { glsl = nullptr; };
    ~opticalModel() { delete glsl; };
    virtual void Init(); /*<Inits OpenGl and all procedures*/
    virtual void Render() {
        glsl->Display();
    }
    virtual void BindBuffer(optix::Context context);
    virtual void UpdateBuffer(); /*<Writes from cuda buffer to texture. Updates Optical Buffer*/
    virtual void Reshape(int width, int height);

    void Modified() {};

protected:

    glslRayCast* glsl;

    /* is used to create a custom GLSL in each optical model*/
    virtual void CreateGLSLModel();
    /**
    *OpenGL variable
    */
    GLuint m_pboOutputBuffer;
};

class antiAlizedModel : public opticalModel {
public:

    antiAlizedModel() {  };
    ~antiAlizedModel() {  };

    //As far the same
    //virtual void UpdateBuffer(); /*<Writes from cuda buffer to texture. Updates Optical Buffer*/
protected:

    /* is used to create a custom GLSL in each optical model*/
    virtual void CreateGLSLModel() {
        glsl = new glslAntiAlizedRayCast();
    };
};
#endif