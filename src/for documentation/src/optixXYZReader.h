/*=========================================================================
Program:   VolumeRenderer
Module:    xyzReader.h

=========================================================================*/

#ifndef optixXYZReader_h
#define optixXYZReader_h
#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>
#include <vector>
#include "optixReader.h"

/*------------------
*
*Basic class for XYZ file reader
*
*------------------------*/
typedef struct {
    std::vector<optix::float3> centers;
    std::vector<float> rad;
    std::vector<int> type;
    optix::float3 bbox_min;
    optix::float3 bbox_max;

    /*getOutput(int i) {
    if(i==0)
    }*/
    //XYZ() {};
} XYZ;

/**
* \class  vaXYZReader
* \brief A basic class for reading molecule data from XYZ files
*
* Reads data from XYZ file, that is usually type of atom and coordinates.
* Returns basic information about structure like: Type, xyz and atomic radii
*
*/
class xyzReader : public optixReader<XYZ>
{
public:
    xyzReader()
    {
        optixReader<XYZ>::optixReader();
    };
    ~xyzReader() {};

    std::vector<optix::float3> GetOutput1() { return optixReader<XYZ>::GetOutput()->centers; };
    std::vector<float> GetOutput2() {
        return optixReader<XYZ>::GetOutput()->rad;
    };
    std::vector<int> GetOutput3() {
        return optixReader<XYZ>::GetOutput()->type;
    };

protected:

    virtual void ReadFile();
    int GetAtomNumber(std::string type);
    float GetAtomRadii(std::string type);
};

#endif