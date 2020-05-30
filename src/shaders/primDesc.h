/*
* Stores universal primitive description

* This
*/
#ifndef PRIMDESC_H
#define PRIMDESC_H

#include <optixu/optixu_vector_types.h>
struct __device_builtin__ primParamDesc {
    int type; //type of primitive
    optix::float3 pos[4];//up to 4 coords
    //float3 dim[4]; //up to 4 dim param
    float rad[4];//radius params
    int types[4];// type info - delete
};
#endif // ATTRIBUTE_H
