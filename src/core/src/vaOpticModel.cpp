#include "vaOpticModel.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

//for renderer class
#include <sutil.h>
#include <optixu/optixpp_namespace.h>

//---------------------------------
/*opticalModel
*a set of helping classes to process optical ray casting
*------------------------------------
/**
*Handles OpenGL interop and actually binds cuda output m_bufferOutput
* and OpenGL m_pboOutputBuffer
*/
void opticalModel::BindBuffer(optix::Context context)
{
    m_bufferOutput = (m_interop) ? context->createBufferFromGLBO(RT_BUFFER_INPUT_OUTPUT, m_pboOutputBuffer)
        : context->createBuffer(RT_BUFFER_INPUT_OUTPUT);
    m_bufferOutput->setFormat(RT_FORMAT_FLOAT4); // RGBA32F}
}

void opticalModel::Reshape(int width, int height)
{
    SetDim(width, height);
    //std::cout << "width=" << m_width << std::endl;
    if (m_interop)
    {
        m_bufferOutput->unregisterGLBuffer(); // Must unregister or CUDA won't notice the size change and crash.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_bufferOutput->getGLBOId());
        glBufferData(GL_PIXEL_UNPACK_BUFFER, m_bufferOutput->getElementSize() * width * height, nullptr, GL_STREAM_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        m_bufferOutput->registerGLBuffer();
    }

    m_width = width;
    m_height = height;
}

void opticalModel::UpdateBuffer()
{
    glsl->ActivateTexture();
    if (m_interop)
    {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_bufferOutput->getGLBOId());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei)m_width, (GLsizei)m_height, 0, GL_RGBA, GL_FLOAT, (void*)0); // RGBA32F from byte offset 0 in the pixel unpack buffer.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }
    else
    {
        const void* data = m_bufferOutput->map(0, RT_BUFFER_MAP_READ);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei)m_width, (GLsizei)m_height, 0, GL_RGBA, GL_FLOAT, data); // RGBA32F
        m_bufferOutput->unmap();
    }
}

void opticalModel::CreateGLSLModel()
{
    if (glsl == nullptr) {
        glsl = new glslRayCast();
    }
}
void opticalModel::Init() //setsOpticalBuffer
{
    CreateGLSLModel();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glViewport(0, 0, m_width, m_width);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (m_interop)
    {
        // PBO for the fast OptiX sysOutputBuffer to texture transfer.
        glGenBuffers(1, &m_pboOutputBuffer);
        if (m_pboOutputBuffer != 0) {
            // Buffer size must be > 0 or OptiX can't create a buffer from it.
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboOutputBuffer);
            glBufferData(GL_PIXEL_UNPACK_BUFFER, m_width*m_height * sizeof(float) * 4, (void*)0, GL_STREAM_READ); // RGBA32F from byte offset 0 in the pixel unpack buffer.
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        }
    }

    // glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // default, works for BGRA8, RGBA16F, and RGBA32F.

    glsl->initTexture();

    glsl->initGLSL();
}