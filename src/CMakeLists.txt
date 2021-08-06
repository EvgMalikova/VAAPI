# 
# Copyright (c) 2013-2018, NVIDIA CORPORATION. All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 

# See top level CMakeLists.txt file for documentation of OPTIX_add_sample_executable.
set(PROJECT_NAME "optixIntro_03")
#add_definitions( -DLIBRARY_NAME=${project_name} )


configure_file( inc/version_config.h.in ${CMAKE_BINARY_DIR}/generated/version_config.h )
include_directories( ${CMAKE_BINARY_DIR}/generated/ )

message(${PROJECT_NAME})

add_subdirectory(core)
add_subdirectory(frep)

OPTIX_add_sample_executable( ${PROJECT_NAME} 
#core modules
#todo: make as described here https://stackoverflow.com/questions/8304190/cmake-with-include-and-source-paths-basic-setup
#look to optix_add_samples executable
#  src/core/optixAbstractMaterial.h
#  src/core/optixAbstractMaterial.cpp
 src/itkUtil.cpp
 src/itkUtil.h

  src/main.cpp
  
  
  src/Application.cpp
  
  src/optixWindow.h
  src/optixWindow.cpp
  
  src/optixXYZReader.h
  src/optixXYZReader.cpp
  
  src/sdfReader.h
  src/sdfReader.cpp
  
 
  src/sdfHeteroGeometry.h
  src/sdfHeteroGeometry.cpp
  
  src/optixMaterial.h
  src/optixMaterial.cpp
  
#  src/optixActor.h
#  src/optixActor.cpp
  

  
  src/vaRenderer.h
  src/vaRenderer.cpp
  
  src/vaAdvancedRenderer.h
  src/vaAdvancedRenderer.cpp
  
 # src/ExampleConnection.cpp
  #src/ExampleConnection.h
  
  inc/PinholeCamera.h
  src/PinholeCamera.cpp
  
  
	
  src/vaBaseWidget.h
  src/vaBaseWidget.cpp
  
  src/vaRayCastBaseWidget.h
  src/vaRayCastBaseWidget.cpp

  inc/Timer.h
  src/Timer.cpp
  inc/MyAssert.h
  
  #${CMAKE_BINARY_DIR}/generated/version_config.h
  #src/macros.h

  shaders/app_config.h

  shaders/renderer/rt_function.h
  shaders/renderer/rt_assert.h
  shaders/renderer/per_ray_data.h
  
  
  shaders/renderer/basic/raygeneration.cu
  shaders/renderer/basic/exception.cu
  shaders/renderer/basic/miss.cu
  
  #shaders/renderer/antialise/antialise_raygeneration.cu
  #shaders/renderer/antialise/antialise_exception.cu
  #shaders/renderer/antialise/antialise_miss.cu
  
  
  
 # shaders/vertex_attributes.h

  #shaders/boundingbox_triangle_indexed.cu
   shaders/intersection_sphere_indexed.cu
  #shaders/intersection_sphere_dynamic.cu
  shaders/intersection_sphere_bond.cu
  shaders/intersection_molecules.cu
  shaders/intersection_molecules_res.cu
  shaders/intersection_molecules4.cu
  
  
  shaders/intersection_hetero_indexed.cu
  shaders/intersection_hetero1D.cu
  shaders/intersection_hetero0D.cu
  shaders/intersection_hetero4D.cu
  
  shaders/intersection_sdfPrim.cu
  shaders/intersection_sdfPrimLib.cu
  
  shaders/materials/materials_sdf_material.cu
  shaders/materials/materials_closesthit.cu
  shaders/materials/materials_volumehit.cu
  shaders/materials/transferFunction.h
  shaders/materials/materials_tf.cu
  shaders/materials/materials_modelling.cu
  shaders/materials/materials_modelling_multiscale.cu
  
  shaders/widgets.cu


)    

include_directories(
  "."
)
include_directories(core/src)
include_directories(frep/src)
include_directories(inc/)
include_directories(shaders/)
include_directories(../LeapSDK/include)


message("CPP python")
 
#python test bindings
SET(SOURCE_DIR "src/python_cpp_example")
# Tell cmake that headers are in alse in source_dir
include_directories(${SOURCE_DIR})
SET(SOURCES "${SOURCE_DIR}/math.cpp")
# Generate a test executable
include_directories(lib/catch/include)
include_directories(core/src)
include_directories(frep/src)
include_directories(inc/)
include_directories(shaders/)
include_directories(../LeapSDK/include)
include_directories(./src)
OPTIX_add_sample_python_lib(vaapi
${SOURCES} 
${SOURCE_DIR}/bindings.cpp
 src/itkUtil.cpp
 src/itkUtil.h

  
  
  src/Application.cpp
  
  src/optixWindow.h
  src/optixWindow.cpp
  
  src/optixXYZReader.h
  src/optixXYZReader.cpp
  
  src/sdfReader.h
  src/sdfReader.cpp
  
 
  src/sdfHeteroGeometry.h
  src/sdfHeteroGeometry.cpp
  
  src/optixMaterial.h
  src/optixMaterial.cpp
  
#  src/optixActor.h
#  src/optixActor.cpp
  

  
  src/vaRenderer.h
  src/vaRenderer.cpp
  
  src/vaAdvancedRenderer.h
  src/vaAdvancedRenderer.cpp
  
 # src/ExampleConnection.cpp
  #src/ExampleConnection.h
  
  inc/PinholeCamera.h
  src/PinholeCamera.cpp
  
  
	
  src/vaBaseWidget.h
  src/vaBaseWidget.cpp
  
  src/vaRayCastBaseWidget.h
  src/vaRayCastBaseWidget.cpp

  inc/Timer.h
  src/Timer.cpp
  inc/MyAssert.h
  
  #${CMAKE_BINARY_DIR}/generated/version_config.h
  #src/macros.h

  shaders/app_config.h

  shaders/renderer/rt_function.h
  shaders/renderer/rt_assert.h
  shaders/renderer/per_ray_data.h
  
  
  shaders/renderer/basic/raygeneration.cu
  shaders/renderer/basic/exception.cu
  shaders/renderer/basic/miss.cu
  
  #shaders/renderer/antialise/antialise_raygeneration.cu
  #shaders/renderer/antialise/antialise_exception.cu
  #shaders/renderer/antialise/antialise_miss.cu
  
  
  
 # shaders/vertex_attributes.h

  #shaders/boundingbox_triangle_indexed.cu
   shaders/intersection_sphere_indexed.cu
  #shaders/intersection_sphere_dynamic.cu
  shaders/intersection_sphere_bond.cu
  shaders/intersection_molecules.cu
  shaders/intersection_molecules_res.cu
  shaders/intersection_molecules4.cu
  
  
  shaders/intersection_hetero_indexed.cu
  shaders/intersection_hetero1D.cu
  shaders/intersection_hetero0D.cu
  shaders/intersection_hetero4D.cu
  
  shaders/intersection_sdfPrim.cu
  shaders/intersection_sdfPrimLib.cu
  
  shaders/materials/materials_sdf_material.cu
  shaders/materials/materials_closesthit.cu
  shaders/materials/materials_volumehit.cu
  shaders/materials/transferFunction.h
  shaders/materials/materials_tf.cu
  shaders/materials/materials_modelling.cu
  shaders/materials/materials_modelling_multiscale.cu
  
  shaders/widgets.cu


)

  target_link_libraries( vaapi
  ${CUDA_LIBRARIES}
  ${CUDA_TOOLKIT_RPATH_FLAG}
  )


	
install (TARGETS vaapi
         ARCHIVE DESTINATION ${INSTALL_DEPENDENCIES_DIR}/lib
         LIBRARY DESTINATION ${INSTALL_DEPENDENCIES_DIR}/lib
         RUNTIME DESTINATION ${INSTALL_DEPENDENCIES_DIR}/bin)
		
# Generate python module
#link_directories("./")
#add_subdirectory(lib/pybind11)
#pybind11_add_module(python_cpp_example ${SOURCES} "${SOURCE_DIR}/bindings.cpp")

