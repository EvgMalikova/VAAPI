#include "vaRayCastBaseWidget.h"

//For registration
#include "itkTranslationTransform.h"
#include "itkEuclideanDistancePointMetric.h"
#include "itkLevenbergMarquardtOptimizer.h"
#include "itkPointSetToPointSetRegistrationMethod.h"
//---------------

void vaRayCastBaseWidget::UpdateHandlePosition()
{
    vaBaseWidget::UpdateHandlePosition();
    UpdateWidgetPos();
}

void vaRayCastBaseWidget::CreateGeometryHandle()
{
    vaBaseWidget::CreateGeometryHandle();

    //sets this as a ray-casting procedure
    if (isRayCast())
    {
        SetHandleAsCallableProg();
        UpdateWidgetPos();

        //create a ray-casting buffers
        BindRayCastBuffers();
    }
}

vaRayCastBaseWidget::PointType vaRayCastBaseWidget::GetPoint(optix::float3 pp)
{
    //TODO:
    PointType p;
    p[0] = pp.x;
    p[1] = pp.y;
    p[2] = pp.z;
    return p;
}

void vaRayCastBaseWidget::UpdateWidgetPos()
{
    // widgetCenter
    optix::float3 pos = m_prog["varCenter"]->getFloat3();// etFloat(coord.x, coord.y, coord.z);
    optix::Program ray_cast = GetRayCastProg();
    if (ray_cast->get() != nullptr) {
        ray_cast["widgetCenter"]->setFloat(pos.x, pos.y, pos.z);
        //std::cout << "POS " << pos.x << "," << pos.y << "," << pos.z << std::endl;
    }
}
void vaRayCastBaseWidget::SetContext(optix::Context &context)
{
    vaBasicObject::SetContext(context);
    //TODO: set correct algorithm for this
    if (isRayCast()) {
        InitProg("audio_ray_cast", "raygeneration.cu", "audio_ray_cast");
        SetRayCastProgName("audio_ray_cast");
    }
}

void vaRayCastBaseWidget::SetRayCastSize(int width, int height)
{
    if (isRayCast()) {
        m_FixedBuffer->setSize(width, height);
        m_MovingBuffer->setSize(width, height);
        m_SoundWidth = width;
        m_SoundHeight = height;
    }
}

//-----------------------------
//  Registration procedure icp
//-----------------------------
void vaRayCastBaseWidget::RegisterBuffers()
{
    if (isRayCast()) {
        PointSetType::Pointer fixedPointSet = PointSetType::New();
        PointSetType::Pointer movingPointSet = PointSetType::New();

        //using PointType = PointSetType::PointType;

        using PointsContainer = PointSetType::PointsContainer;

        PointsContainer::Pointer fixedPointContainer = PointsContainer::New();
        PointsContainer::Pointer movingPointContainer = PointsContainer::New();

        UpdateRayCastBuffers(fixedPointContainer, movingPointContainer);

        fixedPointSet->SetPoints(fixedPointContainer);
        std::cout << "Number of fixed Points = " << fixedPointSet->GetNumberOfPoints()
            << std::endl;

        movingPointSet->SetPoints(movingPointContainer);
        std::cout << "Number of moving Points = " << movingPointSet->GetNumberOfPoints()
            << std::endl;

        // Software Guide : BeginLatex
        //
        // After the points are read in from files, set up the metric type.
        //
        // Software Guide : EndLatex

        // Software Guide : BeginCodeSnippet
        using MetricType = itk::EuclideanDistancePointMetric<PointSetType, PointSetType>;

        MetricType::Pointer metric = MetricType::New();
        // Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        //
        // Now, setup the transform, optimizers, and registration method using the
        // point set types defined earlier.
        //
        // Software Guide : EndLatex

        // Software Guide : BeginCodeSnippet
        using TransformType = itk::TranslationTransform<double, 3>;

        TransformType::Pointer transform = TransformType::New();

        // Optimizer Type
        using OptimizerType = itk::LevenbergMarquardtOptimizer;

        OptimizerType::Pointer optimizer = OptimizerType::New();
        optimizer->SetUseCostFunctionGradient(false);

        // Registration Method
        using RegistrationType =
            itk::PointSetToPointSetRegistrationMethod<PointSetType, PointSetType>;

        RegistrationType::Pointer registration = RegistrationType::New();

        // Scale the translation components of the Transform in the Optimizer
        OptimizerType::ScalesType scales(transform->GetNumberOfParameters());
        scales.Fill(0.1);
        // Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        //
        // Next we setup the convergence criteria, and other properties required
        // by the optimizer.
        //
        // Software Guide : EndLatex

        // Software Guide : BeginCodeSnippet
        unsigned long numberOfIterations = 300;
        double        gradientTolerance = 1e-5; // convergence criterion
        double        valueTolerance = 1e-5;    // convergence criterion
        double        epsilonFunction = 1e-6;   // convergence criterion

        optimizer->SetScales(scales);
        optimizer->SetNumberOfIterations(numberOfIterations);
        optimizer->SetValueTolerance(valueTolerance);
        optimizer->SetGradientTolerance(gradientTolerance);
        optimizer->SetEpsilonFunction(epsilonFunction);
        // Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        //
        // In this case we start from an identity transform, but in reality the user
        // will usually be able to provide a better guess than this.
        //
        // Software Guide : EndLatex

        // Software Guide : BeginCodeSnippet
        transform->SetIdentity();
        // Software Guide : EndCodeSnippet

        registration->SetInitialTransformParameters(transform->GetParameters());

        // Software Guide : BeginLatex
        //
        // Finally, connect all the components required for the registration, and an
        // observer.
        //
        // Software Guide : EndLatex

        // Software Guide : BeginCodeSnippet
        registration->SetMetric(metric);
        registration->SetOptimizer(optimizer);
        registration->SetTransform(transform);
        registration->SetFixedPointSet(fixedPointSet);
        registration->SetMovingPointSet(movingPointSet);

        try
        {
            registration->Update();
        }
        catch (itk::ExceptionObject & e)
        {
            std::cerr << e << std::endl;
            //return EXIT_FAILURE;
        }

        std::cout << "Solution = " << transform->GetParameters()[0] << std::endl;
        //return EXIT_SUCCESS;

        //update widget position
        UpdateGeo();

        //Show();
        optix::float3 c = m_prog["varCenter"]->getFloat3();
        c.x += transform->GetParameters()[0] * 0.1; // (250 * 0.4);
        c.y += transform->GetParameters()[1] * 0.1; // (250 * 0.4);
        c.z += transform->GetParameters()[2] * 0.1; // (250 * 0.4);

                                                    //c.x /= (250 * 0.4);
                                                    //c.y /= (250 * 0.4);
                                                    //c.z /= (250 * 0.4);

        m_prog["varCenter"]->setFloat(c.x, c.y, c.z);
        //m_geo->markDirty();
    }
}

void vaRayCastBaseWidget::BindRayCastBuffers()
{
    optix::Program pr = GetRayCastProg();
    m_FixedBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT3, 0, 0);
    // m_FixedBuffer->setElementSize(sizeof(optix::float3));

    m_MovingBuffer = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT3, 0, 0);
    // m_MovingBuffer->setElementSize(sizeof(optix::float3));

    m_FixedBuffer->setSize(m_SoundWidth, m_SoundHeight);
    m_MovingBuffer->setSize(m_SoundWidth, m_SoundHeight);
    pr["fixedPoints"]->setBuffer(m_FixedBuffer);
    pr["movingPoints"]->setBuffer(m_MovingBuffer);
}

void vaRayCastBaseWidget::SetHandleAsCallableProg() {
    optix::Program pr = GetHandleProg();
    optix::Program ray_cast = GetRayCastProg();
    if (ray_cast->get() != nullptr) {
        if (pr->get() != nullptr) {
            ray_cast["sdfPrim"]->setProgramId(pr);
            // this->GetContext()
            // it->second->getId()
        }
    }
}
void vaRayCastBaseWidget::UpdateRayCastBuffers(PointsContainer* fPoints, PointsContainer* mPoints)
{
    optix::float3* FixedImageData = (optix::float3*)(m_FixedBuffer->map());
    optix::float3* MovingImageData = (optix::float3*)(m_MovingBuffer->map());

    // int w, h;
    // m_window->GetRenderer()->GetAudioDim(w, h);

    // m_distNorm = 0;
    std::cout << m_SoundWidth << "," << m_SoundHeight << std::endl;

    PointType fixedPoint;
    PointType movingPoint;
    // Software Guide : EndCodeSnippet

    // Read fixed and moving points from optix Buffers
    unsigned int pointId = 0;
    for (unsigned int y = 0; y < m_SoundHeight; ++y)
    {
        for (unsigned int x = 0; x < m_SoundWidth; ++x)
        {
            int index = y * m_SoundWidth + x;
            optix::float3 pF = FixedImageData[index];
            optix::float3 pM = MovingImageData[index];

            fixedPoint = GetPoint(pF);
            movingPoint = GetPoint(pM);
            if (fixedPoint[0] != -1000) { //there is an intersection
                fPoints->InsertElement(pointId, fixedPoint);
                mPoints->InsertElement(pointId, movingPoint);

                pointId++;
            }
        }
    }

    m_FixedBuffer->unmap();
    m_MovingBuffer->unmap();

    std::cout << "Copied " << std::endl;
}