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

    /*getOutput(int i) {
    if(i==0)
    }*/
    //XYZ() {};
} MOL;

/**
* \class  vaXYZReader
* \brief A basic class for reading molecule data from XYZ files
*
* Reads data from XYZ file, that is usually type of atom and coordinates.
* Returns basic information about structure like: Type, xyz and atomic radii
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
protected:

    virtual void ReadFile();
    int GetAtomNumber(std::string type);
    float GetAtomRadii(std::string type);
};

#endif