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
    m_bufferOutput->setFormat(RT_FORMAT_FLOAT4); // RGBA32F
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
    glsl.ActivateTexture();
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

void opticalModel::Init() //setsOpticalBuffer
{
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

    glsl.initTexture();

    glsl.initGLSL();
}

//---------------------------------
//-glslRay casting procedures
//-
//------------------------------------

void glslRayCast::Display()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_hdrTexture);

    glUseProgram(m_glslProgram);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    glUseProgram(0);
}

void glslRayCast::initTexture()
{
    glGenTextures(1, &m_hdrTexture);
    if (m_hdrTexture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_hdrTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        // DAR ImGui has been changed to push the GL_TEXTURE_BIT so that this works.
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    }
    else std::cerr << "no texture defined" << std::endl;
}

glslRayCast::glslRayCast()
{
    vsSource =
        "#version 330\n"
        "layout(location = 0) in vec4 attrPosition;\n"
        "layout(location = 8) in vec2 attrTexCoord0;\n"
        "out vec2 varTexCoord0;\n"
        "void main()\n"
        "{\n"
        "  gl_Position  = attrPosition;\n"
        "  varTexCoord0 = attrTexCoord0;\n"
        "}\n";

    fsSource =
        "#version 330\n"
        "uniform sampler2D samplerHDR;\n"
        "in vec2 varTexCoord0;\n"
        "layout(location = 0, index = 0) out vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "  outColor = texture(samplerHDR, varTexCoord0);\n"
        "}\n";

    m_glslVS = 0;
    m_glslFS = 0;
    m_glslProgram = 0;
}
void glslRayCast::initGLSL()
{
    GLint vsCompiled = 0;
    GLint fsCompiled = 0;

    m_glslVS = glCreateShader(GL_VERTEX_SHADER);
    if (m_glslVS)
    {
        GLsizei len = (GLsizei)vsSource.size();
        const GLchar *vs = vsSource.c_str();
        glShaderSource(m_glslVS, 1, &vs, &len);
        glCompileShader(m_glslVS);
        //checkInfoLog(vs, m_glslVS);

        glGetShaderiv(m_glslVS, GL_COMPILE_STATUS, &vsCompiled);
        if (!vsCompiled) {
            std::cerr << "Vertex Shader was not compiled " << std::endl;
        }
    }

    m_glslFS = glCreateShader(GL_FRAGMENT_SHADER);
    if (m_glslFS)
    {
        GLsizei len = (GLsizei)fsSource.size();
        const GLchar *fs = fsSource.c_str();
        glShaderSource(m_glslFS, 1, &fs, &len);
        glCompileShader(m_glslFS);
        //		checkInfoLog(fs, m_glslFS);

        glGetShaderiv(m_glslFS, GL_COMPILE_STATUS, &fsCompiled);
        if (!fsCompiled) {
            std::cerr << "Fragment Shader was not compiled " << std::endl;
        }
    }

    m_glslProgram = glCreateProgram();
    if (m_glslProgram)
    {
        GLint programLinked = 0;

        if (m_glslVS && vsCompiled)
        {
            glAttachShader(m_glslProgram, m_glslVS);
        }
        if (m_glslFS && fsCompiled)
        {
            glAttachShader(m_glslProgram, m_glslFS);
        }

        glLinkProgram(m_glslProgram);
        //checkInfoLog("m_glslProgram", m_glslProgram);

        glGetProgramiv(m_glslProgram, GL_LINK_STATUS, &programLinked);
        if (!programLinked) {
            std::cerr << "Program linking error " << std::endl;
        }

        else
        {
            glUseProgram(m_glslProgram);

            glUniform1i(glGetUniformLocation(m_glslProgram, "samplerHDR"), 0); // texture image unit 0

            glUseProgram(0);
        }
    }
}