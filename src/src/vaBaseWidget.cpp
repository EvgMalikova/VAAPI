/*

 */

#include "vaBaseWidget.h"

#include "optixMaterial.h"
#include "optixSDFOperations.h"
#include "vaActor.h"
#include <iostream>

 //---

vaBaseWidget::vaBaseWidget()
    : m_shift(1.0)
    //,m_distance(10.0f) // Some camera defaults for the demo scene.

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
    //m_center = optix::make_float3(0.0f, 0.0f, 0.0f);
    m_rayCasting = false;
    m_registration = false;
    useController = false;
    m_cameraPosition = optix::make_float3(0.0f, 0.0f, 1.0f);
    m_cameraU = optix::make_float3(1.0f, 0.0f, 0.0f);
    m_cameraV = optix::make_float3(0.0f, 1.0f, 0.0f);
    m_cameraW = optix::make_float3(0.0f, 0.0f, -1.0f);

    //    updateCam = true;

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
                sdfT = new optixSDFHand();
                sdfT->SetContext(vaBasicObject::GetContext());
                sdfT->SetCenter1(optix::make_float3(1.0));
                sdfT->SetRadius1(optix::make_float3(0.1, 0.1, 0.1));

                /* sdfT = new optixSDFSphere();
                 sdfT->SetContext(vaBasicObject::GetContext());
                 sdfT->SetCenter1(optix::make_float3(5.0));
                 sdfT->SetRadius1(optix::make_float3(1.4, 1.4, 1.4));

                 for (int i = 0; i < 5; i++) {
                     optixSDFSphere sdfT2;// = new optixSDFSphere();
                     sdfT2.SetContext(vaBasicObject::GetContext());
                     sdfT2.SetCenter1(optix::make_float3(5.0));
                     sdfT2.SetRadius1(optix::make_float3(1.4, 1.4, 1.4));

                     SDFBlendUnionOp opBlend;
                     opBlend.SetContext(vaBasicObject::GetContext());
                     opBlend.AddOpperand1(sdfT);
                     opBlend.AddOpperand2(&sdfT2);
                     opBlend.SetKoeff(0.3);
                     opBlend.Update();
                 }*/

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

    //optix::float3 dir = optix::normalize(sysCameraPosition - sysCameraCenter);
    sysCameraCenter = center;// +dir*m_fov;
    sysCameraDist = dist;
    //std::cout << sysCameraCenter.x << "," << sysCameraCenter.y << sysCameraCenter.z << std::endl;
    //std::cout << dist << std::endl;
   // updateCam = false;
}

void vaBaseWidget::SetHandlePosition(optix::float3 coord) {
    // Show();
    sysCameraPosition = coord;
    useController = true;
    optix::float3 pos = optix::make_float3(sysCameraPosition.x / 60, sysCameraPosition.y / 60, sysCameraPosition.z / 60);

    m_prog["varCenter"]->setFloat(pos.x, pos.y, pos.z);
    std::cout << pos.x << "," << pos.y << "," << pos.z << std::endl;
}

void vaBaseWidget::SetFingerPosition(int i, optix::float3 coord) {
    // Show();
    sysCameraPosition = coord;
    useController = true;

    optix::float3 pos = optix::make_float3(sysCameraPosition.x / 60, sysCameraPosition.y / 60, sysCameraPosition.z / 60);

    std::string name = "varCenter" + std::to_string(i);
    m_prog[name]->setFloat(pos.x, pos.y, pos.z);
    std::cout << name << coord.x << std::endl;
}
void vaBaseWidget::UpdateHandlePosition()
{
    if (useController) {
        optix::float3 pos = optix::make_float3(sysCameraPosition.x / 20, sysCameraPosition.y / 20, sysCameraPosition.z / 20);
        //m_prog["varCenter"]->setFloat(pos);
       // std::cout << pos.x << "," << pos.y << "," << pos.z << std::endl;
    }
    else {
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
        const optix::float3 coord = origin + direction* sysCameraDist*m_shift;
        //0.0 - center
        optix::float3 coordx = m_prog["varCenter"]->getFloat3();
        //coordx.x =coord.x;
        m_prog["varCenter"]->setFloat(coord.x, coord.y, coord.z);
    }
    // m_geo["varCenter"]->setFloat(coordx.x, coordx.y, coordx.z);
    // m_geo->markDirty();
    // m_geo->validate();

    // std::cout << ndc.x << "," << ndc.y <<std::endl;
}

vaBaseWidget::~vaBaseWidget()
{
    delete sdfT;
}

void vaBaseWidget::SetViewport(int w, int h)
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

void vaBaseWidget::SetBaseCoordinates(int x, int y)
{
    m_baseX = x;
    m_baseY = y;
}

void vaBaseWidget::zoom(float x)
{
    m_shift += float(x / (m_speedRatio * 10));
    if (m_shift < 0.01f)
    {
        m_shift = 0.01f;
    }
    else if (2.0 < m_shift)
    {
        m_shift = 2.0f;
    }
    //SetFocusDistance(m_distance);
    m_changed = true;
    // std::cout << "Dist= " << m_distance << std::endl;
}

float vaBaseWidget::getAspectRatio() const
{
    return m_aspect;
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

void vaBaseWidget::SetSpeedRatio(float f)
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