#include "glslProcedures.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

//for renderer class
#include <sutil.h>
#include <optixu/optixpp_namespace.h>

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

            SetGLSLVars();

            glUseProgram(0);
        }
    }
}

void glslRayCast::SetGLSLVars()
{
    glUniform1i(glGetUniformLocation(m_glslProgram, "samplerHDR"), 0); // texture image unit 0
}

void glslAntiAlizedRayCast::SetGLSLVars()
{
    glUniform1i(glGetUniformLocation(m_glslProgram, "samplerHDR"), 0);       // texture image unit 0
    glUniform1f(glGetUniformLocation(m_glslProgram, "invGamma"), 1.0f / m_gamma);
    glUniform3f(glGetUniformLocation(m_glslProgram, "colorBalance"), m_colorBalance.x, m_colorBalance.y, m_colorBalance.z);
    glUniform1f(glGetUniformLocation(m_glslProgram, "invWhitePoint"), m_brightness / m_whitePoint);
    glUniform1f(glGetUniformLocation(m_glslProgram, "burnHighlights"), m_burnHighlights);
    glUniform1f(glGetUniformLocation(m_glslProgram, "crushBlacks"), m_crushBlacks + m_crushBlacks + 1.0f);
    glUniform1f(glGetUniformLocation(m_glslProgram, "saturation"), m_saturation);
}
glslAntiAlizedRayCast::glslAntiAlizedRayCast()
{
    std::string vs =
        "#version 330\n"
        "layout(location = 0) in vec4 attrPosition;\n"
        "layout(location = 8) in vec2 attrTexCoord0;\n"
        "out vec2 varTexCoord0;\n"
        "void main()\n"
        "{\n"
        "  gl_Position  = attrPosition;\n"
        "  varTexCoord0 = attrTexCoord0;\n"
        "}\n";

    std::string fs =
        "#version 330\n"
        "uniform sampler2D samplerHDR;\n"
        "uniform vec3  colorBalance;\n"
        "uniform float invWhitePoint;\n"
        "uniform float burnHighlights;\n"
        "uniform float saturation;\n"
        "uniform float crushBlacks;\n"
        "uniform float invGamma;\n"
        "in vec2 varTexCoord0;\n"
        "layout(location = 0, index = 0) out vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "  vec3 hdrColor = texture(samplerHDR, varTexCoord0).rgb;\n"
        "  vec3 ldrColor = invWhitePoint * colorBalance * hdrColor;\n"
        "  ldrColor *= (ldrColor * burnHighlights + 1.0) / (ldrColor + 1.0);\n"
        "  float luminance = dot(ldrColor, vec3(0.3, 0.59, 0.11));\n"
        "  ldrColor = max(mix(vec3(luminance), ldrColor, saturation), 0.0);\n"
        "  luminance = dot(ldrColor, vec3(0.3, 0.59, 0.11));\n"
        "  if (luminance < 1.0)\n"
        "  {\n"
        "    ldrColor = max(mix(pow(ldrColor, vec3(crushBlacks)), ldrColor, sqrt(luminance)), 0.0);\n"
        "  }\n"
        "  ldrColor = pow(ldrColor, vec3(invGamma));\n"
        "  outColor = vec4(ldrColor, 1.0);\n"
        "}\n";

    SetVs(vs);
    SetFs(fs);

    SetDefaults();
    //std::cout << "NEW MODEL ANTIAL " << std::endl;
}