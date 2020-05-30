/*=========================================================================
Program:   VolumeRenderer
Module:    optixReader.h

=========================================================================*/
/**
* @class  optixReader
* @brief
*
* The class creates a basic triangular plane for Optix use. 
* Architecture much inspired by VTK (http://vtk.org) implementation
*
*/

#ifndef optixReader_h
#define optixReader_h



#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include "macros.h"
/*------------------
*
*Abstract class for Triangulated Geometry
*
*------------------------*/
template <class T>
class optixReader 
{
public:
    optixReader() {
        file = "";
        data = std::shared_ptr<T>(new T());
    };
    ~optixReader() {};


    virtual void Update() { ReadFile(); };
    virtual std::shared_ptr<T> GetOutput() { return data; };
    

    opxSetMacro(file, std::string);
    opxGetMacro(file, std::string);
private:
    std::shared_ptr<T> data;
    std::string file;


protected:
    virtual void Modified() {};
    virtual void ReadFile() {};


};


#endif