#include "vaAudioModel.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

//for renderer class
#include <sutil.h>
#include <optixu/optixpp_namespace.h>
#include "shaders/renderer/per_ray_data.h"
#include "stkSound.h"

static LPALCGETSTRINGISOFT alcGetStringiSOFT;
static LPALCRESETDEVICESOFT alcResetDeviceSOFT;

//TODO
//Move to mapping and create several buffers

float auditoryMapper::getAtomSound(int type)
{
    float vel_magnitude = 0;
    switch (type)
    {
    case 1:
        vel_magnitude = 0.1;
        break;
    case 2:
        vel_magnitude = 0.5;
        break;
    case 3:
        vel_magnitude = 1.0;
        break;
    case 4:
        vel_magnitude = 1.5;
        break;
    case 5:
        vel_magnitude = 0.3;
        break;
    case 6:
        vel_magnitude = 2.0;
        break;

    case -1:
        vel_magnitude = -0.1;
        break;
    case -2:
        vel_magnitude = -0.5;
        break;
    case -3:
        vel_magnitude = -1.0;
        break;
    case -4:
        vel_magnitude = -1.5;
        break;
    case -5:
        vel_magnitude = -0.3;
        break;
    case -6:
        vel_magnitude = -2.0;
        break;
    }

    return vel_magnitude;
}
int  auditoryMapper::getAtomNumber(std::string type)
{
    int vel_magnitude = 0;

    if (type == "H")       //and check its value
    {
        vel_magnitude = 1; ///yellow
    }
    if (type == "C")       //and check its value
    {
        vel_magnitude = 2;
    }
    if (type == "N")       //and check its value
    {
        vel_magnitude = 3;// / 8.0;
    }
    if (type == "S")       //and check its value
    {
        vel_magnitude = 4; //not visible
    }
    if (type == "O")       //and check its value
    {
        vel_magnitude = 5;
    }
    if (type == "P")       //and check its value
    {
        vel_magnitude = 6;// / 8.0;
    }
    return vel_magnitude;
}

//---------------------------------
//-auditoryModel
//-a set of helping classes to process optical ray casting
//------------------------------------

void auditoryModel::BindBuffer(optix::Context context)
{
    m_bufferOutput = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_USER, 0, 0);
    m_bufferOutput->setElementSize(sizeof(auditoryPrim));
    //m_bufferOutput->setSize(1,1); // size will be rederined later
}

//TODO: implement when context is set
void auditoryModel::SetAudioScanningParam(AudioScanMode m)
{
    m_mode = m;
    //vaBasicObject::GetContext()["scan_all"]->setInt(int(m));
}
void auditoryModel::UpdateBuffer()
{
    void* imageData2;

    auditoryPrim* ImageData = (auditoryPrim*)(m_bufferOutput->map());

    // int w, h;
    // m_window->GetRenderer()->GetAudioDim(w, h);

    m_distNorm = 0;
    std::cout << m_width << "," << m_height << std::endl;
    for (unsigned int y = 0; y < m_height; ++y)
    {
        for (unsigned int x = 0; x < m_width; ++x)
        {
            int index = y * m_width + x;
            int num = int(ImageData[index][MAX_PRIM_ALONG_RAY - 1].x);

            if (num > 0) {
                desc.push_back(primDes(x, y, ImageData[index]));
                for (int i = 0; i < num; i++) {
                    std::cout << " " << desc[desc.size() - 1].data[i].y;
                    //compute maximum value for normalisation
                    if (m_distNorm < desc[desc.size() - 1].data[i].x) m_distNorm = desc[desc.size() - 1].data[i].x;
                }
                std::cout << std::endl;
            }
        }
    }

    m_bufferOutput->unmap();
}

void auditoryModel::ComputeSoundRaycast(float distComp)
{
    //float l_n = m_distNorm; //max dist for normalization
    double norml_l = 2.0 * 44100.0 / 130.81;

    //https://newt.phys.unsw.edu.au/jw/notes.html
    //261.63 - C3

    //play l_n set it to pitch
    double koef = norml_l / m_distNorm;
    //GenerateWave(261.63);
    //std::cout << "l=" << l[1] << "," << l[2] << std::endl;
    double norm_length = koef*distComp;
    double p = (2.0 * 44100.0) / norm_length;
    std::cout << "pitch = " << p << std::endl;
    std::cout << "play time = " << m_distNorm << std::endl;
    float seed = 222220000000;
    stkSound::GenerateNoise(m_distNorm, 48000, seed);
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
}
//-----------------------------------
void auditoryModel::Reshape(int width, int height)
{
    SetDim(width, height);
    m_bufferOutput->setSize(m_width, m_height);
}

void auditoryModel::Init()
{
    //initialise Alut
    stkSound::InitEnv();
}

float auditoryModel::GetNormaKoeff()
{
    return scale / m_distNorm;
}

void auditoryMapperCmaj::Map(float maxDist, std::vector<stk::StkFrames>& scannedRays, std::vector<primDes> desc, int scale)
{
    std::cout << "Start generating waves Cmaj " << std::endl;
    //float maxDist = m_distNorm;
    //Normalization variables
     //Total time the sound will play

    //TODO:
    //Set auditory normalisation coeff for highlightment
    //context["SoundTimeCoef"]->setFloat(scale / maxDist);
    scannedRays.clear();

    //pregenerate all data
    stk::StkFrames framesNoise = stkSound::GenerateNoise(scale, 48000, 222222);
    float koef1 = 0.6;
    float koef2 = 0.1;
    //if (xPosRay.size() > 1) koef1 /= (xPosRay.size() / 2);
    framesNoise.attenuate(koef1, koef2);
    std::vector<stk::StkFrames> notes;
    //std::cout << maxDist << "," << ImageData[lastNum].x << std::endl;
    float koef = scale / maxDist;// ImageData[lastNum].x; //normalization coefficent //maxDist
    float normVal = 1 / maxDist;// ImageData[lastNum].x; //maxDist
    double srate2 = 48000;

    //music scale approximate mappings
    float scaleM = C6 - C2; //total number of notes

    //Pregenerate all waves for all types of atoms
    for (int i = 1; i < 7; i++)
    {
        int ii = i;

        //TODO: set highlightment features
        /*
        if (Render::selection[i - 1])
        {
            ii = -ii;
        }*/
        int m = C2 + int(scaleM*getAtomSound(ii));

        float freq = 440 * pow(2.0, (m - 69) / 12);
        stk::Stk::setSampleRate(srate2);
        stk::StkFrames frA = stkSound::GeneratePlucked(2, srate2, freq, 1);
        notes.push_back(frA);
    }
    std::cout << "mapping done" << std::endl;
    for (int j = 0; j < desc.size(); j++)
    {
        //if (num[j] > 0)  ///only those are filled

            //std::cout << "detected " << num[j] << std::endl;

        stk::Stk::setSampleRate(srate2);
        stk::StkFrames frames(framesNoise);// = stkSound::GenerateNoise(scale, 48000, 222222);

        //get the first distance and make it noise
        stkSound::ApplyADSR(frames, desc[j].data[0].x * koef*srate2);

        //for all atoms along ray fill the bufferf
        //TODO::we skip the last one atom
        int num = int(desc[j].data[MAX_PRIM_ALONG_RAY - 1].x);

        if (num > 1)
        {
            std::cout << "adsr applied " << j << " of " << desc.size() << " num of elements " << num << std::endl;
            for (int i = 1; i < num; i++)
            {
                int curNum = i - 1;
                int nextNum = i;
                float timeSound = abs((desc[j].data[nextNum].x - desc[j].data[curNum].x)) * koef;

                float start = desc[j].data[curNum].x * koef*srate2;

                int noteN = int(desc[j].data[curNum].y); //getting noteNum on base of type

                stk::StkFrames fr(notes[noteN - 1]); //as index starts from 1
                // if (Render::selection[noteN - 1]) //attenuate all not highlighted notes
                //     fr.attenuate(1.9, 1.5);
                frames.addFrames(start, fr);

                frames.attenuate(0.8, 0.001);
                scannedRays.push_back(frames);
            }
        }
    }

    std::cout << "adsr applied" << std::endl;
}

///-----------------------------------------------------------------
void auditoryMapperPlucked::Map(float maxDist, std::vector<stk::StkFrames>& scannedRays, std::vector<primDes> desc, int scale)
{
    std::cout << "Start generating waves for plucked" << std::endl;
    //float maxDist = m_distNorm;
    //Normalization variables
    //Total time the sound will play

    //TODO:
    //Set auditory normalisation coeff for highlightment
    //context["SoundTimeCoef"]->setFloat(scale / maxDist);
    scannedRays.clear();

    //pregenerate all data

    std::vector<stk::StkFrames> notes;
    //std::cout << maxDist << "," << ImageData[lastNum].x << std::endl;
    float koef = scale / maxDist;// ImageData[lastNum].x; //normalization coefficent //maxDist
    float normVal = 1 / maxDist;// ImageData[lastNum].x; //maxDist
    double srate2 = 48000;

    for (int j = 0; j < desc.size(); j++)
    {
        //if (num[j] > 0)  ///only those are filled

        //std::cout << "detected " << num[j] << std::endl;

        stk::Stk::setSampleRate(srate2);
        stk::StkFrames frames(scale);// = stkSound::GenerateNoise(scale, 48000, 222222);

                                           //get the first distance and make it noise

        //for all atoms along ray fill the bufferf
        //TODO::we skip the last one atom
        int num = int(desc[j].data[MAX_PRIM_ALONG_RAY - 1].x);

        if (num > 0)
        {
            std::cout << "adsr applied " << j << " of " << desc.size() << " num of elements " << num << std::endl;
            for (int i = 1; i < num; i++)
            {
                int curNum = i - 1;
                int nextNum = i;
                float timeSound = abs((desc[j].data[nextNum].x));// -desc[j].data[curNum].x)) * koef;

                float start = desc[j].data[curNum].x * koef*srate2;

                //compute note
              /////l=2*R/p

                float freq = srate2 * 2 / timeSound;
                stk::Stk::setSampleRate(srate2);
                stk::StkFrames fr = stkSound::GeneratePlucked(2, srate2, freq, 1);

                //stk::StkFrames fr(notes[noteN - 1]); //as index starts from 1
                                                     // if (Render::selection[noteN - 1]) //attenuate all not highlighted notes
                                                     //     fr.attenuate(1.9, 1.5);
                frames.addFrames(start, fr);

                frames.attenuate(0.8, 0.001);
                scannedRays.push_back(frames);
            }
        }
    }

    std::cout << "Mapping Done" << std::endl;
}

//--------------------------------------
//audio experiments with treads
void auditoryModel::Render()
{
    ConfigureHRTF();

    descSs2.clear();
    for (int j = 0; j < scannedRays.size(); j++) {
        //std::cout << "scanned rays " << scannedRays[j].size() << std::endl;

        stk::StkFloat* ptr = &scannedRays[j][0];
        //float xpos = float(xPosRay[j].x);
        //if (xpos > 1) xpos = -1.0;
        //xpos = xpos / 2 - 0.5;

        stkSound::soundDesc sDesc = stkSound::PlayBuffer(ptr, scale, int(scannedRays[j].size()), j, optix::make_float2(desc[j].x, desc[j].y));
        descSs2.push_back(sDesc);
        //stkSound::WaitforSound(10, sDesc.source);
    }

    for (int j = 0; j < descSs2.size(); j++)
    {
        stkSound::PlaySelSound(descSs2[j].source);
    }
}
void auditoryModel::CheckAllAudio(float time)
{
    //move sounds
    //stkSound::MoveSounds(descSs, xPosRay, scale);

    stkSound::WaitforSound(time);
}
void auditoryModel::ClearAllAudio()
{
    for (int j = 0; j < descSs2.size(); j++) {
        alDeleteSources(1, &descSs2[j].source);
        alDeleteBuffers(1, &descSs2[j].buffer);
    }
    descSs2.clear();
    // context["TimeSound"]->setFloat(float(0));
     //playGenerated = 0;
}