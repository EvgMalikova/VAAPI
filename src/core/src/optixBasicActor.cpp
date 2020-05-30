#include "optixBasicActor.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>


void vaBasicActor::Update()
{
    //set acceleration properties for geometry
    acceleration = vaBasicObject::GetContext()->createAcceleration(m_builder);

    SetAccelerationProperties();
    //gg->setAcceleration(acceleration);
    gg = vaBasicObject::GetContext()->createGeometryGroup();
    gg->setAcceleration(GetAcceleration());
    gg->setChildCount(geom.size());
    for (int i = 0; i < geom.size(); i++) {
        gg->setChild(i, geom[i]);
      //  m_mapper[i]->PrintInfo();
    }

    RebuildAccel();
    

}

