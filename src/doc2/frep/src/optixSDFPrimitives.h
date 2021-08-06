/*=========================================================================
Program:   VolumeRenderer
Module:    optixSDFPrimitives.h

=========================================================================*/

#ifndef optixSDFPrimitives_h
#define optixSDFPrimitives_h

#include "optixSDFBasicPrimitives.h"
#include "renderTypes.h"

/**
* \class  optixSDFSphere
* \brief  SDF sphere primitive
*
* For math  [see] (https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm)
*
*/

class optixSDFSphere : public optixSDFPrimitive {
public:

    optixSDFSphere() { };
    ~optixSDFSphere() {};

    virtual void SetCenter1(optix::float3 center);
    virtual void SetRadius1(optix::float3 radius);

protected:

    virtual void InitCallableProg();
};

/**
* \class  optixSDFBox
* \brief  SDF Box primitive
*
* For math  [see] (https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm)
*
*/

class optixSDFBox : public optixSDFPrimitive {
public:

    optixSDFBox() { };
    ~optixSDFBox() {};

    virtual void SetCenter1(optix::float3 center);
    virtual void SetDims(optix::float3 radius);

protected:

    virtual void InitCallableProg();
};

/**
* \class  optixSDFTorus
* \brief  SDF Torus primitive
*
* For math [see] (https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm)
*
*/
class optixSDFTorus : public optixSDFPrimitive {
public:

    optixSDFTorus() { };
    ~optixSDFTorus() {};

    virtual void SetCenter1(optix::float3 center);
    virtual void SetRadius1(optix::float3 radius);

protected:

    virtual void InitCallableProg();
};

/**
* \class  sdfMicrosturcture
* \brief  An example of simple microstructure
*
* For math  [see] (https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm)
*
*/

class sdfMicrosturcture : public optixSDFPrimitive {
public:

    sdfMicrosturcture() { };
    ~sdfMicrosturcture() {};

    virtual void SetCenter1(optix::float3 center);
    virtual void SetDims(optix::float3 radius);

protected:

    virtual void InitCallableProg();
};

/**
* \class  optixSDFHand
* \brief  SDF Hand primitive for leap motion
*
*
*/
class optixSDFHand : public optixSDFPrimitive {
public:

    optixSDFHand() { };
    ~optixSDFHand() {};

    virtual void SetCenter1(optix::float3 center);
    virtual void SetRadius1(optix::float3 radius);

protected:

    virtual void InitCallableProg();
};

/**
* \class  sdfTexture
* \brief  Generates a dynamic SDF primitive on base of sdf textures
*
* The sdf textures can be obtained on base of signed distance transform procedure. In the API
* the ITK signed distance transform is used to obtain such textures. See sdfTextureReader
* class: sdfTextureReader::PreprocessData() function for example.
*
*/
class sdfTexture : public optixSDFPrimitive {
public:

    sdfTexture() { };
    ~sdfTexture() {};

    void SetTexture(optix::TextureSampler samp, sdfParams* param);

private:
    std::vector<optix::TextureSampler> sampler;
    std::vector<sdfParams> m_param;
protected:

    virtual void InitCallableProg();
    // virtual void SetCallableProg(); //because we set parameters here

    virtual void SetParameters();
};

/**
* \class  sdfCPKMol
* \brief  Example class for simple molecular geometry
*
* Uses SDF sphere primitive to render molecule atoms. A simple CPK molecule representation
*
*/

class sdfCPKMol : public optixSDFGeometry {
public:

    sdfCPKMol() {
        // optixSDFGeometry::optixSDFGeometry();
        primNumber = 0;
        m_typeRange = optix::make_float2(0);
        numFrames = 0;
        m_scale = 1.0;
        m_shift = optix::make_float3(0);
    };
    ~sdfCPKMol() {};

    //Set
    void SetRadius(std::vector<float> rad);
    optix::Buffer GetRadius() { return radBuffer; }

    void SetCenter(std::vector<optix::float3> c);
    void SetCenter(std::vector<optix::float3> c, int frame);

    optix::Buffer GetCenter() { return centersBuffer; }

    void SimulateDynamic(int numFrames, std::vector<optix::float3> c);
    void SetType(std::vector<int> type);

    //virtual void Update() { CreateGeometry(); };
    virtual optix::Geometry GetOutput() {
        return optixSDFGeometry::GetOutput();
    }
    void SetScale(float sc);
    void SetShift(optix::float3 sc);

    optix::float2 GetTypeRange() { return m_typeRange; };
    void SetNumFrames(int fr) { numFrames = fr; };
protected:
    float m_scale;
    optix::float3 m_shift;
    optix::Buffer radBuffer;
    optix::Buffer centersBuffer;
    optix::Buffer typeBuffer;

    //Porcedure for geometry creation
    virtual void CreateGeometry();
    //virtual void GenerateGeometry();
    virtual void Initialize();
    virtual void SetCallableProg();
    void SetPrimNumber(int n) { primNumber = n; };

private:
    int primNumber;
    optix::float2 m_typeRange;
    int numFrames;
};

/**
* \class  sdfHeterogeneous
* \brief  Main class for heterogeneous objects with simple molecular geometry
*
* Uses SDF sphere primitive to render molecule atoms.
*/

class sdfHeterogeneous : public optixSDFGeometry {
public:

    sdfHeterogeneous() {
        // optixSDFGeometry::optixSDFGeometry();
        primNumber = 0;
        m_typeRange = optix::make_float2(0);
        numFrames = 0;

        m_constructiveTreeOptimName = "bondProg";
        m_readDataName = "readTimeData";
    };
    ~sdfHeterogeneous() {};

    //virtual void Update() { CreateGeometry(); };
    virtual optix::Geometry GetOutput() {
        return optixSDFGeometry::GetOutput();
    }

    optix::float2* GetTypeRange() { return &m_typeRange; };
    void SetNumFrames(int fr) { numFrames = fr; };

    std::string GetConstructiveTreeOptimisationProgName() {
        return m_constructiveTreeOptimName;
    };
    std::string GetReadDataProgName() {
        return m_readDataName;
    };

    int GetNumFrames() {
        return numFrames;
    }
    int GetPrimNumber() {
        return primNumber;
    }
protected:

    //Porcedure for geometry creation
    virtual void CreateGeometry();
    //virtual void GenerateGeometry();
    virtual void Initialize();
    virtual void SetCallableProg();
    void SetPrimNumber(int n) { primNumber = n; };
    /* optix intersection program - main frame prog*/
    virtual void InitMainIntersectionProg();

    /* optix bounding volume prog*/
    virtual void InitBoundingBoxProg() {};

    /*compile primitive program*/
    virtual void InitSDFPrimitiveProg() {};

    /*constructive tree subbounding volumes*/
    virtual void InitConstructiveTreeOptimisationProg() {};

    /*Process dynamic data prog*/
    virtual void InitReadDataProg() {};

    optix::Program  GetProgByName(std::string name);

    void SetConstructiveTreeOptimisationProgName(std::string name) {
        m_constructiveTreeOptimName = name;
    };
    void SetReadDataProgName(std::string name) {
        m_readDataName = name;
    };

    virtual void SetParameters();
private:
    int primNumber;
    optix::float2 m_typeRange;
    int numFrames;
    std::string m_constructiveTreeOptimName;
    std::string m_readDataName;
};

class sdfHCPKMol : public sdfHeterogeneous {
public:

    sdfHCPKMol() {
        // optixSDFGeometry::optixSDFGeometry();

        m_scale = 1.0;
        m_shift = optix::make_float3(0);
    };
    ~sdfHCPKMol() {};

    //General procedures for simple primitives
    //Set
    void SetRadius(std::vector<float> rad);
    optix::Buffer GetRadius() { return radBuffer; }

    void SetCenter(std::vector<optix::float3> c);
    void SetCenter(std::vector<optix::float3> c, int frame);

    optix::Buffer GetCenter() { return centersBuffer; }

    void SetType(std::vector<int> type);

    void SetScale(float sc);
    void SetShift(optix::float3 sc);

protected:
    float m_scale;
    optix::float3 m_shift;
    optix::Buffer radBuffer;
    optix::Buffer centersBuffer;
    optix::Buffer typeBuffer;

    /* optix bounding volume prog*/
    virtual void InitBoundingBoxProg();

    /*compile primitive program*/
    virtual void InitSDFPrimitiveProg();

    /*constructive tree subbounding volumes*/
    virtual void InitConstructiveTreeOptimisationProg();

    /*Process dynamic data prog*/
    virtual void InitReadDataProg();
};

/**
* \class  sdfHMicrostructure
* \brief  Example class that redefines mapping with just microstructure prim
*
* Uses SDF microstructure primitive to render molecule atoms.
*/

class sdfHMicrostructure : public  sdfHCPKMol {
public:

    sdfHMicrostructure() {
    };
    ~sdfHMicrostructure() {};

protected:

    /*compile primitive program*/
    virtual void InitSDFPrimitiveProg(); //just one function to redefine
};

/**
* \class  sdfHTetra
* \brief  Class for tetrahedral meshes
* Redefines:
* *SDF primitive
  *Bounding box as center is to be evaluated
  *Constructive optimisation as center is to be evaluated
*
* Uses SDF microstructure primitive to render molecule atoms.
*/

class sdfHTetra : public  sdfHeterogeneous {
public:

    sdfHTetra() {
    };
    ~sdfHTetra() {};

    //   void SetTetra(std::vector<optix::float3> c);

    optix::Buffer GetTetra() { return tetraBuffer; }

protected:

    /*compile primitive program*/
    virtual void InitSDFPrimitiveProg(); //just one function to redefine
                                         /* optix bounding volume prog*/
    virtual void InitBoundingBoxProg();

    /*constructive tree subbounding volumes*/
    virtual void InitConstructiveTreeOptimisationProg();

    /*Process dynamic data prog*/
    virtual void InitReadDataProg();
private:
    optix::Buffer tetraBuffer;
};

/**
* \class  sdfCPKDynMol
* \brief  Example class for dynamic molecular geometry
*
* Uses SDF sphere primitive to render molecule atoms. A simple CPK molecule representation
*
*/

class sdfCPKDynMol : public sdfCPKMol {
public:

    sdfCPKDynMol() {
    };
    ~sdfCPKDynMol() {};

    //Set
    //void SetRadius2(std::vector<float> rad);
   // optix::Buffer GetRadius2() { return radBuffer2; }

    void SetCenter2(std::vector<optix::float3> c);
    optix::Buffer GetCenter2() { return centersBuffer2; }

    //void SetType(std::vector<int> type);

    //virtual void Update() { CreateGeometry(); };
    virtual sdfGeo* GetOutputDesc() {
        return optixSDFGeometry::GetOutputDesc();
    }

    //  optix::float2 GetTypeRange() { return m_typeRange; };

protected:

    // optix::Buffer radBuffer;
    optix::Buffer centersBuffer2;
    //optix::Buffer typeBuffer;

    //Porcedure for geometry creation
   // virtual void CreateGeometry();
    //virtual void GenerateGeometry();
    virtual void Initialize();
    virtual void SetParameters();
    // virtual void SetCallableProg();
private:
    // int primNumber;
    // optix::float2 m_typeRange;
};

/**
* \class  sdfBallSticksMol
* \brief  Example class for simple Balls and sticks molecular geometry
*
*
*/

class sdfBallSticksMol : public sdfCPKMol {
public:

    sdfBallSticksMol() {
    };
    ~sdfBallSticksMol() {};

    //Set
    //void SetRadius2(std::vector<float> rad);
    // optix::Buffer GetRadius2() { return radBuffer2; }

    void SetBonds(std::vector<optix::int2> c);
    optix::Buffer GetBonds() { return bondsBuffer; }

    //void SetType(std::vector<int> type);

    //virtual void Update() { CreateGeometry(); };
    virtual sdfGeo* GetOutputDesc() {
        return optixSDFGeometry::GetOutputDesc();
    }

    //  optix::float2 GetTypeRange() { return m_typeRange; };

protected:

    // optix::Buffer radBuffer;
    optix::Buffer bondsBuffer;
    //optix::Buffer typeBuffer;

    //Porcedure for geometry creation
    // virtual void CreateGeometry();
    //virtual void GenerateGeometry();
    virtual void Initialize();
    virtual void SetParameters();
    virtual void SetCallableProg();

    optix::Program GetProgByName(std::string name);

private:
    // int primNumber;
    // optix::float2 m_typeRange;
    RTprogram boundProg;
};

/**
* \class  sdfMolBallSticksMol
* \brief  Example class for simple Balls and sticks molecular geometry
*
*
*/

class sdfMolBallSticksMol : public sdfBallSticksMol {
public:
    const int maxMolDim = 4;
    /*typedef struct {
        optix::int2 bond_id;
    } Molecules;*/
    sdfMolBallSticksMol() {
    };
    ~sdfMolBallSticksMol() {};

    void SetMols(std::vector<Molecule> c, int maxMolSize);
    optix::Buffer GetMols() { return molsBuffer; }

    //  optix::float2 GetTypeRange() { return m_typeRange; };

protected:

    // optix::Buffer radBuffer;
    optix::Buffer molsBuffer;
    //optix::Buffer typeBuffer;

    //Porcedure for geometry creation
    // virtual void CreateGeometry();
    //virtual void GenerateGeometry();
    virtual void Initialize();
};
#endif