Visual-auditory API {#mainpage}
=================

Introduction
============

Scalar fields are used in many research areas, where computer simulations or experimental 
studies are involved, such as computational chemistry, medical data analysis and physical phenomena studies. 
Visualisation of scalar fields usually employs Volume Rendering techniques as for the computer systems the
 scalar fields data is often converted to data volumes stored in the texture memory.  The visual analysis 
 of scalar fields is not always straightforward, especially when a complex dynamic phenomenon is represented. 
 In Volume Rendering the Multidimensional Transfer functions (TF) and consideration of more complex 
 object-light interaction processes in the optical model is used to address the issues 
 of visual analysis quality improvement and highlight features of interest.  

However, the visual perception limitations not always can be overcome solely with the enhancement of the optical model.
 The visual system can be overloaded and perturbed due to fatigue. An additional introduction of auditory sensory stimuli 
 to address a problem of visual analysis limitations is a well-known technique, called [sonification] (https://sonification.de/handbook/).  
 
This API targets the problems of the visual-auditory interactive study of the dynamic scalar fields using the concept of a heterogeneous object influencing various sensory stimuli. 
The API takes advantages of the similarities between light and sound propagation to suggest a novel procedure of a visual-auditory rendering based on the ray-casting procedure. It makes it possible to conduct a simultaneous visual analysis along with spatial positioning/measuring. 
The API suggests rendering and modelling of both surface and solid geometry with visual and auditory properties. However mainly it is concentrated 
on the Volume scene representation based on the HyperVolume (HV) model and take advantage of the Signed Distance Fields (SDF) for rendering.   
The concept introduces three separate parts of the HV model for Volume Rendering that can be evaluated independently on demand to speed up the rendering process of complex dynamic volume objects 
and a general visual-auditory scene representation for interactive Volume Rendering.


For more details, on heterogeneous objects modelling and HyperVolume approach look at [Approach Overview] (doc/approach.md) Section:

Optix engine and VTK
=====================

The API is based on Optix Engine and takes advantage of it's native rendering acceleration structures like BVH. The API syntacsis is mainly a "copy" of [Visualization Toolkit] (http://vtk.org). As well it much inherits the standard visualisation concepts
reflected in VTK API, like visualisation pipeline, replacing it with notion of visual-auditory pipeline, unified on base of ray-casting procedure. Thus, API targets to provide high level access to geometric modelling and rendering,
allowing the researcher or programmer to operate familiar visualisation concepts and terminology. At the same time it is very lightweight and fast as all core procedures are CUDA kernals running on NVIDIA GPU card. 
The API it mostly oriented to support geometry modelling on base of [FRep concept] (http://hyperfun.org) and targets various aspects of multisensory interaction, like collision detection, interactive manipulations and etc.

For concept of basic use, similarities and differencec to VTK see  [API basics](doc/vtkstyle.md)
For FRep modelling example look [FRep tutorial](doc/freptutorial.md)

Examples
===========
There are several basic tutorials for users and developers in [Tutorials ] (doc/alltutorials.md)

Some featured examples:
 - For examples of Visual-auditory volume and SDF geometry ray-casting see [Examples] (doc/examples.md)

Naming structure
================

Most of API classes have preffix "va" (for example vaBasicRenderer). The data Readers names are formed with prefix of file format. For example xyzReader. All primitives and operations of FRep geometric modelling and Readers that return SDF representation, in other words can be directly used within FRep modelling pipeline without any mapping, have "sdf" prefix.
For example sdfSphere, sdfBox, sdfTextureReader and etc.

Presequencies
=============

The API is based on [Optix engine] (https://developer.nvidia.com/optix). The visual and auditory rendering is performed through use of [OpenGl] (https://www.opengl.org/) 
and [OpenAL] (https://www.openal.org/downloads/) accordingly. For details on visual-auditory rendering interopability see implementation of opticalModel class: for optical model and  auditoryModel class: for 3D sound rendering.
Additionally, some of the API  classes use the following open source libraries (those procedures might be not used and libraries excluded from the project):

 - Readers: [ITK](http://itk.org) for data preprocessing 

 - stkSound class: :[STK] (https://ccrma.stanford.edu/software/stk/) for sound synthesis

TODO
===========
- Haptic implementation
  - Collision detection
- Practical examples
  - Molecular haptics
-Visual-auditory tutorials
-Extending API (advanced tutorials)
