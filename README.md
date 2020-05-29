# VAAPI
An example of visualisation framework for dynamic heterogeneous and multi-scale objects.

This is a source code of the project, submitted to HPG 2020 student competition.

The project depends on Nvidia Optix 6.0 (https://developer.nvidia.com/optix), ITK (http://itk.org) and Leap Motion SDK (https://developer.leapmotion.com/). However, the last two libraries are used only in one of the 
provided examples (). So if removed, you can compile the code without those dependencies. The easiest way to compile the code is to run Cmake at superbuild folder.
However, the compilation was tested only with Visual Studio 2015. 

Most of the examples codes are collected at src\Application.cpp and are called from src\main.cpp one after another.

Please note that the presented research is a work in progress. The main target is to provide a demonstration examples of introduced approach flexibility and user friendliness. However, the source code is a quickly extracted part of a bigger project that currently in a state of active development and very draft codes does present. 



