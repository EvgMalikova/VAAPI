#include "vaActor.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

void optixTRIActor::SetAccelerationProperties()
{
    // To speed up the acceleration structure build for triangles, skip calls to the bounding box program and
    // invoke the special splitting BVH builder for indexed triangles by setting the necessary acceleration properties.
    // Using the fast Trbvh builder which does splitting has a positive effect on the rendering performanc as well.
    if (m_builder == std::string("Trbvh") || m_builder == std::string("Sbvh"))
    {
        //TODO:
        //for triangulated surfaces only
        // This requires that the position is the first element and it must be float x, y, z.
        GetAcceleration()->setProperty("vertex_buffer_name", "attributesBuffer");
        if (sizeof(VertexAttributes) == 48)
            GetAcceleration()->setProperty("vertex_buffer_stride", "48");

        GetAcceleration()->setProperty("index_buffer_name", "indicesBuffer");
        if (sizeof(optix::uint3) == 12)
            GetAcceleration()->setProperty("index_buffer_stride", "12");
    }
}

void vaActor::RebuildAccel()
{
    // builds the BVH (or re-builds it if already existing)
    //optix::Acceleration accel = gg->getAcceleration();
    GetAcceleration()->markDirty();
}