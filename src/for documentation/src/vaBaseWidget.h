#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>

#include "optixBasicActor.h"
#include "optixSDFPrimitives.h"

class vaBaseWidget : public vaBaseProgrammableObject
{
public:

    vaBaseWidget();
    ~vaBaseWidget();
    /* should be done before context currently
    TODO: change logic
    */

    //-------
    virtual void SetContext(optix::Context &context);

    bool updateCam;

    void setViewport(int w, int h);
    void setBaseCoordinates(int x, int y);
    void setSpeedRatio(float f);
    void setFocusDistance(float f);

    void orbit(int x, int y);
    void pan(int x, int y);
    void dolly(int x, int y);
    void focus(int x, int y);
    void zoom(float x);

    bool  getFrustum(optix::float3& pos, optix::float3& u, optix::float3& v, optix::float3& w);
    float getAspectRatio() const;

    optixBasicActor *GetActor() { return acSdf1; };
    virtual void UpdateHandlePosition();
    virtual void CreateGeometryHandle();
    //void CreateGeometryHandle2();
    void SetCamParameters(optix::float3& pos, optix::float3& u, optix::float3& v, optix::float3& w, optix::float3& center, float&dist);
    void setDims(int w, int h) { m_width = w; m_height = h; };
public: // Just to be able to load and save them easily.
    optix::float3 m_center;   // Center of interest point, around which is orbited (and the sharp plane of a depth of field camera).
    float         m_distance; // Distance of the camera from the center of intest.
    float         m_phi;      // Range [0.0f, 1.0f] from positive x-axis 360 degrees around the latitudes.
    float         m_theta;    // Range [0.0f, 1.0f] from negative to positive y-axis.
    float         m_fov;      // In degrees. Default is 60.0f

    void  UpdateGeo()
    {
        m_geo = sdfT->GetOutput();
    };
    vaMapper* map21;
    optixBasicActor* acSdf1;
    optix::Program m_prog;
    optix::Geometry m_geo;

    optix::float3 sysCameraPosition;
    optix::float3 sysCameraU;
    optix::float3 sysCameraV;
    optix::float3 sysCameraW;

    optix::float3 sysCameraCenter;
    float sysCameraDist;

    /*
    Hides widget by setting varRadius parameter to zero
    Handled via sdf primitives. Not optix visibility groups
    */
    void Hide()
    {
        if (m_geo.get() != nullptr)
            m_geo["varRadius"]->setFloat(0, 0, 0);
    };

    /*
    Shows widget. Handled through sdf primitives rendering
    */
    void Show()
    {
        if (m_geo.get() != nullptr)
            m_geo["varRadius"]->setFloat(m_BBVisibility, m_BBVisibility, m_BBVisibility); //for bounding box
        sdfT->SetRadius1(optix::make_float3(m_Radius, m_Radius, m_Radius));
    };

    void SetRadius(float r)
    {
        m_Radius = r;
        //sdfT->SetRadius1(optix::make_float3(m_Radius, m_Radius, m_Radius));
    };

    /*
    Returns sdf primitive object. This can be used as a part of
    geometrical modellig pipeline
    */
    optixSDFPrimitive* GetHandle() { //TODO: make optixSDFPrimitive
        return sdfT;
    };
    /*Returns a program of handle. This is an SDF geometry generation program.
    The program can be used inside auditory-raycasting procedure, that is implemented in widget
    */

    optix::Program GetHandleProg() {
        //optixSDFPrimitive
        return sdfT->GetCallableProg();
    };

    bool isRayCast() { return m_rayCasting; };
    void SetRayCasting(bool rayC)
    {
        m_rayCasting = rayC;
    };

    bool isRegistration() { return m_registration; };
    void SetRegistration(bool rayC)
    {
        m_registration = rayC;
    };
private:

    float m_Radius;
    bool setDelta(int x, int y);
    optixSDFPrimitive* sdfT;

    int t; //number of primitive

    int   m_width;    // Viewport width.
    int   m_height;   // Viewport height.

    float m_aspect;   // m_width / m_height
    int   m_baseX;
    int   m_baseY;
    float m_speedRatio;

    // Derived values:
    int           m_dx;
    int           m_dy;
    bool          m_changed;
    optix::float3 m_cameraPosition;
    optix::float3 m_cameraU;
    optix::float3 m_cameraV;
    optix::float3 m_cameraW;

    float m_BBVisibility;
    /* For identifying if widget is a raycasting one*/
    bool m_rayCasting;
    /*For identifying if there is post-registration procedure
    performed on base of the ray-casting results
    */
    bool m_registration;
};

#endif // 