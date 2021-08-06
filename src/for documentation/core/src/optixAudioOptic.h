#pragma once
/*=========================================================================
Program:   Auditory and Optic interop classes


=========================================================================*/
/**
* @class  auditoryModel, opticModel
* @brief
*
* Architecture much inspired by VTK Renderer(http://vtk.org) implementation
* The classes are mainly responsible for openGL/openAL interop
*/

#ifndef optixAudioOptic_h
#define optixAudioOptic_h


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
class auditoryModel
{
public:
    auditoryModel() {};
    ~auditoryModel() {};

    void Render() {
        //glsl.Display();
    }
    void BindBuffer(optix::Context context); //initBuffer at the same time
    void GetBuffer(); //TDOO
    void Reshape(int width, int height);
    int GetWidth() { return m_width; };
    int GetHeight() { return m_height; };


    optix::Buffer GetOutput() { return m_bufferOutput; }
protected:

    optix::Buffer m_bufferOutput;
    int m_width;
    int m_height;

    //-----------------------------
    //Sound functions
    //-----------------------------
    void ComputeSoundRaycast(float distNorm, float distComp);
    void ConfigureHRTF();
};

class basicOpticalModel
{
public:


    basicOpticalModel() { m_bufferOutput = nullptr; };
    ~basicOpticalModel() {};
    virtual void InitOpenGL() {};
    virtual void Render() {  };
    virtual void BindBuffer(optix::Context context) {};
    virtual void UpdateOpticalBuffer() {};
    virtual void Reshape(int width, int height) {};
    void SetDim(int width, int height) { m_width = width; m_height = height; }


    optix::Buffer GetOutput() { return m_bufferOutput; }
    int GetWidth() { return m_width; }
    int GetHeight() { return m_height; }

    //helpers functions
    void SetBufferSize() {
        m_bufferOutput->setSize(m_width, m_height);

    }
    //protected:

    optix::Buffer m_bufferOutput;
    int m_width;
    int m_height;


    bool         m_interop;
};

//responsible for shaders and other staff
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


class opticalModel :public basicOpticalModel
{
public:
    glslRayCast glsl;

    opticalModel() {
        basicOpticalModel::basicOpticalModel();

    };
    ~opticalModel() {};
    virtual void InitOpenGL();
    virtual void Render() {
        glsl.Display();
    }
    virtual void BindBuffer(optix::Context context);
    virtual void UpdateOpticalBuffer();
    virtual void Reshape(int width, int height);


    //opxSetMacro(m_interop, bool);
    //opxGetMacro(m_interop, bool);

    void Modified() {};

protected:
    // OpenGL variables:
    GLuint m_pboOutputBuffer;



};

#endif