#ifndef RAYCASTBASEWIDGET_H
#define RAYCASTBASEWIDGET_H
#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>

#include "vaBaseWidget.h"
//---For registration
//#include "itkPointSet.h"

class vaRayCastBaseWidget : public vaBaseWidget
{
public:

    // constexpr unsigned int Dimension = 3;

    
    /*
    Redefines procedure for ray-casting widget
    */
    virtual void SetContext(optix::Context context);
    vaRayCastBaseWidget() {
        m_SoundWidth = 1;
        m_SoundHeight = 1;

        callable_program = "ray_cast";
        SetRegistration(true);
        SetRayCasting(true);
    };
    ~vaRayCastBaseWidget() {};
    /* should be done before context currently
    TODO: change logic
    */
    virtual void UpdateHandlePosition();
    void SetRayCastProgName(std::string c) { callable_program = c; }

    //PointType GetPoint(optix::float3 pp);

    virtual void CreateGeometryHandle();

    void BindRayCastBuffers();
   // void UpdateRayCastBuffers(PointsContainer* fPoints, PointsContainer* mPoints);

    std::string GetRayCastProgName() { return callable_program; }
    optix::Program GetRayCastProg() {
        return GetProgramByName(callable_program, this->m_valid);
    };
    /*
    Sets buffer size for registration
    */
    void SetRayCastSize(int width, int height);
    /*
    TODO:
    Allows to define exturnal ray-cast program
    */
    void SetRayCastProgManually(optix::Program pr) {};
    /*
    A test procedure for icp allignment
    A rigid translation registration
    */
    void RegisterBuffers();

private:
    /*callable program for geometry ray-casting*/
    std::string callable_program;
    //for auditory ray-casting

    int m_SoundWidth;
    int m_SoundHeight;
    //--------
    //Buffers for ray-casting
    optix::Buffer m_FixedBuffer;
    optix::Buffer m_MovingBuffer;

protected:
    /*
    updates widget position variable for ray-casting
    */
    void UpdateWidgetPos();
    /*
    Sets Handle program as callable procedure for ray-casting*/

    void SetHandleAsCallableProg();
};

#endif // 
