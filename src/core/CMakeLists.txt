
# See top level CMakeLists.txt file for documentation of OPTIX_add_sample_executable.
set(PROJECT_NAME "VAAPIcore")
#add_definitions( -DLIBRARY_NAME=${project_name} )


#configure_file( inc/version_config.h.in ${CMAKE_BINARY_DIR}/generated/version_config.h )
#include_directories( ${CMAKE_BINARY_DIR}/generated/ )

message(${PROJECT_NAME})

OPTIX_add_library( ${PROJECT_NAME}
#core modules
#todo: make as described here https://stackoverflow.com/questions/8304190/cmake-with-include-and-source-paths-basic-setup
#look to optix_add_samples executable

  src/vaBasicObject.h
  src/vaBasicObject.cpp
  
  src/optixAbstractMaterial.h
  src/optixAbstractMaterial.cpp
  src/optixBasicActor.h
  src/optixBasicActor.cpp
  src/optixBasicRenderer.h
  src/optixBasicRenderer.cpp
  
   src/vaColorScheme.h
  src/vaColorScheme.cpp
  
  
  src/optixSDFGeometry.h
  src/optixSDFGeometry.cpp
  
  src/optixSDFBasicOperations.cpp
  src/optixSDFBasicOperations.h
  
  
  src/optixSDFBasicPrimitives.h
  src/optixSDFBasicPrimitives.cpp
  
  src/optixReader.h
  
  src/vaBasicModel.h
  
    src/glslProcedures.h
	src/glslProcedures.cpp
	
  src/vaOpticModel.h
  src/vaOpticModel.cpp
  src/vaAudioModel.h
  src/vaAudioModel.cpp

  
  src/vaWidgetHandle.h
  src/vaWidgetHandle.cpp

  src/vaActor.h
  src/vaActor.cpp

)    

include_directories(
  "."
  
include_directories(../inc/)
include_directories(../shaders/)
)


	
install (TARGETS ${PROJECT_NAME}
         ARCHIVE DESTINATION ${INSTALL_DEPENDENCIES_DIR}/lib
         LIBRARY DESTINATION ${INSTALL_DEPENDENCIES_DIR}/lib
         RUNTIME DESTINATION ${INSTALL_DEPENDENCIES_DIR}/bin)
