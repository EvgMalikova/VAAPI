/*=========================================================================
Program:   VolumeRenderer
Module:    plySdfTextureReader.h

=========================================================================*/

#ifndef plySdfTextureReader_h
#define plySdfTextureReader_h
#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>
#include <vector>

#include "texReader.h"
#include "itkImage.h"

#include "renderTypes.h"

/**
* \class  plySdfTextureReader
* \brief  Basic class for reading Mesh from file
* and returning an SDFtexture from file
* operates itk::MeshFileReader<MeshType>
* where MeshType = itk::Mesh<T, 3>;
*
*
*/

template <typename T> class plySdfTextureReader : public texReader<T>
{
public:

    plySdfTextureReader() {
        lowerthreshold = T(0.5);
        m_min = T(0);
        m_max = T(0);

        m_ReadMode = RT_TEXTURE_READ_ELEMENT_TYPE;
    };
    ~plySdfTextureReader() {};

    sdfParams* GetParam() { return &param; }
    void SetThreshold(T tr) { lowerthreshold = tr; }

protected:

    virtual void PreprocessData(typename texReader::OutputImageType *fl_image);

    /*overwritten from texReader
    */
    // virtual void ReadFile();

    /*<Reads mesh file*/
    virtual void ReadImage(OutputImageType *fl_image, const char* filename);

private:
    T lowerthreshold;
    T m_min;
    T m_max;
    sdfParams param;
};
#include "plySdfTextureReader.hpp"

#endif