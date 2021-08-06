#ifndef vaBaseObject_h
#define vaBaseObject_h

#include <sutil.h>
#include "version_config.h"
#include <map>
#include <string>

#include "macros.h"

/*
* \class vaBasicObject
* \brief A base helper class that gets context

*
* This is a basic class for all objects operating the optix context
*/

class vaBasicObject
{
public:
    vaBasicObject() { m_context = nullptr; };
    ~vaBasicObject() {};

    //Set
    virtual void SetContext(optix::Context context)
    {
        m_context = context;
    }

    bool CheckContext() {
        if (m_context.get() != nullptr) return true;
        else return false;
    }

    optix::Context GetContext() { return m_context; }

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

/*
* \class vaBaseProgrammableObject
* \brief A base helper class that gets context

* A helper that will create a list of all predefined cuda files to compile to PTX within separated API.
* The class stores those programs and allows to address them by a name.
* TODO: Provides other useful info and debug functions
*
* This is a basic class for all objects operating the optix context and building programmable files
*/

class vaBaseProgrammableObject : public vaBasicObject
{
public:
    vaBaseProgrammableObject() { m_valid = true; };
    ~vaBaseProgrammableObject() {};

    optix::Program GetProgramByName(std::string name, bool& valid);
private:
    //should be in some unified place
    std::map<std::string, optix::Program> m_mapOfPrograms;

protected:
    void InitProg(std::string prog/*<name of program in cuda file*/, std::string file/*<name of cuda file*/, std::string name /*<assigned name of program for further reference*/);
    void SwapProgramByName(std::string name, optix::Program pr, bool& valid);

    bool m_valid;
};
#endif
