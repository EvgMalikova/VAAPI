#include "math.hpp"

//#include "../../guiProc.h"
namespace Render {
    struct CallbackData
    {
        // sutil::Camera& camera;
        PinholeCamera* camera;
        RenderWindowInteractor* iren;
        std::shared_ptr<vaRenderer> ren;
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
        case(GLFW_KEY_A):
        {
            Render::CallbackData* cab = static_cast<Render::CallbackData*>(glfwGetWindowUserPointer(window));
            // cab->ren->isDynamic();

            //cab->ren->SetMode(vaBasicRenderer::RenderModes::PLAY_ANIMATION);
            float v = cab->ren->GetMultiscaleParam();
            v -= 0.2;
            if (v < 0.0)v = 3.0;
            cab->ren->SetMultiscaleParam(v);
            std::cout << "Interpolation between two representations " << v << std::endl;
            if (v == 1.0)
                std::cout << "Balls and Sticks representation " << v << std::endl;
            if (v == 0.0)
                std::cout << "Smothed CPK (Blobby) representation " << v << std::endl;

            break;
        }
        case(GLFW_KEY_Z):
        {
            Render::CallbackData* cab = static_cast<Render::CallbackData*>(glfwGetWindowUserPointer(window));
            // cab->ren->isDynamic();

            //cab->ren->SetMode(vaBasicRenderer::RenderModes::PLAY_ANIMATION);
            float v = cab->ren->GetMultiscaleParam();
            v -= 0.2;
            if (v < 0)v = 0.0;
            cab->ren->SetMultiscaleParam(v);
            std::cout << "Interpolation between two representations " << v << std::endl;
            if (v == 1.0)
                std::cout << "Balls and Sticks representation " << v << std::endl;
            if (v == 0.0)
                std::cout << "Smothed CPK (Blobby) representation " << v << std::endl;

            break;
        }
        case(GLFW_KEY_G):
        {
            Render::CallbackData* cab = static_cast<Render::CallbackData*>(glfwGetWindowUserPointer(window));

            vaBasicRenderer::RenderModes mode = cab->ren->GetMode();
            cab->iren->GetWidget()->Show();
            if (mode == vaBasicRenderer::RenderModes::INTERACTIVE_CAMERA)
            {
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
// Main
//
//------------------------------------------------------------------------------
int examples(int exampleNumber)
{
    int  windowWidth = 1512;
    int  windowHeight = 1512;
    int  devices = 3210;  // Decimal digits encode OptiX device ordinals. Default 3210 means to use all four first installed devices, when available.
    bool interop = true;  // Use OpenGL interop Pixel-Bufferobject to display the resulting image. Disable this when running on multi-GPU or TCC driver mode.
    int  stackSize = 1024;  // Command line parameter just to be able to find the smallest working size.

    std::shared_ptr<GLFW_Window> optixWindowProc;
    //creates scene
    std::cout << "INITED" << std::endl;

    //number of example to run

    SceneManager g_app;
    //g_app.SetDims(windowWidth, windowHeight,
    //   devices, stackSize, interop);

    //chose an example to display
    g_app.SetExample(exampleNumber);

    optixWindowProc->SetDim(windowWidth, windowHeight);
    optixWindowProc->SetRenderer(g_app.ren);
    optixWindowProc->SetContext(g_app.GetContext());
    optixWindowProc->SetCamera(g_app.m_pinholeCamera);

    RenderWindowInteractor iren; //TODO always check that basic still works
    iren.SetWidget(g_app.m_widget);
    iren.SetWindow(optixWindowProc);
    //setting number of type frames to play
    iren.SetTimeFrames(3); //3
                           //iren.SetTimeFrames(3);//5);
    iren.SetTSpeed(0.02);

    //attaching a leap motion controller
    //not used in most of the demonstration examples
    //so it is just commented if not used

    //can be as well commented in main cmake file
    /*Leap::Controller controller;
    controller.enableGesture(Leap::Gesture::TYPE_SWIPE);*/
    if (iren.SetUp()) //does update of WindowProc
    {
        unsigned int accumulation_frame = 0;
        //all should be called after setUp
        Render::CallbackData cb = { (g_app.m_pinholeCamera).get(), &iren, g_app.ren, accumulation_frame };
        glfwSetWindowUserPointer(optixWindowProc->GetOutput(), &cb);
        //sets user defined callback
        iren.SetKeyCallback(keyCallback);
        g_app.Init(); //scene creation
                      // Main loop
        iren.GetWidget()->Hide();
        //attaching controller. Works
        // iren.SetController(&controller);
        iren.Start();
    }

    // Cleanup
    //delete g_app;

    glfwTerminate();

    return 0;
}

int add(int i, int j)
{
    return i + j;
}

int subtract(int i, int j)
{
    return i - j;
}

int main_call()
{
    /*
    std::string out_file;
    //particles_file = "2bxaH.xyz";// "cyclohexane_movie.xyz";
    particles_file = "diffusion.xyz";
    //particles_file = "symcluster.xyz";

    //particles_file = "slice.xyz";
    initAll(particles_file);*/
    return 1;
}