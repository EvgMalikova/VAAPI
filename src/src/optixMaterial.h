/*=========================================================================
Program:   VolumeRenderer
Module:    vaMapper.h

=========================================================================*/
/**
* @class  vaMapper
* @brief
*
* The codes inspired by vtkMapper class. Returns optix::GeometryInstance with assigned material.
* Each material suggests definition of optical model , that treats geometry defined object as a sufrace
* or takes geometry and hetegoreneously defined attributes and maps them to optical and/or auditory properties
* for more details see ..heterogeneous objects modelling
*
*/

#ifndef optixMaterial_h
#define optixMaterial_h

#include "optixAbstractMaterial.h"
#include "sdfHeteroGeometry.h"
#include<vector>
/**
* \class Lambertian
*\brief Example of material for triangulated surfaces
*
* The class is used for comparison of logic with SDF mapper and demonstration purposes,
* It gives the idea how to cleate materials for triangulated surfaces, but idea is not further
* developed within API
*/
class Lambertian : public vaAuditoryMaterial {
public:
    Lambertian() {  };
    ~Lambertian() {};
    //vtkSetOptixFloat3Macro(albedo);

private:
    optix::float3 albedo;
protected:
    virtual void SetMaterialParameters() {
        //gi["albedo"]->set3fv(&albedo.x);
    }
    virtual void Initialize();
};

/**
* \class vaBasicMaterial
*\brief Simple Blin-Phong shading for whitted ray-tracing
*
* The class gives idea how to create surface materials for SDF objects (whitted ray-tracing),
*
*/
class vaBasicMaterial : public vaAuditoryMaterial {
public:
    vaBasicMaterial() { m_ColorMap = nullptr; };
    ~vaBasicMaterial() {};

    //TODO: something is wrong here - verify the entire structure
    void SetColorScheme(optix::Program pr) {
        m_ColorMap = pr;
        SetScalarMode(false);
    }
private:
    optix::Program m_ColorMap;
protected:
    virtual void SetMaterialParameters();
    virtual void Initialize();
    virtual void InitLights();
};

/**
* \class vaComplexVolumeMaterial
*\brief  a volumetric class for materials with post integration
*
*/
class vaComplexVolumeMaterial : public vaBasicMaterial {
public:
    vaComplexVolumeMaterial() { };
    ~vaComplexVolumeMaterial() {};

    void SetSDFProg(optix::Program sdfProg) {
        //m_prog.push_back(sdfProg);
        m_prog = sdfProg;
    };
private:

    //std::vector<optix::Program> m_prog;
    optix::Program m_prog;
protected:
    virtual void SetMaterialParameters();
    virtual void Initialize();
};

/**
* \class vaEAVolume
*\brief Emission-absorption Volume
*
*
*/
class vaEAVolume : public vaAuditoryMaterial {
public:
    enum MaterialType {
        SURFACE,
        VOLUME,
        TRANSP
    };
    vaEAVolume() {
        m_type = MaterialType::TRANSP; m_prog = nullptr;
    };
    ~vaEAVolume() {};

    void SetTexture(optix::TextureSampler samp);
    void AddLight(BasicLight* l);
    void SetType(MaterialType m) { m_type = m; };
    virtual void SetSDFProg(optix::Program sdfProg);
private:
    std::vector<optix::TextureSampler> sampler;

protected:
    MaterialType m_type;
    optix::Program m_prog;
    virtual void SetMaterialParameters();
    virtual void Initialize();
};

/*
Skip and perform post integration material
*/

class vaVolumeSDFComplex : public vaEAVolume {
public:

    vaVolumeSDFComplex() {
        //m_type = MaterialType::TRANSP; m_prog = nullptr;
    };
    ~vaVolumeSDFComplex() {};

protected:
    virtual void SetMaterialParameters();
    virtual void Initialize();
};

/*
Skip and perform post integration material
*/

class vaVolumeSDFHetero : public vaEAVolume {
public:

    vaVolumeSDFHetero() {
        m_hType = sdfHeterogeneous::ObjectType::GENERAL;
        m_postprocess = 0;
    };
    ~vaVolumeSDFHetero() {};
    void SetHeteroObjType(sdfHeterogeneous::ObjectType type) {
        if (type == sdfHeterogeneous::ObjectType::DIM_4D) //concave shapes
        {//concave shapes
         //only postprocessing
            m_postprocess = 1;
        }
        m_hType = type;
    };
    sdfHeterogeneous::ObjectType GetHeteroObjType() { return m_hType; };
    void SetPostprocess(int post) { m_postprocess = post; };
    optix::Program GetEvalProg();
    optix::Program GetColorProg();
protected:
    virtual void SetMaterialParameters();
    virtual void Initialize();
    virtual void CompileFunction();
    std::string GetEvalProgramName();

    int m_postprocess;
private:

    sdfHeterogeneous::ObjectType m_hType;
    optix::Program m_functionProg;
};

class vaVolumeSDFHeteroMultiscale : public vaVolumeSDFHetero {
public:

    vaVolumeSDFHeteroMultiscale() {
        SetHeteroObjType(sdfHeterogeneous::ObjectType::MULTISCALE);
    };
    ~vaVolumeSDFHeteroMultiscale() {};

protected:

    virtual void Initialize();
    virtual void CompileFunction();
};

/*! Optical material for SDF objects
//test case, not finished
*/
class SDFMaterial2 : public vaAuditoryMaterial {
public:
    SDFMaterial2() {  };
    ~SDFMaterial2() {};

    virtual void Update();

protected:
    virtual void SetMaterialParameters() {
        //gi["albedo"]->set3fv(&albedo.x);
    }
    virtual void Initialize();
};

/**
* \class SDFVolumeMaterial
*\brief Simple Volume material with interactive highlighting, applied to several objects of various types
*
* Volume material with interactive highlightment. Emission-absorption model. The material suggests application to several objects of various types
* thus it works with optix::Buffers defining the types of object and transfer function.
* There is a dynamic highlightment of the objects as well, defined through concept of ray propagation.
* For details see how it works with auditory rendering.
*
*/

/*! Example of  volume material with interactive highlightment*/
class SDFVolumeMaterial : public vaAuditoryMaterial {
public:
    SDFVolumeMaterial() { };
    ~SDFVolumeMaterial() {};

protected:
    virtual void SetMaterialParameters() {  };
    virtual void Initialize();
};

/*
For molecules
*/
class vaMolVolume : public vaBasicMaterial {
public:

    vaMolVolume() {
    };
    ~vaMolVolume() {};
private:

    optix::Program m_prog;

protected:

    virtual void Initialize();
};

class vaMolVolume2 : public vaBasicMaterial {
public:

    vaMolVolume2() {
    };
    ~vaMolVolume2() {};
    void SetSDFProg(optix::Program sdfProg) {
        //m_prog.push_back(sdfProg);
        m_prog = sdfProg;
    };
private:

    optix::Program m_prog;

protected:
    virtual void SetMaterialParameters();
    virtual void Initialize();
};

/*! Auditory volume material
Similar to volume ray-casting.
*/
class SDFAudioVolumeMaterial : public vaAuditoryMaterial {
public:
    SDFAudioVolumeMaterial() {  };
    ~SDFAudioVolumeMaterial() {};

protected:
    virtual void SetMaterialParameters() {};
    virtual void Initialize();
};

/*! Auditory volume material
Secondary rays generation for surfaces. Only reflection of rays
*/
class SDFAudioRayTraceMaterial : public vaAuditoryMaterial {
public:
    SDFAudioRayTraceMaterial() {  };
    ~SDFAudioRayTraceMaterial() {};

protected:
    //virtual void SetMaterialParameters() {};
    virtual void Initialize();
};

/*! Example of dynamic optical volume material
* Emission-absorption model
*/
class DynamicTextureVolumeMaterial : public vaAuditoryMaterial {
public:
    DynamicTextureVolumeMaterial() {  };
    ~DynamicTextureVolumeMaterial() {};

protected:
    virtual void SetMaterialParameters() {  };
    virtual void Initialize();
};

/*! Example of auditory volume material */
class AudioTextureVolumeMaterial : public vaAuditoryMaterial {
public:
    AudioTextureVolumeMaterial() {  };
    ~AudioTextureVolumeMaterial() {};

protected:
    virtual void SetMaterialParameters() {};
    virtual void Initialize();
};

#endif