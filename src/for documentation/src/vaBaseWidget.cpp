/*

 */

#include "vaBaseWidget.h"

#include "optixMaterial.h"
#include "optixSDFOperations.h"
#include "vaActor.h"
#include <iostream>

 //---

vaBaseWidget::vaBaseWidget()
    : m_distance(10.0f) // Some camera defaults for the demo scene.
    , m_phi(0.75f)
    , m_theta(0.6f)
    , m_fov(60.0f)
    //:  m_distance(3.0f)
    //, m_phi(0.75f)  // positive z-axis
    //, m_theta(0.5f) // equator
    //, m_fov(60.0f)
    , m_width(1)
    , m_height(1)
    , m_aspect(1.0f)
    , m_baseX(0)
    , m_baseY(0)
    , m_speedRatio(10.0f)
    , m_dx(0)
    , m_dy(0)
    , m_changed(false)
    , m_BBVisibility(100.0)
{
    m_center = optix::make_float3(0.0f, 0.0f, 0.0f);
    m_rayCasting = false;
    m_registration = false;
    m_cameraPosition = optix::make_float3(0.0f, 0.0f, 1.0f);
    m_cameraU = optix::make_float3(1.0f, 0.0f, 0.0f);
    m_cameraV = optix::make_float3(0.0f, 1.0f, 0.0f);
    m_cameraW = optix::make_float3(0.0f, 0.0f, -1.0f);

    updateCam = true;

    sdfT = nullptr;
    t = 0;
    m_Radius = 1.4;
}

void vaBaseWidget::SetContext(optix::Context &context)
{
    vaBasicObject::SetContext(context);
    //TODO: set correct algorithm for this
}

void vaBaseWidget::CreateGeometryHandle()
{
    if (vaBasicObject::GetContext().get() != nullptr)
    {
        try
        {
            //for (int i=0;i<10;i++){
            if (t > 3) t = 0;
            switch (t) {
            case 1:
                sdfT = new optixSDFTorus();
                sdfT->SetContext(vaBasicObject::GetContext());
                sdfT->SetCenter1(optix::make_float3(0.0));
                sdfT->SetRadius1(optix::make_float3(0.4, 0.1, 0.0));
                break;
            case 0:
                sdfT = new optixSDFSphere();
                sdfT->SetContext(vaBasicObject::GetContext());
                sdfT->SetCenter1(optix::make_float3(5.0));
                sdfT->SetRadius1(optix::make_float3(1.4, 1.4, 1.4));
                break;
            case 2:
                sdfT = new optixSDFBox();
                sdfT->SetContext(vaBasicObject::GetContext());
                sdfT->SetCenter1(optix::make_float3(0.0));
                sdfT->SetRadius1(optix::make_float3(0.4, 0.5, 0.5));
                break;
            }
            t++;
            sdfT->Update();

            vaBasicMaterial mSdf;
            mSdf.SetContext(vaBasicObject::GetContext());
            mSdf.Update();

            m_prog = sdfT->GetCallableProg();
            UpdateGeo();

            //initiall creates the biggest geometry handle
            //as further bounding box is not updated
            m_geo["varRadius"]->setFloat(m_BBVisibility, m_BBVisibility, m_BBVisibility);

            map21 = new vaMapper();
            map21->SetContext(vaBasicObject::GetContext());
            map21->SetInput(sdfT->GetOutput());
            map21->AddMaterial(mSdf.GetOutput(), mSdf.GetType());
            map21->Update();

            acSdf1 = new vaActor();
            acSdf1->SetContext(vaBasicObject::GetContext()); //sets context and initialize acceleration properties

                                               //TODO: overwrite with mapper function that returns it's instance
                                               //ac.SetGeometry(mat, pl.GetOutput());
            acSdf1->AddMapper(map21);// .GetOutput());
            acSdf1->Update();
        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
}

void vaBaseWidget::SetCamParameters(optix::float3& pos, optix::float3& u, optix::float3& v, optix::float3& w, optix::float3& center, float&dist)
{
    sysCameraPosition = optix::make_float3(pos.x, pos.y, pos.z);
    sysCameraU = optix::make_float3(u.x, u.y, u.z);
    sysCameraV = optix::make_float3(v.x, v.y, v.z);
    sysCameraW = optix::make_float3(w.x, w.y, w.z);

    sysCameraCenter = center;
    sysCameraDist = dist;
    //std::cout << sysCameraCenter.x << "," << sysCameraCenter.y << sysCameraCenter.z << std::endl;
    //std::cout << dist << std::endl;
    updateCam = false;
}
void vaBaseWidget::UpdateHandlePosition()
{
    const optix::float2 pixel = optix::make_float2(m_baseX, m_baseY);

    //no antializing
    const optix::float2 fragment = pixel;

    // The launch dimension (set with rtContextLaunch) is the full client window in this demo's setup.
    const optix::float2 screen = optix::make_float2(m_width, m_height);

    const optix::float2 ndc = (fragment / screen) * 2.0f - 1.0f;

    const optix::float3 origin = sysCameraPosition;
    const float d_distancePlane = length(origin - sysCameraCenter); //distance from camerat to central point
    //const optix::float3 planeNorm = optix::make_float3(0, 0, -2.0);

    //optix::float3 dir = optix::make_float3(ndc.x, ndc.y, 0);
    const optix::float3 direction = optix::normalize(ndc.x * sysCameraU - ndc.y * sysCameraV + sysCameraW);
    const optix::float3 coord = origin + direction* sysCameraDist;
    //0.0 - center
    optix::float3 coordx = m_prog["varCenter"]->getFloat3();
    //coordx.x =coord.x;
    m_prog["varCenter"]->setFloat(coord.x, coord.y, coord.z);

    // m_geo["varCenter"]->setFloat(coordx.x, coordx.y, coordx.z);
    // m_geo->markDirty();
    // m_geo->validate();

    // std::cout << ndc.x << "," << ndc.y <<std::endl;
}

vaBaseWidget::~vaBaseWidget()
{
    delete sdfT;
}

void vaBaseWidget::setViewport(int w, int h)
{
    if (m_width != w || m_height != h)
    {
        // Never drop to zero viewport size. This avoids lots of checks for zero in other routines.
        m_width = (w) ? w : 1;
        m_height = (h) ? h : 1;
        m_aspect = float(m_width) / float(m_height);
        m_changed = true;
    }
}

void vaBaseWidget::setBaseCoordinates(int x, int y)
{
    m_baseX = x;
    m_baseY = y;
}

void vaBaseWidget::orbit(int x, int y)
{
    if (setDelta(x, y))
    {
        m_phi -= float(m_dx) / float(m_width); // Inverted.
        // Wrap phi.
        if (m_phi < 0.0f)
        {
            m_phi += 1.0f;
        }
        else if (1.0f < m_phi)
        {
            m_phi -= 1.0f;
        }

        m_theta += float(m_dy) / float(m_height);
        // Clamp theta.
        if (m_theta < 0.0f)
        {
            m_theta = 0.0f;
        }
        else if (1.0f < m_theta)
        {
            m_theta = 1.0f;
        }
    }
}

void vaBaseWidget::pan(int x, int y)
{
    if (setDelta(x, y))
    {
        // m_speedRatio pixels will move one vector length.
        float u = float(m_dx) / m_speedRatio;
        float v = float(m_dy) / m_speedRatio;
        // Pan the center of interest, the rest will follow.
        m_center = m_center - u * m_cameraU + v * m_cameraV;
    }
}

void vaBaseWidget::dolly(int x, int y)
{
    if (setDelta(x, y))
    {
        // m_speedRatio pixels will move one vector length.
        float w = float(m_dy) / m_speedRatio;
        // Adjust the distance, the center of interest stays fixed so that the orbit is around the same center.
        m_distance -= w * length(m_cameraW); // Dragging down moves the camera forwards. "Drag-in the object".
        if (m_distance < 0.001f) // Avoid swapping sides. Scene units are meters [m].
        {
            m_distance = 0.001f;
        }
    }
}

void vaBaseWidget::focus(int x, int y)
{
    if (setDelta(x, y))
    {
        // m_speedRatio pixels will move one vector length.
        float w = float(m_dy) / m_speedRatio;
        // Adjust the center of interest.
        setFocusDistance(m_distance - w * length(m_cameraW));
    }
}

void vaBaseWidget::setFocusDistance(float f)
{
    if (m_distance != f && 0.001f < f) // Avoid swapping sides.
    {
        m_distance = f;
        m_center = m_cameraPosition + m_distance * m_cameraW; // Keep the camera position fixed and calculate a new center of interest which is the focus plane.
        m_changed = true; // m_changed is only reset when asking for the frustum
    }
}

void vaBaseWidget::zoom(float x)
{
    m_fov += float(x);
    if (m_fov < 1.0f)
    {
        m_fov = 1.0f;
    }
    else if (179.0 < m_fov)
    {
        m_fov = 179.0f;
    }
    m_changed = true;
}

float vaBaseWidget::getAspectRatio() const
{
    return m_aspect;
}

bool vaBaseWidget::getFrustum(optix::float3& pos, optix::float3& u, optix::float3& v, optix::float3& w)
{
    bool changed = m_changed;
    if (changed)
    {
        // Recalculate the camera parameters.
        const float cosPhi = cosf(m_phi * 2.0f * M_PIf);
        const float sinPhi = sinf(m_phi * 2.0f * M_PIf);
        const float cosTheta = cosf(m_theta * M_PIf);
        const float sinTheta = sinf(m_theta * M_PIf);

        optix::float3 normal = optix::make_float3(cosPhi * sinTheta, -cosTheta, -sinPhi * sinTheta); // "normal", unit vector from origin to spherical coordinates (phi, theta)

        float tanFov = tanf((m_fov * 0.5f) * M_PIf / 180.0f); // m_fov is in the range [1.0f, 179.0f].
        m_cameraPosition = m_center + m_distance * normal;

        m_cameraU = m_aspect * optix::make_float3(-sinPhi, 0.0f, -cosPhi) * tanFov;               // "tangent"
        m_cameraV = optix::make_float3(cosTheta * cosPhi, sinTheta, cosTheta * -sinPhi) * tanFov; // "bitangent"
        m_cameraW = -normal;                                                                   // "-normal" to look at the center.

        pos = m_cameraPosition;
        u = m_cameraU;
        v = m_cameraV;
        w = m_cameraW;

        m_changed = false; // Next time asking for the frustum will return false unless the camera has changed again.
    }
    return changed;
}

bool vaBaseWidget::setDelta(int x, int y)
{
    if (m_baseX != x || m_baseY != y)
    {
        m_dx = x - m_baseX;
        m_dy = y - m_baseY;

        m_baseX = x;
        m_baseY = y;

        m_changed = true; // m_changed is only reset when asking for the frustum.
        return true; // There is a delta.
    }
    return false;
}

void vaBaseWidget::setSpeedRatio(float f)
{
    m_speedRatio = f;
    if (m_speedRatio < 0.01f)
    {
        m_speedRatio = 0.01f;
    }
    else if (100.0f < m_speedRatio)
    {
        m_speedRatio = 100.0f;
    }
}