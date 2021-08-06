Example of dynamic molecule rendering
======================================

Pipeline
--------



Scene creation 
--------------
Below is the example of how such dynamic scene can be constructed with VAAPI
~~~c
void Scene_create(vaRenderer&ren)
{
    /*
    *Reading dynamic electron density and electrostatic potential fields data: initial and final state
    */
    /*<Basic file reader of scalar field data on structural grid, those output is an SDF Texture Sampler*/
    /* Reading electron density field, in the initial state, before geometry optimisation*/
    sdfTextureReader<float> readSDFTex1; 
    readSDFTex1.SetContext(m.GetOutput());
    readSDFTex1.SetSize(139, 150, 160);
    readSDFTex1.SetThreshold(0.1);/*<defines field isosurface that will define the molecule boundary*/
    readSDFTex1.Setfile("ed1.txt");
    readSDFTex1.Update();
    /* Reading electron density field, in the final state, after geometry optimisation*/
    sdfTextureReader<float> readSDFTex2;
    readSDFTex2.SetContext(m.GetOutput());
    readSDFTex2.SetSize(138, 150, 160);
    readSDFTex2.SetThreshold(0.1);
    readSDFTex2.Setfile("ed2.txt");
    readSDFTex2.Update();

    /*<Basic file reader of scalar field data on structural grid, those output is an optix Texture Sampler*/
    /* Reading electrostatic potential field, in the initial state, before geometry optimisation*/
    texReader<float> readR1;
    readR1.SetContext(m.GetOutput());
    readR1.SetSize(139, 150, 160);
    readR1.Setfile("p1.txt");
    readR1.Update();

    /* Reading electrostatic potential field, in the final state, after geometry optimisation*/
    texReader<float> readR2; 
    readR2.SetContext(m.GetOutput());
    readR2.SetSize(138, 150, 160);
    readR2.Setfile("p2.txt");
    readR2.Update();

    /* Creating a dynamic sdf primitive on GPU, representing molecule boundary, described with electron density field
	*The primitive properties are automatically set to static/dynamic, depending on 1 or more input textures are
	*set with sdfTexture::SetTexture function
	*/
    sdfTexture tex; 
    tex.SetContext(m.GetOutput());
    tex.SetTexture(readSDFTex1.GetTexture(), readSDFTex1.GetParam());
    tex.SetTexture(readSDFTex2.GetTexture(), readSDFTex1.GetParam());
    tex.Update();

    BasicLight l1;
    l1.color = optix::make_float3(1.0);
    l1.pos = optix::make_float3(1.0);

    BasicLight l2;
    l2.color = optix::make_float3(1.0);
    l2.pos = optix::make_float3(0, 0, 1.0);

    /*A general material, that implements rendering of two scalar fields in following modes:
    * -Emission-absorption optical model for Volume Rendering with transfer function that 
    *  highlights the internal structure of both electron density and electrostaric potential fields
    *
    * -Colors geometry surface with mapped to color values of  electrostaric potential field
	* 
	*The material property is automatically set to static/dynamic, depending on 1 or more input textures are
	*set with vaEAVolume::SetTexture function
    */
   
    vaEAVolume texMaterial; 
    texMaterial.SetContext(m.GetOutput());
    texMaterial.AddLight(&l1);
    texMaterial.AddLight(&l2);
    texMaterial.SetSDFProg(tex.GetCallableProg()); /*<Gets sdf primitive optix callable program reference for static/dynamic description of SDF field*/
    texMaterial.SetType(vaEAVolume::MaterialType::VOLUME); /*<Volume rendering mode*/
    texMaterial.SetTexture(readR1.GetTexture());
    texMaterial.SetTexture(readR2.GetTexture());
    texMaterial.Update();

    /* Mapper, simillar to vtkMapper*/
    vaMapper map2;
    map2.SetContext(m.GetOutput());
    map2.SetDescInput(tex.GetOutputDesc());
    map2.AddMaterial(texMaterial.GetOutput(), texMaterial.GetType());
    map2.Update();
    
    /*Actor, simillar to vtkActor*/
    optixSdfActor acSdf;
    acSdf.SetContext(m.GetOutput()); 
    acSdf.AddMapper(&map2);
    acSdf.Update();

    /*Add actor to the rendere*/
    ren.AddActor(&acSdf);
	}
~~~

Interface implementation for interactive/dynamic rendering
----------------------------------------------------------

The above scene is generated within the following main program
~~~c
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

    
		if (iren.SetUp()) //everything is ok
    {
        unsigned int accumulation_frame = 0; //for progressive rendering
       try
        {
		Scene_Create(ren);
		}
	   /*
	   * set callback data
	   */
        Render::CallbackData cb = { (g_app.m_pinholeCamera).get(), &(g_app.ren), accumulation_frame };
        glfwSetWindowUserPointer(optixWindowProc.GetOutput(), &cb);
        /* 
		*set user defined key callback
		*/
        iren.SetKeyCallback(keyCallback);
        g_app.Init(); //scene creation
                       // Main loop
        iren.Start();
    }
		
~~~
Let us consider more in details the keyCallback procedure. In the current implementation, the vaRenderer can operate in the following modes, defined in vaBasicRenderer:

~~~c
enum vaBasicRenderer::RenderModes {
        INTERACTIVE_CAMERA, /*<Mode by default. Rendering through defined camera with interactive manipulation
                            of its parameter (pan, zoom, etc.). In this mode the optix context launches rendering
                            of optical model. In this mode only scene geometry and optic materials are rendered.
                            See vaBasicRenderer::LaunchOpticContext() function*/

        COMPUTE_SOUND, /*<Auditory ray-casting. In this mode the optix context launches rendering
                       of optical model. See vaBasicRenderer::LaunchAudioContext() function.
                       The result of procedure is the computed by ray-casting auditory properties
                       defined with scene geometry and auditory materials and configured on host OpenAL
                       auditory scene representation ready for playing  : set of sound sources,
                       sampled sound waves, configured HRTF and etc.

                       If the following scene was computed, it would be automatically played if the RenderModes::PLAY_ANIMATION
                       is activated
                       */

        PLAY_ANIMATION,  /*<Rendering of visual-auditory dynamic scene. */

        INTERACTIVE_WIDGET  /*<Interactive manipulation through assigned widget. The widgets are similar to VTK widgets and are used for
                            interactive definition of various optical/geometry/auditory parameters, that are considered once the renderer
                            */
    };
~~~

To render the dynamic scene we just need to activate the vaBasicRenderer::RenderModes::PLAY_ANIMATION mode in the assigned key callback procedure below.
 The RenderWindowInteractor will read vaRendere modes and initiate according procedures of renderer to perfor visual/auditory or dynamic scene ray-casting.
 
 Below the example of keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) procedure that gets renderer through Render::CallbackData.

~~~c

namespace Render {
    struct CallbackData
    {
        vaRenderer* ren;
      
    };
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    bool handled = false;

    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case(GLFW_KEY_S):
        {
            Render::CallbackData* cab = static_cast<Render::CallbackData*>(glfwGetWindowUserPointer(window));
            if(cab->ren->isDynamic()){
            std::cout << "Playing the dynamic scene" << std::endl;

            cab->ren->SetMode(vaBasicRenderer::RenderModes::PLAY_ANIMATION);
            cab->ren->SetTime(0.0); /*<Set initial time to 0*/
            break;
			
			
        }
    
    if (!handled) {
        // forward key event to imgui
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    }
}

~~~
Dynamic rendering is presented in video ...

The described above multisensory pip



