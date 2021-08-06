#include "optixAudioOptic.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

//for renderer class
#include <sutil.h>
#include <optixu/optixpp_namespace.h>
#include "MyAssert.h"

#include "stkSound.h"




static LPALCGETSTRINGISOFT alcGetStringiSOFT;
static LPALCRESETDEVICESOFT alcResetDeviceSOFT;

//---------------------------------
//-opticalModel
//-a set of helping classes to process optical ray casting
//------------------------------------

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

void opticalModel::UpdateOpticalBuffer()
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

void opticalModel::InitOpenGL() //setsOpticalBuffer
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
        MY_ASSERT(m_pboOutputBuffer != 0);
        // Buffer size must be > 0 or OptiX can't create a buffer from it.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboOutputBuffer);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, m_width*m_height * sizeof(float) * 4, (void*)0, GL_STREAM_READ); // RGBA32F from byte offset 0 in the pixel unpack buffer.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

    // glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // default, works for BGRA8, RGBA16F, and RGBA32F.

    glsl.initTexture();

    glsl.initGLSL();
}




//---------------------------------
//-auditoryModel
//-a set of helping classes to process optical ray casting
//------------------------------------

void auditoryModel::BindBuffer(optix::Context context)
{
    m_bufferOutput = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_USER, 0, 0);
    //context["sysAuditoryOutputBuffer"]->setBuffer(m_bufferOutput);
    m_bufferOutput->setElementSize(sizeof(auditoryPrim));
    //m_bufferOutput->setSize(1,1); // size will be rederined later


}

void auditoryModel::ComputeSoundRaycast(float distNorm, float distComp)
{
    //float x[2];


    //vis.transferScale += 0.01f;
    //compute distances

    //l=2*44100/pitch
    //p=2*44100/l
    float l_n = distNorm; //max dist for normalization
    double norml_l = 2.0 * 44100.0 / 130.81;

    //https://newt.phys.unsw.edu.au/jw/notes.html
    //261.63 - C3

    //play l_n set it to pitch
    double koef = norml_l / l_n;
    //GenerateWave(261.63);
    //std::cout << "l=" << l[1] << "," << l[2] << std::endl;
    double norm_length = koef*distComp;
    double p = (2.0 * 44100.0) / norm_length;
    std::cout << "pitch = " << p << std::endl;
    std::cout << "play time = " << distNorm << std::endl;
    float seed = 222220000000;
    stkSound::GenerateNoise(distNorm, 48000, seed);
    //only good pitches
    //if(p<2000)
    //stkSound::GenerateWave(p, 48000.0);
    //play l[1,2] before normalization


}

//---------------------
//OpenAL HRTF functions
//---------------------

void auditoryModel::ConfigureHRTF()
{
    ALCdevice *device;
    const char *hrtfname = "hrtf\\default-44100";
    ALCint hrtf_state;
    ALCint num_hrtf;
    ALenum state;
    ALdouble angle;

    ALuint buffer;
    ALuint source;
    ALCint attribs[] = { ALC_HRTF_SOFT, ALC_TRUE, 0 };
    //ALCcontext* context = alcCreateContext(device, attribs);

    device = alcGetContextsDevice(alcGetCurrentContext());


    //alcDeviceReset(device, attribs);

    if (!alcIsExtensionPresent(device, "ALC_SOFT_HRTF")) {
        fprintf(stderr, "Error: ALC_SOFT_HRTF not supported\n");
        //CloseAL();
        return;
    }


    /* Define a macro to help load the function pointers. */
#define LOAD_PROC(d, x, y)  ((x) = (y)alcGetProcAddress((d), #x))
    LOAD_PROC(device, alcGetStringiSOFT, LPALCGETSTRINGISOFT);
    LOAD_PROC(device, alcResetDeviceSOFT, LPALCRESETDEVICESOFT);
#undef LOAD_PROC

    ALboolean has_angle_ext = alIsExtensionPresent("AL_EXT_STEREO_ANGLES");
    printf("AL_EXT_STEREO_ANGLES%s found\n", has_angle_ext ? "" : " not");

    /* Enumerate available HRTFs, and reset the device using one. */
    alcGetIntegerv(device, ALC_NUM_HRTF_SPECIFIERS_SOFT, 1, &num_hrtf);
    if (!num_hrtf)
        printf("No HRTFs found\n");
    else {
        ALCint attr[5];
        ALCint index = -1;
        ALCint i;
        ALenum state;

        printf("Available HRTFs:\n");
        for (i = 0; i < num_hrtf; i++) {
            const ALCchar *name = alcGetStringiSOFT(device, ALC_HRTF_SPECIFIER_SOFT, i);
            printf("    %d: %s\n", i, name);

            /* Check if this is the HRTF the user requested. */
            if (hrtfname && strcmp(name, hrtfname) == 0)
                index = i;
        }

        i = 0;
        attr[i++] = ALC_HRTF_SOFT;
        attr[i++] = ALC_TRUE;
        if (index == -1) {
            if (hrtfname)
                printf("HRTF \"%s\" not found\n", hrtfname);
            printf("Using default HRTF...\n");
        }
        else {
            printf("Selecting HRTF %d...\n", index);
            attr[i++] = ALC_HRTF_ID_SOFT;
            attr[i++] = index;
        }
        attr[i] = 0;

        if (!alcResetDeviceSOFT(device, attr))
            printf("Failed to reset device: %s\n", alcGetString(device, alcGetError(device)));
    }

    /* Check if HRTF is enabled, and show which is being used. */
    alcGetIntegerv(device, ALC_HRTF_SOFT, 1, &hrtf_state);
    if (!hrtf_state)
        printf("HRTF not enabled!\n");
    else {
        const ALchar *name = alcGetString(device, ALC_HRTF_SPECIFIER_SOFT);
        printf("HRTF enabled, using %s\n", name);
    }
    fflush(stdout);

    /*
    //
    buffer = CreateSineWave();
    if (!buffer) {
    //    CloseAL();
    return;
    }

    //Create the source to play the sound with.
    source = 0;
    alGenSources(1, &source);
    alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, -1.0f);
    alSourcei(source, AL_BUFFER, buffer);
    assert(alGetError() == AL_NO_ERROR && "Failed to setup sound source");

    //Play the sound until it finishes.
    angle = 0.0;
    alSourcePlay(source);
    do {

    al_nssleep(100000);/1000000000ul;
    cout << "first finished" << endl;

    //Rotate
    angle += 0.01 * M_PI * 0.5;
    if (angle > M_PI)
    angle -= M_PI*2.0;

    // This only rotates mono sounds.
    alSource3f(source, AL_POSITION, (ALfloat)sin(angle), 0.0f, -(ALfloat)cos(angle));

    if (has_angle_ext) {

    ALfloat angles[2] = {(ALfloat)(M_PI / 6.0 - angle), (ALfloat)(-M_PI / 6.0 - angle)};
    alSourcefv(source, AL_STEREO_ANGLES, angles);
    }

    alGetSourcei(source, AL_SOURCE_STATE, &state);
    } while (alGetError() == AL_NO_ERROR && state == AL_PLAYING);

    // All done. Delete resources, and close down SDL_sound and OpenAL.
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);

    alutExit();
    */
}
//-----------------------------------
void auditoryModel::Reshape(int width, int height)
{
    m_width = width;
    m_height = height;
    m_bufferOutput->setSize(m_width, m_height);
}

void auditoryModel::GetBuffer()
{
    auditoryPrim* audio = static_cast<auditoryPrim*>(m_bufferOutput->map());
    // ... // Do something with the double data inside your buffer.
    m_bufferOutput->unmap();


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
    MY_ASSERT(m_hdrTexture != 0);

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
        MY_ASSERT(vsCompiled);
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
        MY_ASSERT(fsCompiled);
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
        MY_ASSERT(programLinked);

        if (programLinked)
        {
            glUseProgram(m_glslProgram);

            glUniform1i(glGetUniformLocation(m_glslProgram, "samplerHDR"), 0); // texture image unit 0

            glUseProgram(0);
        }
    }
}

