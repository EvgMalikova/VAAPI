
# See top level CMakeLists.txt file for documentation of OPTIX_add_sample_executable.
set(PROJECT_NAME "VAAPIfrep")
#add_definitions( -DLIBRARY_NAME=${project_name} )


#configure_file( inc/version_config.h.in ${CMAKE_BINARY_DIR}/generated/version_config.h )
#include_directories( ${CMAKE_BINARY_DIR}/generated/ )

message(${PROJECT_NAME})

OPTIX_add_library( ${PROJECT_NAME} 
#core modules
#todo: make as described here https://stackoverflow.com/questions/8304190/cmake-with-include-and-source-paths-basic-setup
#look to optix_add_samples executable
#  src/texReader.h
#  src/texReader.hpp

  src/optixTextureReader.h
  src/optixTextureReader.hpp
  
  src/plySdfTextureReader.h
  src/plySdfTextureReader.hpp
  
  src/optixSDFOperations.h
  src/optixSDFOperations.cpp

 
  src/optixSDFPrimitives.h
  src/optixSDFPrimitives.cpp

)    

include_directories(
  "."
)

include_directories(../core/src)
include_directories(../inc)

target_link_libraries (${PROJECT_NAME} VAAPIcore)
	
install (TARGETS ${PROJECT_NAME}
         ARCHIVE DESTINATION ${INSTALL_DEPENDENCIES_DIR}/lib
         LIBRARY DESTINATION ${INSTALL_DEPENDENCIES_DIR}/lib
         RUNTIME DESTINATION ${INSTALL_DEPENDENCIES_DIR}/bin)
