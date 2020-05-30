#pragma once
//
// Set built-in type.  Creates member Set"name"() (has taken from VTK);
//
#include <cstring>
#include <iostream>
#include <sstream>
#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#define opxSetMacro(name,type) \
virtual void Set##name (type _arg) \
{ \
  if (this->name != _arg) \
  { \
    this->name = _arg; \
    this->Modified(); \
  } \
}

//
// Get built-in type.  Creates member Get"name"() (e.g., GetVisibility());
//
#define opxGetMacro(name,type) \
virtual type Get##name () { \
  return this->name; \
}

typedef struct sdfGeo
{
    optix::Geometry geo;
    optix::Program prog;
    sdfGeo() {
        geo = nullptr;
        prog = nullptr;
    };
    void SetTime(float time)
    {
        if (prog.get() != nullptr) {
            //  float f = prog["TimeSound"]->getFloat();
            prog["TimeSound"]->setFloat(time);
            //   std::cout << "there is a dynamic scene " << f << std::endl;
        }
    }
};

#define vtkSetVectorMacro(name,type,count) \
virtual void Set##name(type data[]) \
{ \
  int i; \
   for (i=0; i<count; i++) { if ( data[i] != this->name[i] ) { break; }} \
  if ( i < count ) \
     { \
     this->Modified(); \
     for (i=0; i<count; i++) { this->name[i] = data[i]; }\
     } \
 }

#define vtkGetVectorMacro(name, type, count) \
virtual type *Get##name () \
{ \
return this->name; \
} \
virtual void Get##name(type data[count]) \
{ \
for (int i=0; i<count; i++) { data[i] = this->name[i]; }\
}

#define vtkSetOptixFloat3Macro(name) \
virtual void Set##name (float _arg1, float _arg2, float _arg3) \
{ \
  if ((this->name.x != _arg1)||(this->name.y != _arg2)||(this->name.z != _arg3)) \
   { \
     this->Modified(); \
     this->name.x = _arg1; \
    this->name.y = _arg2; \
    this->name.z = _arg3; \
     } \
}

typedef struct {
    int id;
    std::vector<int> bond_id;
} Molecule;
