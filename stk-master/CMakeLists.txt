cmake_minimum_required(VERSION 3.5.1)
message(STATUS "CMake version: ${CMAKE_VERSION}")

project(stk)



include_directories(  
"./include"

)

file(GLOB cpu_source_files "./src/*.cpp")
add_library(stk STATIC ${stk_lib_SRC})	# Создание статической библиотеки 
install(TARGETS stk DESTINATION ${INSTALL_DEPENDENCIES_DIR}/lib)
