#pragma once
//
// Set built-in type.  Creates member Set"name"() (has taken from VTK);
//
#include <cstring>
#include <iostream>
#include <sstream>

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



#include <sutil.h>
#include "version_config.h"

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
            prog["TimeSound"]->setFloat(time);
           // std::cout << "there is a dynamic scene" << std::endl;
        }
    }
};

//TODO: add helper that will create a list of all predefined cuda files to compile to PTX within separated API
//TODO: move as a part of basic class



//Basic class to operate context
class optixObject
{
public:
	optixObject() { m_context = nullptr; };
	~optixObject() {};

	//Set
	virtual void SetContext(optix::Context &context)
	{
		m_context = context;
	}

	optix::Context GetContext() { return m_context; }
	//void SetLIBRARY_NAME(char*  name)
	//{
	//	char*  LIBRARY_NAME = name;
	//}
	//This is defined with CMAKE and should be unchangable
	const char* GetLIBRARY_NAME() {
		return getLibName();
	}

	
	static std::string ptxPath(std::string const& cuda_file)
	{
		return std::string(sutil::samplesPTXDir()) + std::string("/") +
			std::string(getLibName()) + std::string("_generated_") + cuda_file + std::string(".ptx");
	}

private:
	optix::Context m_context;
	//const char*  LIBRARY_NAME = "optixIntro_03"; //corresponds to CMAKE project name, in our case library
													 //This is inherited from Optix Advanced Samples architecture
													 // This only runs inside the OptiX Advanced Samples location,
													 // unless the environment variable OPTIX_SAMPLES_SDK_PTX_DIR is set.
													 // A standalone application which should run anywhere would place the *.ptx files 
													 // into a subdirectory next to the executable and use a relative file path here!



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

