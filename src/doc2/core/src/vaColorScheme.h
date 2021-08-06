/*=========================================================================
Program:   VolumeRenderer
Module:    vaColorScheme.h

=========================================================================*/
#ifndef vaColorScheme_h
#define vaColorScheme_h

#include "macros.h"
#include "vaBaseMapScheme.h"
/*
\class vaColorScheme
\brief Color mapping scheme. Builds Buffer and callable program

TODO: The implementation is not finished. Created for documentation
*/
class vaColorScheme :public vaBaseMapScheme
{
public:

    vaColorScheme() {
    };
    ~vaColorScheme() {};

    void AddColor(optix::float3&c) {
        m_colors.push_back(c);
    }
    virtual void Update();

    int GetSize() {
        return m_colors.size();
    };
private:
    std::vector<optix::float3> m_colors;
protected:
};
#endif
