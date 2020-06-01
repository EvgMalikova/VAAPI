#pragma once

#ifndef OPTIXWINDOW_H
#define OPTIXWINDOW_H
/*=========================================================================
Program:   GLFW_Window

=========================================================================*/
/**
* Architecture much inspired by VTK Window class(http://vtk.org) implementation
* The window handles:
*-interactive call back procedures
*-Renderer
*
*The window is written on base of GLFW
*/
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <optix.h>
#include <optixu/optixpp_namespace.h>

#include "inc/Timer.h"

#include <string>
#include <map>
#if defined(_WIN32)
#include <windows.h>
#endif

#include <imgui/imgui.h>

#define IMGUI_DEFINE_MATH_OPERATORS 1
#include <imgui/imgui_internal.h>

#include <imgui/imgui_impl_glfw_gl2.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

//Drafts for triangular meshes
#include "optixGeometry.h"
#include "optixSDFPrimitives.h"
//#include "optixSDFDynamicTexture.h"
#include "optixMaterial.h"
#include "vaActor.h"
#include "vaAdvancedRenderer.h"

#include "optixReader.h"

#include "vaBaseWidget.h"
#include "Leap.h"
class basicGLFW_Window
{
public:
    basicGLFW_Window();
    ~basicGLFW_Window();
    void SetDim(int width, int height) {
        m_width = width;
        m_height = height;
    }
    virtual void Update();
    //void SetErrorCallBack(GLFWerrorfun cbfun) { glfwSetErrorCallback(cbfun); }
    bool isValid() { return !m_failed; };
    GLFWwindow* GetOutput() {
        return m_window;
    }
    void SetWindowName(std::string name) { m_WindowName = name; };

    //gui procedures
    void guiRender();
    void guiNewFrame();
    void guiReferenceManual();

private:

    GLFWwindow* m_window;
    std::string m_WindowName;

protected:
    bool m_failed;
    int m_width;
    int m_height;
    void glfwInitialize();
    //GUI initialization procedure
    void SetInterface();
};

#define RT_CHECK_ERROR_NO_CONTEXT( func ) \
  do { \
    RTresult code = func; \
    if (code != RT_SUCCESS) \
      std::cerr << "ERROR: Function " << #func << std::endl; \
  } while (0)

class contextManager
{
public:
    contextManager() {
        m_Valid = false; m_devicesEncoding = 3210;
    };
    ~contextManager() {};

    optix::Context GetOutput() { return m_context; }
    void Update();
    bool GetValid() { return m_Valid; }
    void SetDeviceEncoding(unsigned int devicesEncoding) { m_devicesEncoding = devicesEncoding; }
private:
    unsigned int m_devicesEncoding;
    // OptiX variables:
    optix::Context m_context;

    bool m_Valid;

protected:

    void GetSystemInformation();
};

enum GuiState
{
    GUI_STATE_NONE,
    GUI_STATE_ORBIT,
    GUI_STATE_PAN,
    GUI_STATE_DOLLY,
    GUI_STATE_FOCUS
};

//----------------------
//Example class with custom procedures
//-----------------------
class GLFW_Window :public basicGLFW_Window
{
public:
    GLFW_Window() {
        //basicGLFW_Window::basicGLFW_Window();  - already called by default
        m_ren = nullptr;
        m_context = nullptr;
    };
    ~GLFW_Window() {
        //basicGLFW_Window::~basicGLFW_Window();
    };

    //-----------
    //should be set as called procedure
   // bool createScene() ;
    //---------

    virtual void Update();
    void SetRenderer(vaBasicRenderer* ren) { m_ren = ren; }
    // std::shared_ptr<optixBasicRenderer>(ren);};
    void SetContext(optix::Context cont) { m_context = cont; }
    //TODO: badly made
    void SetCamera(PinholeCamera* cam) { m_pinholeCamera = cam; };

    //PinholeCamera* GetCamera() { return m_pinholeCamera.get(); };

    PinholeCamera* m_pinholeCamera;
    //vaBaseWidget* m_widget;

    vaBasicRenderer* GetRenderer() { return m_ren; };
    int GetWidth() { return m_width; };
    int GetHeight() { return m_height; };
private:
    //std::shared_ptr<optixBasicRenderer> m_ren;
    vaBasicRenderer*m_ren;
    optix::Context m_context;
};

//Basic interactor
//without any windows or interactivity
//only camera
class BasicRenderWindowInteractor
{
public:
    BasicRenderWindowInteractor() {
        m_window = nullptr;
        m_guiState = GUI_STATE_NONE;
        m_widget = nullptr;
        m_controller = nullptr;
    };
    ~BasicRenderWindowInteractor() { glfwTerminate(); };
    void SetWindow(GLFW_Window*window) { m_window = window; };
    virtual bool SetUp();
    virtual void Start();
    void SetController(Leap::Controller* control);

    void SetKeyCallback(void(*f)(GLFWwindow*, int, int, int, int)) {
        GLFWwindow* window = m_window->GetOutput();
        glfwSetKeyCallback(window, (*f));
    }
    void SetWidget(vaBaseWidget* v) { m_widget = v; };
    vaBaseWidget* GetWidget() { return m_widget; };
private:

    bool m_isWindowVisible;
    Leap::Controller* m_controller;
    GuiState m_guiState;
    vaBaseWidget* m_widget;
protected:

    GLFW_Window*m_window;

    virtual void guiWindow() {}; //draws all necessary windows with imGUI

    virtual void guiEventHandler(); //call all proc from renderer
    virtual void guiCameraProc();
    virtual void guiSoundRenderingProc() {};
    virtual void guiPlayAnimationProc() {};
    virtual void guiWidgetProc();
    virtual void SetDefaultCallbacks() {};
};

//-----------
//Example of interactior class
// operates ref to Renderer, Actor, Mappers that should be available
class RenderWindowInteractor :public BasicRenderWindowInteractor
{
public:
    RenderWindowInteractor() {
        m_isWindowVisible = false; // Hide the GUI window completely with SPACE key.
        m_mouseSpeedRatio = 10.0f;
        TimeFrames = 1;
        tSpeed = 0.005;
        frame_count = 0;
    };
    ~RenderWindowInteractor() {  };

    void SetTimeFrames(int t) { TimeFrames = t; };
    void SetTSpeed(float t) { tSpeed = t; };
protected:
    virtual void guiWindow(); //draws all necessary windows with imGUI
    //virtual void guiCameraProc(); here we use default camera proc
    virtual void guiSoundRenderingProc(); //currently not defined
    virtual void guiPlayAnimationProc(); //currently not defined
    //may be should be moved here
    //virtual void guiWidgetProc();
    virtual void SetDefaultCallbacks() {};

private:
    bool m_isWindowVisible; // hides/shows gui windows
    float m_mouseSpeedRatio;
    int TimeFrames;
    float tSpeed;
    int frame_count;
};

#include "vaWidgetHandle.h"
class SceneManager
{
public:
    std::vector<vaMapper *> mappers;
    std::vector<vaActor *> actorSdf;
    std::vector<vaTRIActor* > actorTri;
    contextManager m;
    vaWidgetHandle w;

    //scenes creation

    void createDynamicHeterogeneousObjectScene();
    void createFrepScene();
    void createMolSolventScene();
    void createMicrostructureScene();
    //void createCrystalSceneMol2();
    void createMicrostuctureMOlScene();
    /* Performance comparison example
    sdfHetoreneous vs sdfHeterogeneous0D
    Note: Material optimisation was not done*/
    void Example0();
    void Example1();
    /*Fully optimised simple molecular data volume rendering*/
    void Example2();
    void Example31();
    void Example3();
    void ExampleTetra();

    void createAuditoryMoleculeSceneMolDynam();
    void createAuditoryMoleculeScene2();

    optix::Context GetContext() { return m.GetOutput(); };

    void Init();
    SceneManager();

    SceneManager(const int width,
        const int height,
        const unsigned int devices,
        const unsigned int stackSize,
        const bool interop);
    ~SceneManager();

    bool isValid();
    vaRenderer* ren;
    //to share between applications and later make
    std::shared_ptr<PinholeCamera> m_pinholeCamera;
    std::shared_ptr<vaBaseWidget> m_widget;
    void SetExample(int m) { m_example = m; };
private:
    int m_example;
    void createScene();

    Timer m_timer;
};
#endif // 