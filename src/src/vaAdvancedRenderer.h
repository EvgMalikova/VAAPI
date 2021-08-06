/*=========================================================================
Program:   VolumeRenderer
Module:    vaAdvancedRenderer.h

=========================================================================*/

#ifndef vaAdvancedRenderer_h
#define vaAdvancedRenderer_h

#include "vaRenderer.h"

/*
*\class vaAdvancedRenderer
*\brief  Antialized rendering.

*/
class vaAdvancedRenderer :public vaRenderer
{
public:

    vaAdvancedRenderer() {
        InitDefaultModels();

        m_minPathLength = 2;    // Minimum path length after which Russian Roulette path termination starts.
        m_maxPathLength = 4;    // Maximum path length. Need at least 6 path segments to go through a glass sphere, hit something, and back through that sphere to the viewer.

    //for itteration
        m_frames = 0; // Samples per pixel. 0 == render forever.
        m_present = false;  // Update once per second. (The first half second shows all frames to get some initial accumulation).
        m_presentNext = true;
        m_presentAtSecond = 1.0;

        m_iterationIndex = 0;
    };
    ~vaAdvancedRenderer() {};

    /*Main rendering procedure - TODO: optimise in vaRenderer*/
    virtual bool Render();

    // void GetMissProg();
private:
    int m_minPathLength;    // Minimum path length after which Russian Roulette path termination starts.
    int m_maxPathLength;    // Maximum path length. Need at least 6 path segments to go through a glass sphere, hit something, and back through that sphere to the viewer.

    //for itteration
    int  m_frames; // Samples per pixel. 0 == render forever.
    bool m_present;  // Update once per second. (The first half second shows all frames to get some initial accumulation).
    bool  m_presentNext;
    float  m_presentAtSecond;
    int m_iterationIndex;
protected:

    void restartAccumulation();
    /*Inits default models*/
    virtual void InitDefaultModels();

    virtual void SetRayGenerationProg();
    virtual void SetExceptionProg();
    virtual void SetMissProg();
    //-----------------------------
    virtual void InitRenderer(); //entry point for all programs and rendering stuff, both optical and auditory

    /* updates camera. Returns true if camara was updated*/
    virtual bool UpdateCamera();

    /*Launches context for antialising renderer*/
    virtual void LaunchOpticContext();
    virtual void Modified() {};
};
#endif