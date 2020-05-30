#pragma once
/*=========================================================================
Program:   Auditory and Optic interop classes

=========================================================================*/

#ifndef vaBasicModel_h
#define vaBasicModel_h

#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include "predefined.h"

/**
* \class  vaBasicModel
* \brief The class responsible for rendering auditory propertis
*
* The class implements basic features, common for both optical and auditory models
*/
class vaBasicModel
{
public:

    vaBasicModel() { m_bufferOutput = nullptr; };
    ~vaBasicModel() {};

    virtual void Init() {};
    virtual void Render() {  };
    virtual void BindBuffer(optix::Context context) {};
    virtual void UpdateBuffer() {};
    virtual void Reshape(int width, int height) {};

    //helpers functions
    void SetDim(int width, int height) { m_width = width; m_height = height; }
    void SetBufferSize() {
        m_bufferOutput->setSize(m_width, m_height);
    }
    optix::Buffer GetOutput() { return m_bufferOutput; }

    int GetWidth() { return m_width; }
    int GetHeight() { return m_height; }
    void SetInterop(bool inter)/*<Sets OpenGl or OpenAl interop*/
    {
        m_interop = inter;
    };

protected:

    optix::Buffer m_bufferOutput;
    int m_width;
    int m_height;

    bool         m_interop;
};

#endif