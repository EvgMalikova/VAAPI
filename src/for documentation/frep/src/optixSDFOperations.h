/*=========================================================================
Program:   VolumeRenderer
Module:    optixSDFOperations

=========================================================================*/

#ifndef optixSDFOperations_h
#define optixSDFOperations_h

#include "optixSDFBasicOperations.h"

/**
* \class  SDFRoundingOp
* \brief  Smoothing operation
*
* For math of operation [see] (https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm)
*
*/
class SDFRoundingOp : public optixSDFUnaryOp
{
public:

    SDFRoundingOp() { m_k = 0.1; };
    ~SDFRoundingOp() {};

    //Set
    void SetKoeff(float k) { //blending koefficient
        m_k = k;
        optix::Program pr = optixSDFGeometry::GetCallableProg();
        pr["varK"]->setFloat(k);
        //   optixSDFGeometry::GetOutput()["varK"]->setFloat();
    };
    float GetKoeff() { return m_k; };
protected:
    float m_k;

    //initialize callable proc
    virtual void InitCallableProg();
    virtual void AdjustCenterAndBoundingBox();
};

/**
* \class  SDFElongateOp
* \brief  Elongation operation
*
* For math of operation [see] (https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm)
*
*/
class SDFElongateOp : public optixSDFUnaryOp
{
public:

    SDFElongateOp() { m_h = optix::make_float3(0.1); };
    ~SDFElongateOp() {};

    //Set
    void SetHKoeff(optix::float3 h) { //blending koefficient
        m_h = h;
        optix::Program pr = optixSDFGeometry::GetCallableProg();
        pr["varRadius"]->setFloat(h.x, h.y, h.z);
        //   optixSDFGeometry::GetOutput()["varK"]->setFloat();
    };
    optix::float3 GetHKoeff() { return m_h; };
protected:
    optix::float3 m_h;

    //initialize callable proc
    virtual void InitCallableProg();
    virtual void AdjustCenterAndBoundingBox();
};

/**
* \class  SDFBlendUnionOp
* \brief  Blending union operation
*
* For math of operation [see] (https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm)
*
*/
class SDFBlendUnionOp : public optixSDFBinaryOp
{
public:

    SDFBlendUnionOp() { m_k = 0.1; };
    ~SDFBlendUnionOp() {};

    /**
    * sets blending koefficient
    */
    void SetKoeff(float k) {
        m_k = k;
        optix::Program pr = optixSDFGeometry::GetCallableProg();
        pr["varK"]->setFloat(k);
    };
    /**
   * returns blending koefficient
   */
    float GetKoeff() { return m_k; };
protected:
    float m_k;

    //initialize callable proc
    virtual void InitCallableProg();
    virtual void AdjustCenterAndBoundingBox();
};

/*

\class SDFBlendIntersectionOp
*/

class SDFBlendIntersectionOp : public optixSDFBinaryOp
{
public:

    SDFBlendIntersectionOp() { m_k = 0.1; };
    ~SDFBlendIntersectionOp() {};

    /**
    * sets blending koefficient
    */
    void SetKoeff(float k) {
        m_k = k;
        optix::Program pr = optixSDFGeometry::GetCallableProg();
        pr["varK"]->setFloat(k);
    };
    /**
    * returns blending koefficient
    */
    float GetKoeff() { return m_k; };
protected:
    float m_k;

    //initialize callable proc
    virtual void InitCallableProg();
    virtual void AdjustCenterAndBoundingBox();
};

//----------------------------------------------------
/*

\class SDFBlendSubtractionOp
*/

class SDFBlendSubtractionOp : public optixSDFBinaryOp
{
public:

    SDFBlendSubtractionOp() { m_k = 0.1; };
    ~SDFBlendSubtractionOp() {};

    /**
    * sets blending koefficient
    */
    void SetKoeff(float k) {
        m_k = k;
        optix::Program pr = optixSDFGeometry::GetCallableProg();
        pr["varK"]->setFloat(k);
    };
    /**
    * returns blending koefficient
    */
    float GetKoeff() { return m_k; };
protected:
    float m_k;

    //initialize callable proc
    virtual void InitCallableProg();
    virtual void AdjustCenterAndBoundingBox();
};

//----------------------------------------------------
/*

\class SDFSubtractionOp
*/

class SDFSubtractionOp : public optixSDFBinaryOp
{
public:

    SDFSubtractionOp() { };
    ~SDFSubtractionOp() {};

protected:

    //initialize callable proc
    virtual void InitCallableProg();
    virtual void AdjustCenterAndBoundingBox();
};

#endif