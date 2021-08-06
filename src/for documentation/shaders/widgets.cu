#include <optix.h>

rtDeclareVariable(int, vis, , );

RT_PROGRAM void vis_prog()
{
    rtIntersectChild(0);
}