Main logic and similarities with VTK API
========================================

The basics of API use and main concept that is much simillar to [Visualisation Toolkit ](http://vtk.org). VTK is defacto a standard for general proglems of visual analysis solving.
It's integration with VTK-m provides other dimension to the API efficient computation and integration with simulation software. 

The proposed Visual-auditory API targets multisensory interaction problem, unified on base of ray-casting. It is first of all is a research project that targets to explore a unified approach 
to multisensory feedback on base of ray-casting procedure. Similarity to VTK syntacsis targets to provide user and potential further developers easy entry access to API and of course the author is quite familiar with VTK and that's why providing 
similar sintacsis in API seemed to him quite logical. The main target of the proposed API: provide a base for further Multisensory interfaces development on base of Ray-casting procedure.

The core of the VTK is a visualisation pipeline of data, that starts from data source and ends up with image rendered on screen. 

Below are basic similarities and differences with VTK in the main concept
The basic classes of VTK API are:
- Sources

- Filters

- Mappers

- Actors

- Renderers

- Interactors

- Windows

The proposed solution targets visual-auditory ray-casting of scalar fields. We believe that for this type of data, there is no need to reimplement a large "filtering" part in visualisation pipeline of VTK. 
The most of scalar fields data preprecessing tasks are already addressed with Image Processing tookit [ITK] (http::itk.org). As a result the API mainly provides only targets mapping and rendering tasts and 
suggests some data reading and preprocessing procedures can be implemented on base of ITK or from scratch. 
 
The Visual-auditory API implements basic Mappers, Actors, Renderers and Windows that are intended to support ray-casting of mainly Signed Distance Fields, althought triangulated geometry is also supported and can be integrated within pipeline.
Similarly to VTK the pipeline is constructed through "input-output" concept.  The scalar field data is provided by file Readers that eighter output structured data or directly create Optix Buffers and TextureSamplers on GPU. The inputs and outputs are 
usually Optix objects, like optix::Geometry, optix::Material, optix::GeometryInstance and etc.. As all this objects are created through optix::Context, the reference to it is given to each class. For context creation a ContextManager
 class is responsible. 

Data readers and SDF geometry creation
--------------------------------------
In the example below, the Reader that reads molecular data described in [XYZ file format] (http://wiki.jmol.org/index.php/File_formats/Formats/XYZ) and transfers it to "ball" like representation, 
when atoms represented as spheres (balls). This is current version of [CPK molecule representation](https://en.wikipedia.org/wiki/Space-filling_model). For info on [molecule representation types] (https://en.wikipedia.org/wiki/Jmol).

~~~c
	contextManager m; 
	m.Update();//creates context
	
	xyzReader read;  //reads data
	read.Setfile("molecule.xyz");
	read.Update();

	sdfCPKMol mol;  //
	mol.SetContext(m.GetOutput());
	mol.SetCenter(read.GetOutput1()); //centers, data stored in std::vector
	mol.SetTypes(read.GetOutput2()); //set types of atoms, data stored in std::vector
	mol.SetMaterialType(0); 
	mol.Update();
~~~

The other example is reading dynamic scalar data, that represents . Gamess files.
The reader performs signed distance transform of data on base of [Image Processing Toolkit] (http://itk.org). It generates optix Buffers and Textures Samplers. The optixSDFTexture gets Texture Samplers describing initial and final states of molecule dynamic electron density field 
and forms dynamic SDF geometry object. 
~~~c
	contextManager m;
	m.Update();//creates context

	sdfTextureReader<float> readSDFTex1;
	readSDFTex1.SetContext(m.GetOutput());
	readSDFTex1.SetSize(139, 150, 160); //data dimensions

	readSDFTex1.SetThreshold(0.1); //isolevel of scalar field
	readSDFTex1.Setfile("ed1.txt");
	readSDFTex1.Update();

	sdfTextureReader<float> readSDFTex2;
	readSDFTex2.SetContext(m.GetOutput());
	readSDFTex2.SetSize(138, 150, 160);

	readSDFTex2.SetThreshold(0.1);
	readSDFTex2.Setfile("ed2.txt");
	readSDFTex2.Update();

	sdfTexture tex; //dynamic SDF object creation
	tex.SetContext(m.GetOutput());
	tex.SetTexture(readSDFTex1.GetTexture(), readSDFTex1.GetParam());
	tex.SetTexture(readSDFTex2.GetTexture(), readSDFTex1.GetParam());
	tex.Update();
~~~

Mapper, Actor, Renderer
-----------------------

Mapper, Actor and Renderer classes are much simillar to VTK in concept and syntacsis. The Mapper applies a material to the objects (in current implementation with optical or auditory properties). 
The Mapper is can activate optical or auditory material, depending on type of rendering procedure: rendering optical or auditory properties. For this, in current implementation the Mapper receives
 optix::Material from Material class (material.GetOutput() procedure ) and description of it's properties in , material.GetType() procedure. 
 
The Renderer is responsible for rendering visual and auditory properties. Whether optical or auditory properties should be rendered, the Renderer operates its actors to activate the necessary visual or auditory model for all created pipelines of data.

Below the code demonstrating the above concepts and syntacsis of Mapper, Actor and Renderer classes basic usage.
~~~c
	vaMaterial mSdf;
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
	
	
	....
	ren.LaunchAuditoryContext(); //compute auditory ray-tracing
	
	...
	ren.LaunchOpticContext();// compute optical ray-tracing
~~~	

GUI  and interaction
--------------------

The GUI procedures are implemented on base of [GLFW] (https://github.com/glfw/glfw) and [ImGUI](https://github.com/ocornut/imgui) libraries. 
A custom GUI can be easily implemented on base of those libraries and integrated to API pipeline. 

The API provides a basic Renderer, GLFW window and Interactor. As far the basic class of PinholeCamera from NVIDIA OPTIX ADVANCED examples was used to render data:
~~~c
  contextManager m;
  m.Update();//creates context

  PinholeCamera pinholeCamera;  //basic camera
  
  optixRenderer ren;
  ren.SetValid(m.GetValid());
  ren.SetContext(m.GetOutput());


  ren.SetOpticalDims(windowWidth, windowWidth);
  ren.SetCamera(&pinholeCamera);



  //set not dynamic
  ren.SetDynamic(false);
  ren.SetAuditory(false);



 
  //window procedure

  vaWindow optixWindowProc; 
  

  optixWindowProc.SetDim(windowWidth, windowHeight);
  optixWindowProc.SetRenderer(&ren);
  optixWindowProc.SetContext(m.GetOutput()); //returns context
  optixWindowProc.SetCamera(&pinholeCamera);

  vaRenderWindowInteractor iren; //TODO always check that basic still works
  iren.SetWindow(&optixWindowProc);
~~~


