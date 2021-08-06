/*=========================================================================
Program:   VAAPI
Module:    vaBasicRenderer.h

=========================================================================*/

#ifndef vaBasicRenderer_h
#define vaBasicRenderer_h

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <vector>
#include <map>

#include "macros.h"
//#include <GL/glew.h>

#include "predefined.h"
#include "vaOpticModel.h"
#include "vaAudioModel.h"

#include "optixBasicActor.h"

/**
* \class  vaBasicRenderer
* \brief Abstract visual-auditory renderer
*
* Abstract visual-auditory renderer. The class is responsible for:
* - setting optical model
* - setting auditory model

* - specifies main rendering modes in API enum RenderModes

* - launches ray-tracing of visual and optical stimuli
*/

class vaBasicRenderer :public vaBasicObject
{
public:
    /* Sets Background color in default programs (context, per ray data, miss prog)*/
    virtual void SetBackground(optix::float3 bg)
    {
        GetContext()["sysBackground"]->setFloat(bg.x, bg.y, bg.z);
    }
    /**
    * 1) Gets to host the ray-casted buffer of audio model
    2) Maps to auditory stimuli according to specified auditory model mapping
    */
    void UpdateAudioBuffer();
    /* Plays sound */
    void RenderAudio();

    /*Sets external auditory mapper*/
    void SetAuditoryMapModel(auditoryMapper*m);

    optix::Buffer GetOutputSoundBuffer() {
        return audioM->GetOutput();
        //return vaBasicObject::GetContext()["sysAuditoryOutputBuffer"]->getBuffer();
    };

    /**
*Binds a buffer of a particular name to audio output buffer
*/
    void setAudioBuffer() {
        vaBasicObject::GetContext()[AudioOutputBuffer]->setBuffer(audioM->GetOutput());
        vaBasicObject::GetContext()[AudioMode]->setInt(int(audioM->GetMode()));
    }
    /**
    *Binds a buffer of a particular name to visual output buffer
    */
    void setOpticBuffer()
    {
        vaBasicObject::GetContext()[OutputBuffer]->set(opticM->GetOutput());
    }

    void GetAudioDim(int& w, int& h) {
        h = audioM->GetHeight();
        w = audioM->GetWidth();
    }

    enum {
        OPTIC_RAYCASTING,
        AUDITORY_RAYCASTING
    };

    enum RenderModes {
        INTERACTIVE_CAMERA, /*<Mode by default. Rendering through defined camera with interactive manipulation
                            of its parameter (pan, zoom, etc.). In this mode the optix context launches rendering
                            of optical model. In this mode only scene geometry and optic materials are rendered.
                            See vaBasicRenderer::LaunchOpticContext() function*/

        COMPUTE_SOUND, /*<Auditory ray-casting. In this mode the optix context launches rendering
                       of optical model. See vaBasicRenderer::LaunchAudioContext() function.
                       The result of procedure is the computed by ray-casting auditory properties
                       defined with scene geometry and auditory materials and configured on host OpenAL
                       auditory scene representation ready for playing  : set of sound sources,
                       sampled sound waves, configured HRTF and etc.

                       If the following scene was computed, it would be automatically played if the RenderModes::PLAY_ANIMATION
                       is activated
                       */

        PLAY_ANIMATION,  /*<Rendering of visual-auditory dynamic scene. */

        INTERACTIVE_WIDGET  /*<Interactive manipulation through assigned widget. The widgets are similar to VTK widgets and are used for
                            interactive definition of various optical/geometry/auditory parameters, that are considered once the renderer
                            */
    };

    vaBasicRenderer(bool init);

    ~vaBasicRenderer() {};
    /* sets interop*/
    void SetInterop(bool inter) {
        if (opticM == nullptr) {
            opticM = std::unique_ptr<opticalModel>(new opticalModel());
        }
        opticM->SetInterop(inter);
    };

    /*
    Sets external optic model*/
    virtual void SetOpticModel(vaBasicModel* m)
    {
        opticM = std::unique_ptr<vaBasicModel>(m);
    }

    /*
    Sets external auditory model*/
    virtual void SetAudioModel(auditoryModel* m)
    {
        audioM = std::unique_ptr<auditoryModel>(m);
    }
    /**
    *Sets dimensions for optical Model
    */
    void SetOpticalDims(int width, int height)
    {
        //std::cout << "OPtical set" << std::endl;
        if (opticM == nullptr) {
            opticM = std::unique_ptr<opticalModel>(new opticalModel());
        }

        opticM->SetDim(width, height);
        //std::cout << "OPtical set 2" << std::endl;
    }

    /**
    *Sets dimensions for optical Model
    */
    void SetAuditoryDims(int width, int height)
    {
        if (audioM == nullptr) {
            audioM = std::unique_ptr<auditoryModel>(new auditoryModel());
        }
        audioM->SetDim(width, height);
    }

    void GetAuditoryDims(int& width, int& height)
    {
        height = audioM->GetHeight();
        width = audioM->GetWidth();
        // GetDim(width, height);
    }
    RenderModes GetMode() { return m_mode; };
    void SetMode(RenderModes m) { m_mode = m; };
    virtual void Update() {};

    void SetUpRenderer(); /*<Inits OpenGL, OpenAL ; Sets All program and Initialise the renderer (InitRenderer())*/

    void SetDynamic(bool type) {
        m_isDynamic = type;
        //dynamic scene parameters
        vaBasicObject::GetContext()["isDynamic"]->setInt(type);
        vaBasicObject::GetContext()["TimeSound"]->setFloat(0);
    };
    void SetAuditory(bool type) {
        m_isAuditory = type;
        vaBasicObject::GetContext()["computeAuditoryRendering"]->setInt(type);
    };

    bool isDynamic() {
        return m_isDynamic;
    }
    bool isAuditory() { return m_isAuditory; }

    /*Sets Time frame for rendering*/
    void   SetTime(float time);

    /*Sets Multiscale parameter for multiscale rendering*/
    virtual void   SetMultiscaleParam(float time);
    float GetMultiscaleParam() {
        return vaBasicObject::GetContext()["MultiscaleParam"]->getFloat();
    }
    /*Gets current time frame*/
    float GetTime() {
        return m_timeSound;
    }

    /*Adds geometry actor to scene for rendering */
    void AddActor(vaBasicActor* act); //{ m_act = act; }

    /*Returns i-th actor*/
    vaBasicActor* GetActor(int i) { return m_act[i]; }
    int GetNumOfActors() {
        return m_act.size();
    }

    /* Launches ray-tracing of visual stimuli*/
    virtual void LaunchOpticContext();

    /* Launches ray-tracing of auditory stimuli*/
    void LaunchAuditoryContext();

    // void StartPlayingAnimation();

    //procedures for optical model
    virtual void Reshape(int width, int height) {};
    virtual bool Render() { return true; };
    virtual void Display() {};
    /*
    Activates optical materials for visual stimuli rendering
    */
    void ActivateOpticalModel();
    /*
    Activates auditory materials for visual stimuli rendering
    */
    void ActivateAuditoryModel();

    /*
    overwrites default auditory ray generation program by widget's
    */
    void SetAuditoryRayGenerationFromWidget(std::string name, optix::Program prog);

    virtual void UpdateParam() {};

    //temporary for sdf
    void SetMissProgSDFProg(optix::Program sdfProg);
    void SetMissProgSDFProg2(optix::Program sdfProg);

private:
    bool m_AudioWidget;
    RenderModes m_mode;
    std::vector<vaBasicActor*> m_act;
    std::string m_builder;

    float m_timeSound; //time variable
    bool m_isDynamic;
    bool m_isAuditory;

    //stored names of minimul 3 following programs
    //raygeneration_program
    //exception_program
    //miss_program

    //those names are reserved and can't be changed
    std::string raygeneration_program;
    std::string exception_program;
    std::string miss_program;

    //similar for auditory tracing context launch
    std::string auditory_raygeneration_program;
    std::string auditory_exception_program;
    std::string auditory_miss_program;

    // The root node of the OptiX scene graph (sysTopObject)
    optix::Group        m_rootGroup;
    optix::Acceleration m_rootAcceleration;

protected:
    /*Inits default models*/
    virtual void InitDefaultModels();

    void InitDefaults();
    /*
    *References to auditory  model
    */
    std::unique_ptr<auditoryModel> audioM;
    /*
    *References to optical  model
    */
    std::unique_ptr<vaBasicModel> opticM;
    /*
    Inits main ray-tracing parameters.
    1) Identifies number of entry poings (auditory ray-tracing, visual, haptic
    2) Binds according output buffers

    Please note:
    In case of auditory rendering by default rays are shouted from the camera position. In other words,
    listener position. However, this can be changed by interactive widgets, that can set their own programs
    for auditory stimuli ray-casting, and ray-shouting procedures. In this case, the default
    void vaBasicRenderer::SetAuditoryRayGenerationProg()
    is overwritten by widget program
    */
    virtual void InitRenderer();

    //animation mode activated
    bool PlayAnimation(float play_time);

    //should be in some unified place
    std::map<std::string, optix::Program> m_mapOfPrograms;

    void InitProg(std::string prog, std::string file, std::string name);

    virtual void SetRayGenerationProg();
    virtual void SetExceptionProg();
    virtual void SetMissProg();

    //similar for auditory
    virtual void SetAuditoryRayGenerationProg();

    void SetAuditoryExceptionProg();
    void SetAuditoryMissProg();

    void SetRayGenerationProgName(std::string name) { raygeneration_program = name; };
    void SetExceptionProgName(std::string name) { exception_program = name; };
    void SetMissProgName(std::string name) { miss_program = name; };

    //similar for auditory
    void SetAuditoryRayGenerationProgName(std::string name) { auditory_raygeneration_program = name; };
    void SetAuditoryExceptionProgName(std::string name) { auditory_exception_program = name; };
    void SetAuditoryMissProgName(std::string name) { auditory_miss_program = name; };

    std::string GetRayGenerationProgName() { return raygeneration_program; };
    std::string GetExceptionProgName() { return exception_program; };
    std::string GetMissProgName() { return miss_program; };

    //auditory
    std::string GetAuditoryRayGenerationProgName() { return auditory_raygeneration_program; };
    std::string GetAuditoryExceptionProgName() { return auditory_exception_program; };
    std::string GetAuditoryMissProgName() { return auditory_miss_program; };

    /*acceleration structure on top*/
    void InitAcceleration();
    void SetPrograms(); /*<initialize all programs*/
    void InitializePrograms();
};

#endif