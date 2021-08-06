/*=========================================================================
Program:   VolumeRenderer
Module:    vaSoundMapScheme.h

=========================================================================*/
#ifndef vaSoundMapScheme_h
#define vaSoundMapScheme_h
#include "macros.h"
#include "vaBaseMapScheme.h"
/*
\class vaSoundMapScheme
\brief Auditory mapping scheme. Builds Buffer and callable program

TODO: The implementation is not finished. Created for documentation
*/

class vaSoundMapScheme :public vaBaseMapScheme
{
public:

    vaSoundMapScheme() {
    };
    ~vaSoundMapScheme() {};

    virtual void Update();

    void AddFreq(float f) { m_freq.push_back(f); };

    int GetSize() {
        return m_freq.size();
    };
private:
    MappingMode m_mode;

    optix::float2 m_range;
    float m_bbox[6];

    std::vector<float> m_freq;

    optix::Program m_prog;
protected:
};
#endif
