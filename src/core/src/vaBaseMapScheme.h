/*=========================================================================
Program:   VolumeRenderer
Module:    vaColorScheme.h

=========================================================================*/
#ifndef vaBaseMapScheme_h
#define vaBaseMapScheme_h

#include "macros.h"
#include "vaBasicObject.h"
/*
\class vaBaseMapScheme
\brief mapping scheme abstract class for auditory and color mapping
. Builds Buffer and callable program

TODO: The implementation is not finished. Created for documentation
*/

class vaBaseMapScheme :public vaBaseProgrammableObject
{
public:
    enum MappingMode {
        ids,
        xyz
    };
    vaBaseMapScheme() {
        m_mode = MappingMode::ids;
    };
    ~vaBaseMapScheme() {};

    void SetRange(optix::float2& r) { m_range = optix::make_float2(r.x, r.y); };

    virtual void Update() {};

    void SetIdType() { m_mode = MappingMode::ids; };
    void SetXYZType() { m_mode = MappingMode::xyz; };
    optix::Program GetOutput() { return m_prog; };

private:
    MappingMode m_mode;

    optix::float2 m_range;
    float m_bbox[6];

protected:
    optix::Program m_prog;
};
#endif