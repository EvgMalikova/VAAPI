#pragma once
/*=========================================================================
Program:   Auditory and Optic interop classes

=========================================================================*/

#ifndef vaAudioModel_h
#define vaAudioModel_h

#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <vector>

#include "macros.h"
#include "vaBasicObject.h"
#include <../shaders/renderer/per_ray_data.h>

#include "stkSound.h"
#include "predefined.h"
#include "vaBasicModel.h"

/**
* \class  auditoryModel
* \brief The class responsible for rendering auditory propertis
*
* The model is used by vaRenderer to render auditory image of data
* The class is mainly responsible for cuda/openAL interop
*/

//for music scale
const int C6 = 60; // 1046.5;
const int C2 = 48;// 65.406;

/**
* Mapper to auditory waves
*TODO: make a part of Mapper to be called
*/
class auditoryMapper : public vaBasicObject
{
public:
    auditoryMapper() {};
    ~auditoryMapper() {};

    virtual void Map(float maxDist, std::vector<stk::StkFrames>& scannedRays, std::vector<primDes> desc, int scale) {}; /*<Maps the desc to auditory ways and stores position of each wave. The output is scanned Rays*/
    float getAtomSound(int type);
    int getAtomNumber(std::string type);
protected:
};
/**
* Mapper to auditory waves
* other mapping style

TODO: clean the entire architecture here
*/
class auditoryMapperCmaj : public auditoryMapper
{
public:
    auditoryMapperCmaj() {};
    ~auditoryMapperCmaj() {};
    // virtual void Map(float maxDist, std::vector<stk::StkFrames>& scannedRays, std::vector<primDes> desc, int scale) override; /*<Maps the desc to auditory ways and stores position of each wave. The output is scanned Rays*/
    virtual void Map(float maxDist, std::vector<stk::StkFrames>& scannedRays, std::vector<primDes> desc, int scale); /*<Maps the desc to auditory ways and stores position of each wave. The output is scanned Rays*/
};
/**
* Mapper to auditory waves
* other mapping style

TODO: clean the entire architecture here
*/
class auditoryMapperPlucked : public auditoryMapper
{
public:
    auditoryMapperPlucked() {};
    ~auditoryMapperPlucked() {};
    // virtual void Map(float maxDist, std::vector<stk::StkFrames>& scannedRays, std::vector<primDes> desc, int scale) override; /*<Maps the desc to auditory ways and stores position of each wave. The output is scanned Rays*/
    virtual void Map(float maxDist, std::vector<stk::StkFrames>& scannedRays, std::vector<primDes> desc, int scale); /*<Maps the desc to auditory ways and stores position of each wave. The output is scanned Rays*/
};

class auditoryModel : public vaBasicModel
{
public:

    auditoryMapper* map;
    enum AudioScanMode /*<Scannng parameter for shoouting rays pattern*/
    {
        LINE,
        GRID,
        CONE
    };

    enum AudioMapMode /*<Scannng parameter for shoouting rays pattern*/
    {
        CMAJSCALE,
        PlUCKED
    };
    auditoryModel() {
        //m_bufferOutput = nullptr;
        m_distNorm = 0;
        m_mode = AudioScanMode::GRID;
        scale = 20;
        map = nullptr;
        // m_MappingMode = AudioMapMode::CMAJSCALE;
    };
    ~auditoryModel() { ClearAllAudio(); delete map; };

    AudioScanMode GetMode() { return m_mode; };
    /*void SetAudioMapMode(AudioMapMode m) {
        m_MappingMode = m;
    }*/
    virtual void Init(); /*<Inits OpenAL*/
    void SetAuditoryMapper(auditoryMapper* mm) { map = mm; }
    virtual void UpdateBuffer();/*<The procedure technicall gets the computed auditory rays. The output is desc */
    void MapBuffer() {
        if (map == nullptr)
            map = new auditoryMapper(); //create default auditory mapper
        /*switch (m_MappingMode)
        {
        case AudioMapMode::CMAJSCALE:
            map.Map1(m_distNorm, scannedRays, desc, scale);
            break;
        case PlUCKED:
            map.Map2(m_distNorm, scannedRays, desc, scale);
        }*/

        map->Map(m_distNorm, scannedRays, desc, scale);
    }; /*<Maps data to audio to perform further rendering operating selected audio mapping model*/
    virtual void Render();
    //UpdateBuffer;
    //Map
    //Render

    virtual void BindBuffer(optix::Context context); //initBuffer at the same time

    virtual void Reshape(int width, int height);
    void SetAudioScanningParam(AudioScanMode m);

    //TODO: create a separate AuditoryMapping class
    //
    float GetNormaKoeff();

    void CheckAllAudio(float time);

private:
    //AudioMapMode m_MappingMode;
    std::vector<primDes> desc;

    AudioScanMode m_mode;
    float m_distNorm;

    std::vector<stk::StkFrames> scannedRays;
    // std::vector<optix::float2> xPosRay;
    // std::vector<optix::float2> yPos;
    int scale;
    //played description
    std::vector<stkSound::soundDesc> descSs2;
protected:

    //-----------------------------
    //Sound functions
    //-----------------------------
    void ComputeSoundRaycast(float distComp);
    void ConfigureHRTF();

    void ClearAllAudio();
};

#endif