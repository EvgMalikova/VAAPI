/*

 */

#include "shaders/app_config.h"
#include "optixWindow.h"

#include <sutil.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

namespace Render {
    struct CallbackData
    {
        // sutil::Camera& camera;
        PinholeCamera* camera;
        RenderWindowInteractor* iren;
        vaRenderer* ren;
        unsigned int& accumulation_frame;
    };
}

static bool displayGUI = true;

static void error_callback(int error, const char* description)
{
    std::cerr << "Error: " << error << ": " << description << std::endl;
}
//------------------------------------------------------------------------------
//
//  GLFW callbacks
//
//------------------------------------------------------------------------------

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    bool handled = false;

    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE: {
            /*            if (context)
            context->destroy();
            if (window)
            glfwDestroyWindow(window);
            glfwTerminate();
            exit(EXIT_SUCCESS);
            */

            Render::CallbackData* cab = static_cast<Render::CallbackData*>(glfwGetWindowUserPointer(window));

            //cab->camera->setSpeedRatio(10.0);
            //cab->accumulation_frame++;
            cab->ren->isAuditory();
            std::cout << "ESC key pressed" << cab->ren->isAuditory() << std::endl;

            int m = int(cab->ren->GetMode());
            m++;
            if (m > 2)m = 0;
            cab->ren->SetMode(vaBasicRenderer::RenderModes(m));
            std::cout << m << std::endl;
            break;
        }
        case(GLFW_KEY_S):
        {
            Render::CallbackData* cab = static_cast<Render::CallbackData*>(glfwGetWindowUserPointer(window));
            cab->ren->isDynamic();
            std::cout << "S key pressed" << cab->ren->isDynamic() << std::endl;

            cab->ren->SetMode(vaBasicRenderer::RenderModes::PLAY_ANIMATION);
            cab->ren->SetTime(0.0);
            break;
        }
        case(GLFW_KEY_G):
        {
            Render::CallbackData* cab = static_cast<Render::CallbackData*>(glfwGetWindowUserPointer(window));

            vaBasicRenderer::RenderModes mode = cab->ren->GetMode();

            if (mode == vaBasicRenderer::RenderModes::INTERACTIVE_CAMERA)
            {
                cab->iren->GetWidget()->Show();
                cab->ren->SetMode(vaBasicRenderer::RenderModes::INTERACTIVE_WIDGET);
            }
            else {
                //cab->iren->GetWidget()->Hide();
                cab->ren->SetMode(vaBasicRenderer::RenderModes::INTERACTIVE_CAMERA);
            }

            handled = true;
            break;
        }
        case(GLFW_KEY_N):
        {
            Render::CallbackData* cab = static_cast<Render::CallbackData*>(glfwGetWindowUserPointer(window));

            vaBasicRenderer::RenderModes mode = cab->ren->GetMode();

            //cab->iren->GetWidget()->Hide();
            cab->iren->GetWidget()->CreateGeometryHandle();
            // cab->ren->SetMode(vaBasicRenderer::RenderModes::);

             //cab->iren->GetWidget()->Show();

            handled = true;
            break;
        }
        case(GLFW_KEY_F):
        {
            std::cout << "F key pressed, launching auditory context" << std::endl;
            Render::CallbackData* cab = static_cast<Render::CallbackData*>(glfwGetWindowUserPointer(window));

            cab->ren->SetMode(vaBasicRenderer::RenderModes::COMPUTE_SOUND);

            handled = true;
            break;
        }
        case(GLFW_KEY_SPACE):
        {
            std::cout << "SPACE key pressed" << std::endl;
            //Render::camera_slow_rotate = !Render::camera_slow_rotate;
            handled = true;
            break;
        }
        }
    }

    if (!handled) {
        // forward key event to imgui
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    }
}

//------------------------------------------------------------------------------
//
// GLFW setup and run
//
//------------------------------------------------------------------------------

#include "optixWindow.h"
#include "optixSDFPrimitives.h"
#include "optixSDFOperations.h"

int main_tutor(int argc, char *argv[])
{
    int  windowWidth = 512;
    int  windowHeight = 512;

    PinholeCamera pinholeCamera; //creates basic camera

    contextManager m;
    m.Update();//creates context

    vaRenderer ren;
    ren.SetValid(m.GetValid());
    ren.SetContext(m.GetOutput());

    ren.SetOpticalDims(windowWidth, windowWidth);
    ren.SetCamera(&pinholeCamera);

    //set not dynamic
    ren.SetDynamic(false);
    ren.SetAuditory(false);

    //window procedure

    GLFW_Window optixWindowProc;

    optixWindowProc.SetDim(windowWidth, windowHeight);
    optixWindowProc.SetRenderer(&ren);
    optixWindowProc.SetContext(m.GetOutput()); //returns context
    optixWindowProc.SetCamera(&pinholeCamera);

    RenderWindowInteractor iren; //TODO always check that basic still works
    iren.SetWindow(&optixWindowProc);

    std::cout << "DONE WITH WINDOW" << std::endl;

    if (iren.SetUp()) //update of WindowProc
    {
        std::cout << "START MAIN LOOP" << std::endl;

        //Scene creation
        try
        {
            optixSDFBox sdf;
            sdf.SetContext(m.GetOutput());
            sdf.SetCenter1(optix::make_float3(1.0));
            sdf.SetDims(optix::make_float3(0.3));
            sdf.Update();
            const int nums = 10;
            SDFRoundingOp round[nums];
            round[0].SetContext(m.GetOutput());
            round[0].AddOpperand(&sdf);
            round[0].SetKoeff(0.01);
            round[0].Update();

            for (int i = 1; i < nums; i++)
            {
                round[i].SetContext(m.GetOutput());

                round[i].AddOpperand(round[i - 1].GetOutputSdfObject());// &sdf);
                round[i].SetKoeff(0.01*i);
                round[i].Update();
            }

            optixSDFTorus sdfT;
            sdfT.SetContext(m.GetOutput());
            sdfT.SetCenter1(optix::make_float3(0.0));
            sdfT.SetRadius1(optix::make_float3(0.4, 0.1, 0.0));
            sdfT.Update();

            SDFElongateOp el;

            el.SetContext(m.GetOutput());
            el.AddOpperand(&sdfT);
            el.SetHKoeff(optix::make_float3(0.0, 1.0, 2.1));
            el.Update();

            SDFBlendUnionOp opBlend;
            opBlend.SetContext(m.GetOutput());
            opBlend.AddOpperand1(&sdf);
            opBlend.AddOpperand2(el.GetOutputSdfObject());
            opBlend.SetKoeff(0.3);
            opBlend.Update();

            vaBasicMaterial mSdf;
            mSdf.SetContext(m.GetOutput());
            mSdf.Update();

            vaMapper map21;
            map21.SetContext(m.GetOutput());
            map21.SetInput(sdf.GetOutput());
            map21.AddMaterial(mSdf.GetOutput(), mSdf.GetType());
            map21.Update();

            vaActor acSdf1;
            acSdf1.SetContext(m.GetOutput());
            acSdf1.AddMapper(&map21);
            acSdf1.Update();

            ren.AddActor(&acSdf1);
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
        std::cout << "DONE WITH SCENE" << std::endl;

        //Main loop
        iren.Start();
        std::cout << "MAIN LOOP STARTED" << std::endl;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int  windowWidth = 512;
    int  windowHeight = 512;
    int  devices = 3210;  // Decimal digits encode OptiX device ordinals. Default 3210 means to use all four first installed devices, when available.
    bool interop = true;  // Use OpenGL interop Pixel-Bufferobject to display the resulting image. Disable this when running on multi-GPU or TCC driver mode.
    int  stackSize = 1024;  // Command line parameter just to be able to find the smallest working size.

    GLFW_Window optixWindowProc;
    //creates scene
    SceneManager g_app;
    //g_app.SetDims(windowWidth, windowHeight,
     //   devices, stackSize, interop);

    optixWindowProc.SetDim(windowWidth, windowHeight);
    optixWindowProc.SetRenderer(&(g_app.ren));
    optixWindowProc.SetContext(g_app.GetContext());
    optixWindowProc.SetCamera(g_app.m_pinholeCamera.get());

    RenderWindowInteractor iren; //TODO always check that basic still works
    iren.SetWidget(g_app.m_widget.get());
    iren.SetWindow(&optixWindowProc);
    iren.SetTimeFrames(1); //set 1 time Frame
    if (iren.SetUp()) //does update of WindowProc
    {
        unsigned int accumulation_frame = 0;
        //all should be called after setUp
        Render::CallbackData cb = { (g_app.m_pinholeCamera).get(), &iren, &(g_app.ren), accumulation_frame };
        glfwSetWindowUserPointer(optixWindowProc.GetOutput(), &cb);
        //sets user defined callback
        iren.SetKeyCallback(keyCallback);
        g_app.Init(); //scene creation
                       // Main loop
        iren.Start();
    }

    // Cleanup
    //delete g_app;

    glfwTerminate();

    return 0;
}