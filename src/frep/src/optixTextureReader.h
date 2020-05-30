/*=========================================================================
Program:   VolumeRenderer
Module:    texReader.h

=========================================================================*/

#ifndef optixTextureReader_h
#define optixTextureReader_h
#include <cstring>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>

#include "texReader.h"
#include "itkImage.h"

#include "renderTypes.h"
#include <vector>

/**
* \class  sdfTextureReader
* \brief Read texture from file and applies signed distance transfrom to generate SDF texture
*
* Uses itk signed distance transform
*
*/
template <typename T> class sdfTextureReader : public texReader<T>
{
public:

    sdfTextureReader() {
        lowerthreshold = T(0.5);
        m_min = T(0);
        m_max = T(0);

        m_ReadMode = RT_TEXTURE_READ_ELEMENT_TYPE;
    };
    ~sdfTextureReader() {};

    sdfParams* GetParam() { return &param; }
    void SetThreshold(T tr) { lowerthreshold = tr; }

protected:

    virtual void PreprocessData(typename texReader::OutputImageType *fl_image);

private:
    T lowerthreshold;
    T m_min;
    T m_max;
    sdfParams param;
};
#include "optixTextureReader.hpp"

#endif