/*=========================================================================
Program:   VolumeRenderer
Module:    optixSDFPrimitives.h

=========================================================================*/

#ifndef optixSDFBasicOperations_h
#define optixSDFBasicOperations_h

#include "optixSDFGeometry.h"

#include "macros.h"

/**
* \class  optixSDFUnaryOp
* \brief Unary operation
*
*  Abstract class. Defines sdf unary operation for FRep tree construction pipeline
*
*/
class optixSDFUnaryOp : public optixSDFGeometry {
public:

    optixSDFUnaryOp() {
        m_sdf = nullptr;//geoDesc = new sdfGeo();
    };
    ~optixSDFUnaryOp() { //delete geoDesc;
    };

    virtual void Update() {
        optixSDFGeometry::Update();
        if (m_sdf != nullptr)
            m_sdf->SetCallableProgManually(GetCallableProg());
    };
    virtual optix::Geometry GetOutput() {
        return m_sdf->GetOutput();
    }
    //redefine context proc
    virtual void SetContext(optix::Context &context);
    virtual void SetInput(optix::Geometry geo1) { g1 = geo1; };
    optix::Geometry GetInput() { return g1; };

    void AddOpperand(optixSDFGeometry* sdf) {
        m_sdf = sdf;
        SetInput(sdf->GetOutput());
    }

    optixSDFGeometry* GetOutputSdfObject() { return m_sdf; }

protected:
    optixSDFGeometry* m_sdf;

    virtual void CreateGeometry();
    //virtual void GenerateGeometry();
    virtual void Initialize();

    virtual void SetCallableProg();
    virtual void SetMainPrograms();

    virtual void InitCallableProg() {};
    virtual void AdjustCenterAndBoundingBox() {};
private:
    //optix::Program primProg;
    optix::Geometry g1;
};

/**
* \class  optixSDFBinaryOp
* \brief Binary operation
*
*  Abstract class. Defines sdf Binary operation for FRep tree construction pipeline
*
*/

class optixSDFBinaryOp : public optixSDFGeometry {
public:

    optixSDFBinaryOp() {
        m_sdf1 = nullptr;
        m_sdf2 = nullptr;
    };
    ~optixSDFBinaryOp() {
    };

    virtual void Update() {
        optixSDFGeometry::Update();

        if (m_sdf1 != nullptr)
            m_sdf1->SetCallableProgManually(GetCallableProg());
    };
    virtual optix::Geometry GetOutput() {
        return m_sdf1->GetOutput();
    }
    //redefine context proc
    virtual void SetContext(optix::Context &context);
    virtual void SetInput1(optix::Geometry geo1) { g1 = geo1; };
    optix::Geometry GetInput1() { return g1; };
    virtual void SetInput2(optix::Geometry geo1) { g2 = geo1; };
    optix::Geometry GetInput2() { return g2; };

    void AddOpperand1(optixSDFGeometry* sdf) {
        m_sdf1 = sdf;
        SetInput1(sdf->GetOutput());
    }

    void AddOpperand2(optixSDFGeometry* sdf) {
        m_sdf2 = sdf;
        SetInput2(sdf->GetOutput());
    }
protected:
    optixSDFGeometry* m_sdf1;
    optixSDFGeometry* m_sdf2;

    //Porcedure for geometry creation
    virtual void CreateGeometry();
    //virtual void GenerateGeometry();
    virtual void Initialize();

    virtual void SetCallableProg();
    virtual void SetMainPrograms();

    virtual void InitCallableProg() {};
    virtual void AdjustCenterAndBoundingBox() {};
private:
    //optix::Program primProg;
    optix::Geometry g1;
    optix::Geometry g2;
protected:
    sdfGeo* geoDesc1;
    sdfGeo* geoDesc2;
};
#endif