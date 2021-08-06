message( "External project - OPENAL" )

find_package(Git)
if(NOT GIT_FOUND)
  message(ERROR "Cannot find git. git is required for Superbuild")
endif()

option( USE_GIT_PROTOCOL "If behind a firewall turn this off to use http instead." ON)

set(git_protocol "git")
if(NOT USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()


#building openal_soft
ExternalProject_Add(AL
GIT_REPOSITORY ${git_protocol}://github.com/kcat/openal-soft.git
  SOURCE_DIR OpenAL-Soft
  BINARY_DIR OpenAL-Soft-build
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  CMAKE_GENERATOR ${EP_CMAKE_GENERATOR}
   CMAKE_ARGS
    #${ep_common_args}
	#${_vtkOptions}
    -DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_DEPENDENCIES_DIR}
	-DBUILD_ALSOFT_NO_QT5:BOOL=TRUE
  INSTALL_DIR ${INSTALL_DEPENDENCIES_DIR}
)