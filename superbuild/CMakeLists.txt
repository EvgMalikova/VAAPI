cmake_minimum_required( VERSION 2.8.10 )

project( jane-Superbuild )

# set CMAKE_MODULE_PATH for cmake macro/function and modules
set( CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}/../cmake
)

if( APPLE )
  include( CMakeOSXVariables )
endif()

if( CMAKE_HOST_WIN32 )

  string( LENGTH "${CMAKE_CURRENT_SOURCE_DIR}" n )
  if( n GREATER 50 )
    message( FATAL_ERROR
      "jane source code directory path length is too long (${n} > 50)."
      "Please move the jane source code directory to a directory with a shorter path."
    )
  endif()

  string( LENGTH "${CMAKE_CURRENT_BINARY_DIR}" n )
  if( n GREATER 50 )
    message( FATAL_ERROR
      "jane build directory path length is too long (${n} > 50)."
      "Please move the jane build directory to a directory with a shorter path."
    )
  endif()

endif()

option( BUILD_SHARED_LIBS "Build shared libraries" ON )

include( ExternalProject )

set( BUILD_TYPE ${CMAKE_BUILD_TYPE} )
if( NOT INSTALL_DEPENDENCIES_DIR )
    set( INSTALL_DEPENDENCIES_DIR ${CMAKE_BINARY_DIR}/INSTALL CACHE STRING "Install directory for dependencies")
endif()
set( Patches_DIR ${CMAKE_CURRENT_SOURCE_DIR}/patches )
set( jane_DEPENDENCIES )


option( USE_SYSTEM_AL "Use system libraries for OPENAL" OFF )
if( ${USE_SYSTEM_AL} MATCHES "OFF" )
  include( External-AL.cmake )
  set( jane_DEPENDENCIES ${jane_DEPENDENCIES} AL )
else()
  find_package( AL REQUIRED )
  #include( ${ITK_USE_FILE} )
endif()

option( USE_SYSTEM_ALUT "Use system libraries for ALUT" OFF )
if( ${USE_SYSTEM_ALUT} MATCHES "OFF" )
  include( External-ALUT.cmake )
  set( jane_DEPENDENCIES ${jane_DEPENDENCIES} ALUT )
else()
  find_package( ALUT REQUIRED )
  include( ${AL_USE_FILE} )
endif()


include( ExternalProject )
#set project configuration files


 
#set project configuration files
ExternalProject_Add(jane
  DEPENDS ${jane_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..
  BINARY_DIR jane-build
  CMAKE_GENERATOR ${EP_CMAKE_GENERATOR}
  CMAKE_ARGS
    ${ep_common_args}
    #${cmake_hdf5_libs}
    -DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}
    -DINSTALL_DEPENDENCIES_DIR:PATH=${INSTALL_DEPENDENCIES_DIR}
     INSTALL_DIR ${INSTALL_DEPENDENCIES_DIR}
)
