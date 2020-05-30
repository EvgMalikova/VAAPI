/*=========================================================================
Program:   VolumeRenderer
Module:    xyzReader.h

=========================================================================*/

#ifndef optixSDFReader_h
#define optixSDFReader_h
#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>
#include <vector>
#include "optixReader.h"

/*------------------
*
*Basic class for SDF/Mol file reader
*
*------------------------*/

typedef struct {
    std::vector<optix::float3> centers;
    std::vector<float> rad;
    std::vector<int> type;
    optix::float3 bbox_min;
    optix::float3 bbox_max;
    std::vector<optix::int2> bonds;
    std::vector<Molecule> mols;
    int maxMolSize;
    /*getOutput(int i) {
    if(i==0)
    }*/
    //XYZ() {};
} MOL;

typedef struct {
    optix::float3 p1;
    optix::float3 p2;
    optix::float3 p3;
    optix::float3 p4;
} tetra;
typedef struct {
    std::vector<optix::float3> centers;
    std::vector<int> type;
    optix::float3 bbox_min;
    optix::float3 bbox_max;
    std::vector<optix::int4> tetra;
} TETRAHEDRAS;

/**
* \class  sdfReader
* \brief A basic class for reading molecule data from SDF files
*
* Reads data from sdf file and creates balls and sticks representation
* based on "bonds"
*
*/

class sdfReader : public optixReader<MOL>
{
public:
    sdfReader()
    {
        optixReader<MOL>::optixReader();
    };
    ~sdfReader() {};

    std::vector<optix::float3> GetOutput1() { return optixReader<MOL>::GetOutput()->centers; };
    std::vector<float> GetOutput2() {
        return optixReader<MOL>::GetOutput()->rad;
    };
    std::vector<int> GetOutput3() {
        return optixReader<MOL>::GetOutput()->type;
    };
    std::vector<optix::int2> GetOutput4() {
        return optixReader<MOL>::GetOutput()->bonds;
    };
    optix::float3 GetCenter() {
        optix::float3 bmax = optixReader<MOL>::GetOutput()->bbox_max;
        optix::float3 bmin = optixReader<MOL>::GetOutput()->bbox_min;

        return (bmax + bmin) / 2;
    };

    optix::float3 GetBMax() {
        optix::float3 f = optixReader<MOL>::GetOutput()->bbox_max;
        f += optix::make_float3(17);
        return f;
    }

    optix::float3 GetBMin() {
        optix::float3 f = optixReader<MOL>::GetOutput()->bbox_min;
        f -= optix::make_float3(17);
        return f;
    };
protected:

    virtual void ReadFile();
    int GetAtomNumber(std::string type);
    float GetAtomRadii(std::string type);
};
/*
\class  tetReader
* \brief A class for tetrahedra mesh reading
*
*/

class tetReader : public optixReader<TETRAHEDRAS>
{
public:
    tetReader()
    {
        optixReader<TETRAHEDRAS>::optixReader();
    };
    ~tetReader() {};

    std::vector<optix::float3> GetOutput1() { return optixReader<TETRAHEDRAS>::GetOutput()->centers; };
    std::vector<optix::int4> GetOutput2() {
        return optixReader<TETRAHEDRAS>::GetOutput()->tetra;
    };
    std::vector<int> GetOutput3() {
        return optixReader<TETRAHEDRAS>::GetOutput()->type;
    };

    optix::float3 GetCenter() {
        optix::float3 bmax = optixReader<TETRAHEDRAS>::GetOutput()->bbox_max;
        optix::float3 bmin = optixReader<TETRAHEDRAS>::GetOutput()->bbox_min;

        return (bmax + bmin) / 2;
    };

    optix::float3 GetBMax() {
        optix::float3 f = optixReader<TETRAHEDRAS>::GetOutput()->bbox_max;
        f += optix::make_float3(7);
        return f;
    }

    optix::float3 GetBMin() {
        optix::float3 f = optixReader<TETRAHEDRAS>::GetOutput()->bbox_min;
        f -= optix::make_float3(7);
        return f;
    };

protected:

    virtual void ReadFile();
};

/**
* \class  sdfMolReader
* \brief A basic class for reading molecule data from SDF files
*
* Reads data from sdf file and creates balls and sticks representation
* based on "Molecules". A start point for multiscale level of detail
*
*
*/

class sdfMolReader : public sdfReader
{
public:
    sdfMolReader()
    {
    };
    ~sdfMolReader() {};
    /*
    std::vector<optix::float3> GetOutput1() { return optixReader<MOL>::GetOutput()->centers; };
    std::vector<float> GetOutput2() {
        return optixReader<MOL>::GetOutput()->rad;
    };
    std::vector<int> GetOutput3() {
        return optixReader<MOL>::GetOutput()->type;
    };
    std::vector<optix::int2> GetOutput4() {
        return optixReader<MOL>::GetOutput()->bonds;
    };*/
    std::vector<Molecule> GetOutput5() {
        return optixReader<MOL>::GetOutput()->mols;
    };

protected:

    virtual void ReadFile();
    int GetMaxMolSize() {
        return optixReader<MOL>::GetOutput()->maxMolSize;
    }
    // int GetAtomNumber(std::string type);
    // float GetAtomRadii(std::string type);
};

#endif