#include "shaders/app_config.h"
#include "optixWindow.h"
#include "Leap.h"
#include <sutil.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

//------------------------------------------------------------------------------
//
// GLFW setup and run
//
//------------------------------------------------------------------------------

#include "optixWindow.h"
#include "optixSDFPrimitives.h"
#include "optixSDFOperations.h"
#include "sdfReader.h"
#include "optixXYZReader.h"
#include "vaRayCastBaseWidget.h"

int examples(int exampleNumber);

/*! Add two integers
    \param i an integer
    \param j another integer
*/
int add(int i, int j);
/*! Subtract one integer from another
    \param i an integer
    \param j an integer to subtract from \p i
*/
int subtract(int i, int j);

//main function with interface
int main_call();