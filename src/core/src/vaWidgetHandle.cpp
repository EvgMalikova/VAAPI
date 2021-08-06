#include "vaWidgetHandle.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

/*
optix::Selector m_sel selector = context_->createSelector();

std::string path_to_ptx = VRTPtxFinder::GetPtxPath("VistaRayTracing_generated_animation_selector.cu.ptx");

optix::Program selection_program = context_->createProgramFromPTXFile(path_to_ptx, "animation_selector");
selector["timestep"]->setUint(0u);
selector->setVisitProgram(selection_program);

for (auto group : groups)
{
selector->addChild(group);
}
*/
void vaWidgetHandle::SetContext(optix::Context context)
{
    vaBasicObject::SetContext(context);
    Initialize();
}

void vaWidgetHandle::Update()
{
    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        m_sel = vaBasicObject::GetContext()->createSelector();
        SetVisitorProg();
        SetParameters();
        std::cout << "ALL DONE SUCCESSFULLY" << std::endl;
    }
}

void vaWidgetHandle::SetParameters()
{
    m_sel["vis"]->setInt(0);

}
void vaWidgetHandle::Show()
{
    m_sel["vis"]->setInt(1);

}
void vaWidgetHandle::Hide()
{
    m_sel["vis"]->setInt(0);

}
void vaWidgetHandle::SetVisitorProg()
{
    std::map<std::string, optix::Program>::const_iterator it = m_mapOfPrograms.find("vis_prog");
    if (it != m_mapOfPrograms.end()) {
        m_sel->setVisitProgram(it->second);
        std::cout << "assigned ALL DONE SUCCESSFULLY" << std::endl;
    }
}
void vaWidgetHandle::Initialize() {
    
       

    if (vaBasicObject::GetContext()->get() != nullptr)
    {
        try
        {
            m_mapOfPrograms["vis_prog"] = vaBasicObject::GetContext()->createProgramFromPTXFile(vaBasicObject::ptxPath("widgets.cu"), "vis_prog");
            std::cout << "compiled and created ALL DONE SUCCESSFULLY" << std::endl;
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;
}
