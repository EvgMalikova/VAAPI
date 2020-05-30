#include <optix_math.h>


rtDeclareVariable(int, numTexDefined, , );
rtDeclareVariable(float , TimeSound, , );
//for SDF




//array of lights
rtBuffer<float3> TFBuffer;



RT_CALLABLE_PROGRAM float3 transfer_function(int t)
{
    return TFBuffer[t];
}

