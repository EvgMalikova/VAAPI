/*=========================================================================
Program:   VolumeRenderer
Module:    optixSDFPrimitives.h

=========================================================================*/

#ifndef sdfHeteroGeom_h
#define sdfHeteroGeom_h

#include "optixSDFPrimitives.h"
#include "renderTypes.h"

/**
* \class  sdfHeterogeneous
* \brief  Main class for heterogeneous objects with simple molecular geometry
*
* Uses SDF sphere primitive to render molecule atoms.
*/

class sdfHeterogeneous : public optixSDFGeometry {
public:
    enum ObjectType {
        DIM_0D, //only geometry, points
        DIM_1D, //geometry and topology, points connected with lines
        DIM_4D,  //Cell volume, example tetrahedron in mesh
        MULTISCALE,
        GENERAL  //defalt description with not prespecified type
    };
    sdfHeterogeneous() {
        // optixSDFGeometry::optixSDFGeometry();
        primNumber = 0;
        m_typeRange = optix::make_float2(0);
        numFrames = 0;

        m_constructiveTreeOptimName = "bondProg";
        m_readDataName = "readTimeData";

        m_shift = optix::make_float3(0);
        m_primType = ObjectType::GENERAL;
    };
    ~sdfHeterogeneous() {};

    virtual sdfGeo* GetOutputDesc() {
        return optixSDFGeometry::GetOutputDesc();
    }

    static std::string GetPrimProgramName(ObjectType type);
    //--------
    /* Reading data
    */
    //at least points without any topological info are read
    void SetCenter(std::vector<optix::float3> c);
    void SetCenter(std::vector<optix::float3> c, int frame);

    optix::Buffer GetCenter() { return centersBuffer; }

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
    void SetShift(optix::float3 sc);
    ObjectType GetPrimType() { return m_primType; };
protected:
    /*Returns name of callable id function in cuda code*/
    std::string GetPrimProgName();

    void SetPrimType(ObjectType type) { m_primType = type; };
    //Porcedure for geometry creation
    virtual void CreateGeometry();
    //virtual void GenerateGeometry();
    virtual void Initialize();

    /* sets callable program depending on type of
    heterogeneous object*/
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
    ObjectType m_primType;
    optix::float3 m_shift;
    optix::Buffer centersBuffer;
    int primNumber;
    optix::float2 m_typeRange;
    int numFrames;
    std::string m_constructiveTreeOptimName;
    std::string m_readDataName;
};

/**
* \class  sdfHCPKMol
* \brief  CPK molecules mapping. Currently used as an example of class with minimum
* of procedures overwriting

*For efficiency see sdfHeterogeneous0D
*
* Uses SDF sphere primitive to render molecule atoms.
*/

class sdfHCPKMol : public sdfHeterogeneous {
public:

    sdfHCPKMol() {
        // optixSDFGeometry::optixSDFGeometry();
        m_scale = 1.0;
    };
    ~sdfHCPKMol() {};

    //General procedures for simple primitives
    //Set
    void SetRadius(std::vector<float> rad);
    optix::Buffer GetRadius() { return radBuffer; }

    void SetType(std::vector<int> type);

    void SetScale(float sc);

protected:
    float m_scale;

    optix::Buffer radBuffer;

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
* \class  sdfHeterogeneous0D
* \brief  CPK molecules mapping. Currently used as an example of class with minimum
* of procedures overwriting

*For efficiency see sdfHeterogeneous1D
*
* Uses SDF sphere primitive to render molecule atoms.
*/

class sdfHeterogeneous0D : public sdfHeterogeneous {
public:

    sdfHeterogeneous0D() {
        // optixSDFGeometry::optixSDFGeometry();
        m_scale = 1.0;
        //Definies type of primitive
        SetPrimType(sdfHeterogeneous::ObjectType::DIM_0D);
    };
    ~sdfHeterogeneous0D() {};

    //General procedures for simple primitives
    //Set
    void SetRadius(std::vector<float> rad);
    optix::Buffer GetRadius() { return radBuffer; }

    void SetType(std::vector<int> type);

    void SetScale(float sc);

protected:
    float m_scale;

    optix::Buffer radBuffer;

    optix::Buffer typeBuffer;

    virtual void Initialize();
    virtual void InitMainIntersectionProg();
    /* optix bounding volume prog*/
    virtual void InitBoundingBoxProg();

    /*compile primitive program*/
    virtual void InitSDFPrimitiveProg();

    virtual void SetParameters() {};//no extra callable programs

private:
    std::string GetHeteroName();
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

class sdfHMicro : public  sdfHeterogeneous0D {
public:

    sdfHMicro() {
    };
    ~sdfHMicro() {};

protected:

    /*compile primitive program*/
    virtual void InitSDFPrimitiveProg(); //just one function to redefine
};

/**
* \class  sdfHeterogeneous1D
* \brief  sdfHeterogeneous1D Basic class for 1D types of
 primitives. Example molecules with balls and sticks mapping
*
* Uses SDF balls and sticks representation
*/
class sdfHeterogeneous1D : public sdfHeterogeneous0D {
public:

    sdfHeterogeneous1D() {
        SetPrimType(sdfHeterogeneous::ObjectType::DIM_1D);
    };
    ~sdfHeterogeneous1D() {};

    //General procedures for simple primitives
    //Set
    void SetBonds(std::vector<optix::int2> c);
    optix::Buffer GetBonds() { return bondsBuffer; }

protected:
    optix::Buffer bondsBuffer;
    virtual void InitMainIntersectionProg();
    /* optix bounding volume prog*/
    virtual void InitBoundingBoxProg();
};

class sdfHBondsSticks : public  sdfHeterogeneous1D {
public:

    sdfHBondsSticks() {
    };
    ~sdfHBondsSticks() {};

protected:

    /*compile primitive program*/
    virtual void InitSDFPrimitiveProg(); //just one function to redefine
};

class sdfHCrazyBonds : public  sdfHeterogeneous1D {
public:

    sdfHCrazyBonds() {
    };
    ~sdfHCrazyBonds() {};

protected:

    /*compile primitive program*/
    virtual void InitSDFPrimitiveProg(); //just one function to redefine
};
/**
* \class  sdfMoleculeBallSticks
* \brief  sdfMoleculeBallSticks for molecules
primitives. Example molecules with balls and sticks mapping
*
* Uses SDF balls and sticks representation
*/

class sdfMoleculeBallSticks : public sdfHeterogeneous1D {
public:
    const int maxMolDim = 4;
    /*typedef struct {
    optix::int2 bond_id;
    } Molecules;*/
    sdfMoleculeBallSticks() {
        SetPrimType(sdfHeterogeneous::ObjectType::MULTISCALE);
    };
    ~sdfMoleculeBallSticks() {};

    void SetMols(std::vector<Molecule> c, int maxMolSize);
    optix::Buffer GetMols() { return molsBuffer; }

    //  optix::float2 GetTypeRange() { return m_typeRange; };

protected:

    // optix::Buffer radBuffer;
    optix::Buffer molsBuffer;
    //optix::Buffer typeBuffer;

    virtual void InitMainIntersectionProg();
    /* optix bounding volume prog*/
    virtual void InitBoundingBoxProg();
    virtual void Initialize();
    virtual void InitSDFPrimitiveProg();
};
/**
* \class  sdfHeterogeneous1D
* \brief  sdfHeterogeneous1D Basic class for 1D types of
primitives. Example molecules with balls and sticks mapping
*
* Uses SDF balls and sticks representation
*/
class sdfHeterogeneous4D : public sdfHeterogeneous0D {
public:

    sdfHeterogeneous4D() {
        SetPrimType(sdfHeterogeneous::ObjectType::DIM_4D);
    };
    ~sdfHeterogeneous4D() {};

    //General procedures for simple primitives
    //Set
    void SetTetras(std::vector<optix::int4> c);
    optix::Buffer GetTetras() { return tetBuffer; }

protected:
    optix::Buffer tetBuffer;
    virtual void InitMainIntersectionProg();
    /* optix bounding volume prog*/
    virtual void InitBoundingBoxProg();

    virtual void InitSDFPrimitiveProg();
};

/**
* \class  sdfHCPKMol
* \brief  CPK molecules mapping
*
* Uses SDF balls and sticks representation
*/
class sdfHBallSticksMol : public sdfHCPKMol {
public:

    sdfHBallSticksMol() {
        // optixSDFGeometry::optixSDFGeometry();
    };
    ~sdfHBallSticksMol() {};

    //General procedures for simple primitives
    //Set
    void SetBonds(std::vector<optix::int2> c);
    optix::Buffer GetBonds() { return bondsBuffer; }

    optix::Program GetCallableProgSimple() {
        return GetProgByName("simple");
    };
protected:
    optix::Buffer bondsBuffer;
    virtual void Initialize();
    virtual void InitMainIntersectionProg();
    /* optix bounding volume prog*/
    virtual void InitBoundingBoxProg();

    /*compile primitive program*/
    virtual void InitSDFPrimitiveProg();

    virtual void SetCallableProg();

private:
    optix::Program m_simple;
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
* \class  sdfBallSticksMol
* \brief  Example class for simple Balls and sticks molecular geometry
*
*
*/

#endif