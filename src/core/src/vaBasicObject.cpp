#include "vaBasicObject.h"

void vaBaseProgrammableObject::InitProg(std::string prog, std::string file, std::string name)
{
    m_mapOfPrograms[name] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath(file), prog);
}

optix::Program vaBaseProgrammableObject::GetProgramByName(std::string name, bool& valid)
{
    std::map<std::string, optix::Program>::const_iterator it;
    it = m_mapOfPrograms.find(name);
    if (it != m_mapOfPrograms.end())
    {
        valid = true;
        return it->second;
    }
    else {
        valid = false;
        return nullptr;
    }
}

void vaBaseProgrammableObject::SwapProgramByName(std::string name, optix::Program pr, bool& valid)
{
    std::map<std::string, optix::Program>::iterator it = m_mapOfPrograms.find(name);
    if (it != m_mapOfPrograms.end())
    {
        valid = true;
        it->second = pr;
    }
    else {
        valid = false;
    }
}