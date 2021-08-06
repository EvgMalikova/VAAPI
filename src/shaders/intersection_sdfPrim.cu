/*
All basic variables for SDFs visual-auditory ray-tracing
 */
#include "sdfGeometryVariables.h"

 //for SDF spehre and others
rtDeclareVariable(optix::float3, varCenter, , );
rtDeclareVariable(optix::float3, varRadius, , );

//for SDF

typedef rtCallableProgramId<float(float3, float3)> callT;
rtDeclareVariable(callT, sdfPrim, , );

//TODO:
//or create a callT Buffer

//second buffer of parameters for the  buffer of called programs
//should be created as well

RT_PROGRAM void intersection_sdf_sphere(int primIdx)
{
    float tmin, tmax;
    tmin = 0;
    tmax = RT_DEFAULT_MAX;

    const float sqRadius = 100;

    float distance;
    // === Raymarching (Sphere Tracing) Procedure ===
    optix::float3 rd = theRay.direction;
    optix::float3 eye = theRay.origin;
    float eps = 0.001;
    float t = 0.002;
    float material = -1;
    if (varRadius.x > 0.0) //responsible for show/hide
    {
        for (int i = 0; i < 1000; i++)
        {
            optix::float3 p = eye + t* rd;
            float hit = sdfPrim(p, varRadius);
            if (hit < 0.002) //|| t > 20.0)
            {
                //printf("was hitted\n");
                if (rtPotentialIntersection(t))
                {
                    //        sdf.setMaxIterations(14); // more iterations for normal estimate, to fake some more detail
                    // varNormal        = calculateNormal(sdf, x, DEL);

                    float dx = sdfPrim(p + make_float3(eps, 0, 0), varRadius) - sdfPrim(p - make_float3(eps, 0, 0), varRadius);
                    float dy = sdfPrim(p + make_float3(0, eps, 0), varRadius) - sdfPrim(p - make_float3(0, eps, 0), varRadius);
                    float dz = sdfPrim(p + make_float3(0, 0, eps), varRadius) - sdfPrim(p - make_float3(0, 0, eps), varRadius);

                    info.normal = normalize(make_float3(dx, dy, dz));
                    info.hit_point = theRay.origin + t*theRay.direction;
                    info.type = 1;
                    info.maxDist = varRadius.x*2;
                    //rtReportIntersection(0);
                    rtReportIntersection(MaterialIndex);
                }
                break;
            }

            if (t > 20.0) {
                break; //no intersection
            }
            t += abs(hit); //only positive direction
        }
    }
}

RT_PROGRAM void boundingbox_sdf_sphere(int, float result[6])
{
    optix::Aabb* aabb = (optix::Aabb*)result;

    aabb->m_min = varCenter - varRadius;// optix::make_float3(varCenter.x - varRadius, varCenter.y - varRadius, varCenter.z - varRadius);
    aabb->m_max = varCenter + varRadius; // optix::make_float3(varCenter.x + varRadius, varCenter.y + varRadius, varCenter.z + varRadius);
}