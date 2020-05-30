#/**
* \class vaWidgetHandle
* \brief A base class for handles of the widgets
*
*
*------------------------*/
#ifndef vaWidgetHandle_h
#define vaWidgetHandle_h

#include <cstring>
#include <map>

#include "macros.h"
#include "vaBasicObject.h"
class vaWidgetHandle : public vaBasicObject
{
public:
    vaWidgetHandle() {
        m_sel = nullptr;
    };
    ~vaWidgetHandle() {};

    virtual optix::Selector GetOutput() { return m_sel; };

    virtual void SetContext(optix::Context &context);
    virtual void Update();
    void Show();
    void Hide();

private:
    optix::Selector  m_sel;
    std::map<std::string, optix::Program> m_mapOfPrograms;

protected:
    virtual void Modified() {};
    void Initialize();
    virtual void SetParameters();
    virtual void SetVisitorProg();
};
#endif