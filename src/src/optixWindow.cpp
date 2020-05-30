#include <optix.h>
#include <optixu/optixpp_namespace.h>

#include "optixWindow.h"
#include "vaRayCastBaseWidget.h"
#include "inc/predefined.h"
//---------------------
//TEST callbackFunctions
//---------------------
basicGLFW_Window::basicGLFW_Window()
{
    m_width = 0;
    m_height = 0;
    m_failed = false;
    m_WindowName = DefaultWindowName;
    //glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        m_failed = true;
    }
    std::cout << "initialized " << std::endl;
}
void basicGLFW_Window::glfwInitialize()
{
    m_window = glfwCreateWindow(m_width, m_height, m_WindowName.c_str(), NULL, NULL);

    // Note: this overrides imgui key callback with our own.  We'll chain this.
   // glfwSetKeyCallback(window, keyCallback);
    //std::cout << "kallback is set" << std::endl;
    //glfwSetWindowSize(window, (int)m_width, (int)m_height);
    //glfwSetWindowSizeCallback(window, windowSizeCallback); - defined in other way
}

void basicGLFW_Window::Update()
{
    if (!m_failed) {
        //create basic window
        glfwInitialize();
        if (!m_window)
        {
            //error_callback(2, "glfwCreateWindow() failed.");
            glfwTerminate();
            m_failed = true;
            return;
        }
        glfwMakeContextCurrent(m_window);

        if (glewInit() != GL_NO_ERROR)
        {
            //error_callback(3, "GLEW failed to initialize.");
            glfwTerminate();
            m_failed = true;
            return;
        }
    }
    SetInterface();
}

void basicGLFW_Window::SetInterface()
{
    // Setup ImGui binding.
    ImGui::CreateContext();
    ImGui_ImplGlfwGL2_Init(m_window, true);

    // This initializes the GLFW part including the font texture.
    ImGui_ImplGlfwGL2_NewFrame();
    ImGui::EndFrame();
}

basicGLFW_Window::~basicGLFW_Window()
{
    //destroy interactive part
    ImGui_ImplGlfwGL2_Shutdown();
    ImGui::DestroyContext();
}
void basicGLFW_Window::guiRender()
{
    ImGui::Render();
    ImGui_ImplGlfwGL2_RenderDrawData(ImGui::GetDrawData());
}

void basicGLFW_Window::guiNewFrame()
{
    ImGui_ImplGlfwGL2_NewFrame();
}

void basicGLFW_Window::guiReferenceManual()
{
    ImGui::ShowTestWindow();
}

//------------------
//Context manager
//---------------------
void contextManager::Update()
{
    try
    {
        GetSystemInformation();

        m_context = optix::Context::create();

        // Select the GPUs to use with this context.
        unsigned int numberOfDevices = 0;
        RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetDeviceCount(&numberOfDevices));
        std::cout << "Number of Devices = " << numberOfDevices << std::endl << std::endl;

        std::vector<int> devices;

        int devicesEncoding = m_devicesEncoding; // Preserve this information, it can be stored in the system file.
        unsigned int i = 0;
        do
        {
            int device = devicesEncoding % 10;
            devices.push_back(device); // DAR FIXME Should be a std::set to prevent duplicate device IDs in m_devicesEncoding.
            devicesEncoding /= 10;
            ++i;
        } while (i < numberOfDevices && devicesEncoding);
        std::cout << "context created" << std::endl;
        m_context->setDevices(devices.begin(), devices.end());
        std::cout << "context created" << std::endl;
        // Print out the current configuration to make sure what's currently running.
        devices = m_context->getEnabledDevices();
        for (size_t i = 0; i < devices.size(); ++i)
        {
            std::cout << "m_context is using local device " << devices[i] << ": " << m_context->getDeviceName(devices[i]) << std::endl;
        }
        // std::cout << "OpenGL interop is " << ((m_interop) ? "enabled" : "disabled") << std::endl;

        //initPrograms();

        m_Valid = true;
        // If we get here with no exception, flag the initialization as successful. Otherwise the app will exit with error message.
    }
    catch (optix::Exception& e)
    {
        std::cerr << e.getErrorString() << std::endl;
    }
}

void contextManager::GetSystemInformation()
{
    unsigned int optixVersion;
    RT_CHECK_ERROR_NO_CONTEXT(rtGetVersion(&optixVersion));

    unsigned int major = optixVersion / 1000; // Check major with old formula.
    unsigned int minor;
    unsigned int micro;
    if (3 < major) // New encoding since OptiX 4.0.0 to get two digits micro numbers?
    {
        major = optixVersion / 10000;
        minor = (optixVersion % 10000) / 100;
        micro = optixVersion % 100;
    }
    else // Old encoding with only one digit for the micro number.
    {
        minor = (optixVersion % 1000) / 10;
        micro = optixVersion % 10;
    }
    std::cout << "OptiX " << major << "." << minor << "." << micro << std::endl;

    unsigned int numberOfDevices = 0;
    RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetDeviceCount(&numberOfDevices));
    std::cout << "Number of Devices = " << numberOfDevices << std::endl << std::endl;

    for (unsigned int i = 0; i < numberOfDevices; ++i)
    {
        char name[256];
        RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_NAME, sizeof(name), name));
        std::cout << "Device " << i << ": " << name << std::endl;

        int computeCapability[2] = { 0, 0 };
        RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY, sizeof(computeCapability), &computeCapability));
        std::cout << "  Compute Support: " << computeCapability[0] << "." << computeCapability[1] << std::endl;

        RTsize totalMemory = 0;
        RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_TOTAL_MEMORY, sizeof(totalMemory), &totalMemory));
        std::cout << "  Total Memory: " << (unsigned long long) totalMemory << std::endl;

        int clockRate = 0;
        RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_CLOCK_RATE, sizeof(clockRate), &clockRate));
        std::cout << "  Clock Rate: " << clockRate << " kHz" << std::endl;

        int maxThreadsPerBlock = 0;
        RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK, sizeof(maxThreadsPerBlock), &maxThreadsPerBlock));
        std::cout << "  Max. Threads per Block: " << maxThreadsPerBlock << std::endl;

        int smCount = 0;
        RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, sizeof(smCount), &smCount));
        std::cout << "  Streaming Multiprocessor Count: " << smCount << std::endl;

        int executionTimeoutEnabled = 0;
        RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_EXECUTION_TIMEOUT_ENABLED, sizeof(executionTimeoutEnabled), &executionTimeoutEnabled));
        std::cout << "  Execution Timeout Enabled: " << executionTimeoutEnabled << std::endl;

        int maxHardwareTextureCount = 0;
        RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MAX_HARDWARE_TEXTURE_COUNT, sizeof(maxHardwareTextureCount), &maxHardwareTextureCount));
        std::cout << "  Max. Hardware Texture Count: " << maxHardwareTextureCount << std::endl;

        int tccDriver = 0;
        RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_TCC_DRIVER, sizeof(tccDriver), &tccDriver));
        std::cout << "  TCC Driver enabled: " << tccDriver << std::endl;

        int cudaDeviceOrdinal = 0;
        RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_CUDA_DEVICE_ORDINAL, sizeof(cudaDeviceOrdinal), &cudaDeviceOrdinal));
        std::cout << "  CUDA Device Ordinal: " << cudaDeviceOrdinal << std::endl << std::endl;
    }
}

/*
full syntacsis and logic
vtkSmartPointer<vtkRenderer> renderer =
vtkSmartPointer<vtkRenderer>::New();
renderer->AddActor(cylinderActor);
renderer->SetBackground(0.1, 0.2, 0.4);
// Zoom in a little by accessing the camera and invoking its "Zoom" method.
renderer->ResetCamera();
renderer->GetActiveCamera()->Zoom(1.5);

// The render window is the actual GUI window
// that appears on the computer screen
vtkSmartPointer<vtkRenderWindow> renderWindow =
vtkSmartPointer<vtkRenderWindow>::New();
renderWindow->SetSize(200, 200);
renderWindow->AddRenderer(renderer);

// The render window interactor captures mouse events
// and will perform appropriate camera or actor manipulation
// depending on the nature of the events.
vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
vtkSmartPointer<vtkRenderWindowInteractor>::New();
renderWindowInteractor->SetRenderWindow(renderWindow);

// This starts the event loop and as a side effect causes an initial render.
renderWindowInteractor->Start();
*/

void GLFW_Window::Update()
{
    //TODO: check valid var
    if ((m_ren != nullptr) && (m_context->get() != nullptr)) {
        basicGLFW_Window::Update();
    }
    else m_failed = true;
}

bool BasicRenderWindowInteractor::SetUp()
{
    if (m_window != nullptr)//&&(m_window.isValid())
    {
        m_window->Update();
        if (m_window->isValid()) { //successfully created
            //TODO other staff with loop

            GLFWwindow* window = m_window->GetOutput();

            //Basic procedure that inits everything
            m_window->GetRenderer()->SetUpRenderer();//start initialisation of all openGL and OpenAL procedures and rendering

         /*  if (m_window->createScene())
           {
               glfwTerminate();
               //m_failed = true;
               return;
           }*/

           //currently not set
            //https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback
            //glfwSetKeyCallback(window, this->keyCallback);

            return true;
        }
    }
    return false;
}

void BasicRenderWindowInteractor::SetController(Leap::Controller* control)
{
    m_controller = control;
}
void BasicRenderWindowInteractor::Start()
{
    GLFWwindow* window = m_window->GetOutput();

    //redefine this function if you want to write your own callbacks inside class
    SetDefaultCallbacks();

    m_window->GetRenderer()->Update();
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents(); // Render continuously.

        int w = m_window->GetWidth();
        int h = m_window->GetHeight();
        glfwGetFramebufferSize(window, &w, &h);

        m_window->GetRenderer()->Reshape(w, h);

        //if (hasGUI)
        {
            m_window->guiNewFrame();

            //add new procedures
            //optixWindowProc.guiReferenceManual(); // DAR HACK The ImGui "Programming Manual" as example code.

            //moved from app
            guiWindow(); // The OptiX introduction example GUI window.

            guiEventHandler(); // Currently only reacting on SPACE to toggle the GUI window.

                                      //handles auditory type
                                      //test function to test switching of materials

            /*if (ren.isDynamic()) //todo - set to be called from key
            {
                if (g_app->mol.GetMaterialType() == 0) {
                    //g_app->mol.SetMaterialType(1);
                    //g_app->mol.Update();
                    g_app->map3.SetAuditoryModel();
                }
                g_app->ren.StartPlayingAnimation(); //TODO; right now is switched off by  setDynamic false
            }
            else {
                if (g_app->mol.GetMaterialType() == 1) {
                    //g_app->mol.SetMaterialType(0);
                    g_app->map3.SetOpticalModel();
                    //g_app->mol.Update();

                    optix::Buffer soundTrace = g_app->ren.GetOutputSoundBuffer();
                    void* imageData2;

                    RT_CHECK_ERROR(rtBufferMap(soundTrace->get(), &imageData2));
                    optix::float2* ImageData = ((optix::float2*)imageData2);

                    std::cout << "DONE" << std::endl;

                    for (int i = 0; i < MAX_PRIM_ALONG_RAY; i++)
                        std::cout << ImageData[i].x << "," << ImageData[i].y << std::endl;

                    //wei
                }
            }*/

            m_window->GetRenderer()->Render();  // OptiX rendering and OpenGL texture update.
            m_window->GetRenderer()->Display(); // OpenGL display processing

            m_window->guiRender(); // Render all ImGUI elements at last.

            glfwSwapBuffers(window);
        }
    }
}

void RenderWindowInteractor::guiPlayAnimationProc()
{
    //if (m_window->GetRenderer()->isDynamic()) //todo - set to be called from key
    //{
        //m_window->GetRenderer()->GetActor(1)->PrintInfo();
    //std::cout <<"playing"<< std::endl;
   // std::cout << "playing dynamics" << std::endl;

    if (m_window->GetRenderer()->isDynamic())
    {
        float time = m_window->GetRenderer()->GetTime();
        time += tSpeed;//0.005;
        if (time < float(TimeFrames)) //currently set int manually
            m_window->GetRenderer()->SetTime(time);
        else
            m_window->GetRenderer()->SetMode(vaBasicRenderer::RenderModes::INTERACTIVE_CAMERA);
        //std::cout << "time = "<<time << std::endl;
    }
}
void RenderWindowInteractor::guiSoundRenderingProc()
{
    std::cout << "Prepare for registering " << std::endl;
    int w, h;
    m_window->GetRenderer()->GetAuditoryDims(w, h);
    if (GetWidget()->isRayCast()) {
        dynamic_cast<vaRayCastBaseWidget*>(GetWidget())->SetRayCastSize(w, h);
    }

    for (int i = 0; i < m_window->GetRenderer()->GetNumOfActors(); i++) {
        vaBasicActor* a = m_window->GetRenderer()->GetActor(i);
        //a->PrintInfo();  - works
        a->SetAuditoryModel();
    }
    std::cout << "auditory model is set" << std::endl;
    //TODO: now context should be launched
    m_window->GetRenderer()->LaunchAuditoryContext();
    //change to next node to make sure we do it only once

    //GEt sound Buffer
    m_window->GetRenderer()->UpdateAudioBuffer();  //compute auditory Rays

    m_window->GetRenderer()->RenderAudio();

    //---------------------
    std::cout << "START registering " << std::endl;
    if (GetWidget()->isRegistration()) {
        dynamic_cast<vaRayCastBaseWidget*>(GetWidget())->RegisterBuffers();
    }
    //GetWidget()

    //cleaning all buffers
   // m_window->GetRenderer()->CleanAudio();

    //activating optical model and returning back to interaction mode
    m_window->GetRenderer()->ActivateOpticalModel();

    m_window->GetRenderer()->SetMode(vaBasicRenderer::RenderModes::INTERACTIVE_CAMERA);
}

void RenderWindowInteractor::guiWindow()
{
    sutil::displayFps(frame_count++);
    if (!m_isWindowVisible) // Use SPACE to toggle the display of the GUI window.
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiSetCond_FirstUseEver);

    ImGuiWindowFlags window_flags = 0;
    if (!ImGui::Begin("Scene info", nullptr, window_flags)) // No bool flag to omit the close button.
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    ImGui::PushItemWidth(-100); // right-aligned, keep 180 pixels for the labels.

    if (ImGui::CollapsingHeader("System"))
    {
        if (ImGui::DragFloat("Mouse Ratio", &m_mouseSpeedRatio, 0.1f, 0.1f, 100.0f, "%.1f"))
        {
            m_window->m_pinholeCamera->setSpeedRatio(m_mouseSpeedRatio);
        }
    }
    ImGui::PopItemWidth();

    ImGui::End();
}
//-----------------
//--Mouse interaction function
void BasicRenderWindowInteractor::guiEventHandler()
{
    vaBasicRenderer::RenderModes m = m_window->GetRenderer()->GetMode();
    switch (m)
    {
    case vaBasicRenderer::RenderModes::INTERACTIVE_CAMERA:
        guiCameraProc();
        break;
    case vaBasicRenderer::RenderModes::INTERACTIVE_WIDGET:
        guiWidgetProc();
        break;
    case vaBasicRenderer::RenderModes::COMPUTE_SOUND:
        guiSoundRenderingProc();
        break;
    case vaBasicRenderer::RenderModes::PLAY_ANIMATION:
        guiPlayAnimationProc();
        break;
    }
}
void BasicRenderWindowInteractor::guiCameraProc()
{
    ImGuiIO const& io = ImGui::GetIO();

    const ImVec2 mousePosition = ImGui::GetMousePos(); // Mouse coordinate window client rect.
    const int x = int(mousePosition.x);
    const int y = int(mousePosition.y);

    switch (m_guiState)
    {
    case GUI_STATE_NONE:
        if (!io.WantCaptureMouse) // Only allow camera interactions to begin when not interacting with the GUI.
        {
            if (ImGui::IsMouseDown(0)) // LMB down event?
            {
                m_window->m_pinholeCamera->setBaseCoordinates(x, y);
                m_guiState = GUI_STATE_ORBIT;
            }
            else if (ImGui::IsMouseDown(1)) // RMB down event?
            {
                m_window->m_pinholeCamera->setBaseCoordinates(x, y);
                m_guiState = GUI_STATE_DOLLY;
            }
            else if (ImGui::IsMouseDown(2)) // MMB down event?
            {
                m_window->m_pinholeCamera->setBaseCoordinates(x, y);
                m_guiState = GUI_STATE_PAN;
            }
            else if (io.MouseWheel != 0.0f) // Mouse wheel zoom.
            {
                m_window->m_pinholeCamera->zoom(io.MouseWheel);
            }
        }
        break;

    case GUI_STATE_ORBIT:
        if (ImGui::IsMouseReleased(0)) // LMB released? End of orbit mode.
        {
            m_guiState = GUI_STATE_NONE;
        }
        else
        {
            m_window->m_pinholeCamera->orbit(x, y);
        }
        break;

    case GUI_STATE_DOLLY:
        if (ImGui::IsMouseReleased(1)) // RMB released? End of dolly mode.
        {
            m_guiState = GUI_STATE_NONE;
        }
        else
        {
            m_window->m_pinholeCamera->dolly(x, y);
        }
        break;

    case GUI_STATE_PAN:
        if (ImGui::IsMouseReleased(2)) // MMB released? End of pan mode.
        {
            m_guiState = GUI_STATE_NONE;
        }
        else
        {
            m_window->m_pinholeCamera->pan(x, y);
        }
        break;
    }
}

void BasicRenderWindowInteractor::guiWidgetProc()
{
    ImGuiIO const& io = ImGui::GetIO();

    const ImVec2 mousePosition = ImGui::GetMousePos(); // Mouse coordinate window client rect.
    const int x = int(mousePosition.x);
    const int y = int(mousePosition.y);
    bool useController = false;
    optix::float3 poswidj = optix::make_float3(0);
    if (m_controller != nullptr) {
        const Leap::Frame frame = m_controller->frame();
        //// std::cout << "Frame id: " << frame.id()
        //     << ", timestamp: " << frame.timestamp()
        //     << ", hands: " << frame.hands().count()
        //    << ", fingers: " << frame.fingers().count()
        //     << ", tools: " << frame.tools().count()
        //     << ", gestures: " << frame.gestures().count() << std::endl;

        Leap::Hand hand = frame.hands().rightmost();
        Leap::Vector position = hand.palmPosition();
        Leap::Vector velocity = hand.palmVelocity();
        Leap::Vector direction = hand.direction();

        Leap::PointableList pointables = hand.pointables();

        m_widget->SetHandlePosition(optix::make_float3(position.x, position.y, position.z));
        for (int i = 0; i < pointables.count(); i++) {
            Leap::Vector p = pointables[i].tipPosition();
            m_widget->SetFingerPosition(i, optix::make_float3(p.x, p.y, p.z));
        }
        // m_widget->UpdateHandlePosition();
        poswidj = optix::make_float3(position.x, position.y, position.z);
        useController = true;
        //m_window->m_pinholeCamera->pan(position.x, position.y);
    }
    //Update widget

    else {
        if (m_widget != nullptr)
        {
            //   m_widget->SetBaseCoordinates(x, y);
            switch (m_guiState)
            {
            case GUI_STATE_NONE:
                if (!io.WantCaptureMouse) // Only allow camera interactions to begin when not interacting with the GUI.
                {
                    if (ImGui::IsMouseDown(1)) // RMB down event?
                    {
                        //if (useController)
                        {
                            m_widget->SetHandlePosition(poswidj);
                            std::cout << "(" << poswidj.x << "," << poswidj.y << "," << poswidj.z << ")" << std::endl;
                        }
                        //else
                        m_widget->SetBaseCoordinates(x, y);
                        //m_guiState = GUI_STATE_ORBIT;
                    }
                    else if (ImGui::IsMouseDown(2)) // MMB down event?
                    {
                        if (useController) {
                            m_widget->SetHandlePosition(poswidj);
                        }
                        else
                            m_widget->SetBaseCoordinates(x, y);
                        // m_guiState = GUI_STATE_DOLLY;
                    }
                    else if (ImGui::IsMouseDown(0)) // LMB down event?
                    {
                        //Switch to camera mode and update it
                        m_window->m_pinholeCamera->setBaseCoordinates(x, y);
                        m_guiState = GUI_STATE_ORBIT;
                        std::cout << "Camera mode" << std::endl;
                        //m_widget->SetBaseCoordinates(x, y);
                        // m_guiState = GUI_STATE_PAN;
                    }
                    else if (io.MouseWheel != 0.0f) // Mouse wheel zoom.
                    {
                        m_widget->zoom(io.MouseWheel);
                    }
                }
                break;
            case GUI_STATE_ORBIT:
                if (ImGui::IsMouseReleased(0)) // LMB released? End of orbit mode.
                {
                    m_guiState = GUI_STATE_NONE;
                    //m_window->m_pinholeCamera->setBaseCoordinates(x, y);

                    // m_window->GetRenderer()->SetMode(vaBasicRenderer::RenderModes::INTERACTIVE_CAMERA);

                    //m_guiState = GUI_STATE_ORBIT;
                    //std::cout << "ORBIT of camera END" << std::endl;
                }
                else
                {
                    m_window->m_pinholeCamera->orbit(x, y);
                    //customly updates camera without updating widget
                    m_window->GetRenderer()->UpdateParam(); //SetMode(vaBasicRenderer::RenderModes::INTERACTIVE_WIDGET);

                    //std::cout << "ORBIT of camera" << std::endl;
                }
                break;
            }
        }

        else std::cout << "widget not defined" << std::endl;
    }
}