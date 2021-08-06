message( "External project - ALUT" )

find_package(Git)
if(NOT GIT_FOUND)
  message(ERROR "Cannot find git. git is required for Superbuild")
endif()

option( USE_GIT_PROTOCOL "If behind a firewall turn this off to use http instead." ON)

set(git_protocol "git")
if(NOT USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()


set( alut_DEPENDENCIES )
set( alut_DEPENDENCIES ${alut_DEPENDENCIES} AL)
#building alut
ExternalProject_Add(ALUT
  DEPENDS ${alut_DEPENDENCIES}
  GIT_REPOSITORY ${git_protocol}://github.com/vancegroup/freealut.git
  SOURCE_DIR alut
  BINARY_DIR alut-build
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  CMAKE_GENERATOR ${EP_CMAKE_GENERATOR}
  CMAKE_ARGS
    ${ep_common_args}
	${_vtkOptions}
    -DBUILD_STATIC:BOOL=ON
    -DBUILD_TESTING:BOOL=OFF
	-DBUILD_EXAMPLES:BOOL=OFF
    -DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_DEPENDENCIES_DIR}
  INSTALL_DIR ${INSTALL_DEPENDENCIES_DIR}
)