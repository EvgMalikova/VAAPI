# VAAPI
An example of visualisation framework for dynamic heterogeneous and multi-scale objects.

This is a source code of the project, submitted to HPG 2020 student competition.

The project depends on Nvidia Optix 6.0 (https://developer.nvidia.com/optix), ITK (http://itk.org) and Leap Motion SDK (https://developer.leapmotion.com/). However, the last two libraries are used only in one of the 
provided examples (). So if removed, you can compile the code without those dependencies. The easiest way to compile the code is to run Cmake at superbuild folder.
However, the compilation was tested only with Visual Studio 2015. 


Please note that the presented research is a work in progress. The main target is to provide a demonstration examples of introduced approach flexibility and user friendliness. However, the source code is a quickly extracted part of a bigger project that currently in a state of active development and very draft codes does present. 

Most of the examples codes are collected at src\Application.cpp and are called from src\main.cpp one after another. Here are some examples.
Example1. Example of 0D data (points) mapping into dynamic microstructure-like shape.
![Example1](/images/Example1.png)

Example3. Example for 1D data ( geometry and connecting lines). There are just some dynamic changes of shape and topology modelled for this data.
![Example2](/images/Example2.png)

Example3. Example of multi-scale dynamic features for molecular cluster
![Example3](/images/Example3.png)


Example4. Example of 3D volume data  - tetrahedral mesh.
![Example3](/images/Example4.png)


