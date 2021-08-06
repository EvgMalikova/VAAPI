/*
All basic variables for SDFs visual-auditory ray-tracing
 */
#include "sdfGeometryVariables.h"

#include "sdfPrimPrograms.h"
using namespace optix;
rtDeclareVariable(float3, sysCameraPosition, , );
rtBuffer<float3>    Positions;
rtBuffer<int2>    Bonds;
rtBuffer<int4>    Mols4;

rtBuffer<float>    BSRadius;
rtBuffer<int>    BSType;
rtDeclareVariable(float, MultiscaleParam, , );
rtDeclareVariable(float3, pr_pos, attribute primitive_pos, );
rtDeclareVariable(float, pr_rad, attribute primitive_rad, );
//rtDeclareVariable(int, pr_type, attribute primitive_type, );

rtDeclareVariable(float, sysSceneEpsilon, , );

rtDeclareVariable(int, MolSize, , );

//rtDeclareVariable(optix::float3, varNormal, attribute NORMAL, ); //for direct tracing of sdf spheres, or defined set of primitives with definde BB

#include "renderer/per_ray_data.h"
rtDeclareVariable(PerRayData, thePrd, rtPayload, );

//TODO:for SDF
//still have to be implemented as PTX should be generated automatically
//PTX can automatically generated for some primitives like spheres, boxes and etc
//consider further integration with python

//for SDF

//for dynamic staff
rtDeclareVariable(int, PNum, , );
rtDeclareVariable(int, numFrames, , );
rtDeclareVariable(float, TimeSound, , );

inline __device__ float evaluateFunction(float3 x, primParamDesc descPrim)
{
    int type = descPrim.type;
    float f = 10000.0;
    float3 pos1 = descPrim.pos[0];
    float rad1 = descPrim.rad[0];

    float3 pos2 = descPrim.pos[1];
    float rad2 = descPrim.rad[1];

    float3 pos3 = descPrim.pos[2];
    float rad3 = descPrim.rad[2];

    float3 pos4 = descPrim.pos[3];
    float rad4 = descPrim.rad[3];

    //float3 pos5 = descPrim.pos[4];
    //float rad5 = descPrim.rad[4];

    float3 pos = (pos1 + pos2 + pos3 + pos4) / 4.0;
    f = length(x - pos) - rad1;
    //f = sdfPrim5(x, pos1, pos2, pos3, pos4, rad1, rad2);

    return f;
}

inline __device__ float3 computeNormal(float eps, float3 x, primParamDesc descPrim)
{
    float dx = evaluateFunction(x + make_float3(eps, 0, 0), descPrim) - evaluateFunction(x - make_float3(eps, 0, 0), descPrim);
    float dy = evaluateFunction(x + make_float3(0, eps, 0), descPrim) - evaluateFunction(x - make_float3(0, eps, 0), descPrim);
    float dz = evaluateFunction(x + make_float3(0, 0, eps), descPrim) - evaluateFunction(x - make_float3(0, 0, eps), descPrim);

    return normalize(make_float3(dx, dy, dz));
}

inline __device__ float SphereTraceForward(float epsilon, float t, float tmax, primParamDesc descPrim)
{
    optix::float3 ray_direction = theRay.direction;
    optix::float3 x = theRay.origin + theRay.direction*t;

    float dist;

    float totalDistance = t;
    int i = 0;
    bool stop = false;
    while (!stop)
    {
        dist = evaluateFunction(x, descPrim);// sdfPrim1(x, pos, pos2, rad1, rad2);

                                             // Step along the ray and accumulate the distance from the origin.
        x += abs(dist) * ray_direction;
        totalDistance += abs(dist);

        // Check if we're close enough or too far.
        if (abs(dist) < epsilon || totalDistance >= tmax)
        {
            stop = true;
        }
        if (dist < 0) {
            //x -= abs(dist) * ray_direction;
            totalDistance -= abs(dist);
            stop = true;
        }
    }
    return totalDistance;
}

inline __device__ float SphereTraceBack(float epsilon, float t, float tmax, primParamDesc descPrim)
{
    optix::float3 ray_direction = theRay.direction;
    optix::float3 x = theRay.origin + theRay.direction*tmax;

    float dist;

    //TODO: getType

    float totalDistance = tmax - t;
    int i = 0;
    bool stop = false;

    while (!stop)
    {
        //dist = sdfPrim1(x, pos, pos2, rad1, rad2);
        dist = evaluateFunction(x, descPrim);// sdfPrim1(x, pos, pos2, rad1, rad2);

                                             // Step along the ray and accumulate the distance from the origin.
        x -= abs(dist) * ray_direction;
        totalDistance -= abs(dist);

        // Check if we're close enough or too far.
        if (abs(dist) < epsilon || totalDistance <= 0)
        {
            stop = true;
        }
        if (dist < 0) {
            x += abs(dist) * ray_direction;
            totalDistance += abs(dist);
            stop = true;
        }
    }
    return totalDistance;
}

//------------------------

inline __device__  primParamDesc getTimeData(int primIdx)
{
    const int4 idsBonds = Mols4[primIdx];

    const int2 idsB1 = Bonds[idsBonds.x];
    const int2 idsB2 = Bonds[idsBonds.y];
    const int2 idsB3 = Bonds[idsBonds.z];
    const int2 idsB4 = Bonds[idsBonds.w];

    //idsB1.x==idsB2.x;

    //we know there are only 3 atoms - TODO:
    const float rad1 = BSRadius[idsB1.x - 1];
    const float rad2 = BSRadius[idsB1.y - 1];
    const float rad3 = BSRadius[idsB2.y - 1];
    const float rad4 = BSRadius[idsB3.y - 1];
    const float rad5 = BSRadius[idsB4.y - 1];

    const int type = BSType[idsB1.x - 1];
    const int type2 = BSType[idsB1.y - 1];
    const int type3 = BSType[idsB2.y - 1];
    const int type4 = BSType[idsB3.y - 1];
    const int type5 = BSType[idsB4.y - 1];

    //---------------
    int lower = int(floorf(TimeSound));
    int upper = int(ceilf(TimeSound));

    //0,1;1,2;2,3;

    float timeS = TimeSound;
    if (upper > numFrames) upper = numFrames;

    float time = timeS - float(lower);

    primParamDesc descPrim;

    descPrim.type = 5; //sphere data type
    descPrim.pos[0] = Positions[idsB4.y - 1]; //B1.x
    descPrim.pos[1] = Positions[idsB1.y - 1];
    descPrim.pos[2] = Positions[idsB2.y - 1];
    descPrim.pos[3] = Positions[idsB3.y - 1];
    //descPrim.pos[4] = Positions[idsB4.y - 1];

    descPrim.rad[0] = rad1;
    descPrim.rad[1] = rad2;
    descPrim.rad[2] = rad3;
    descPrim.rad[3] = rad4;
    // descPrim.rad[4] = rad5;

    descPrim.types[0] = type;
    descPrim.types[1] = type2;
    descPrim.types[2] = type3;
    descPrim.types[3] = type4;
    // descPrim.types[4] = type5;

    return descPrim;
}
//------------------------------------------------------
//---intersection with dynamic molecule, use of morphing
//------------------------------------------------------

inline __device__   float3 boundIntersection(primParamDesc  descPrim, float3 origin, float3 direction)
{
    // float t1, t2;
    float3 pos_along_ray1, per_ray_data2;
    float rayBoundSphere1, rayBoundSphere2;
    int caseN = -1; //ray miss

                    // float leng = length(pos2 - pos) + length(pos3 - pos);
    const float3 cent = (descPrim.pos[0] + descPrim.pos[1] + descPrim.pos[2] + descPrim.pos[3]) / 4.0;
    // const float delta = fmax(fmaxf(descPrim.rad[1], descPrim.rad[2]), descPrim.rad[0]);
    const float rayBoundSphere = 1.65; // length(cent - descPrim.pos[0]) + delta * 2;//include rad*2 for vibrations movement

    float t = length(cent - theRay.origin);
    float3 pos_along_ray = theRay.origin + theRay.direction * t;
    float tmax = t + 2 * rayBoundSphere; //to stop sphere tracing

                                   /* alternative splitting*/
                                   /*
                                   float3 cent1=(pos+pos2)/2;
                                   float3 cent2= (pos+pos3)/2;

                                   t1 = length(cent1 - theRay.origin);
                                   t2 = length(cent2 - theRay.origin);
                                   pos_along_ray1 = theRay.origin + theRay.direction * t1;
                                   pos_along_ray2 = theRay.origin + theRay.direction * t2;
                                   rayBoundSphere1 =length(cent1 - pos) + delta * 2;
                                   rayBoundSphere2 =length(cent1 - pos) + delta * 2;
                                   */

                                   /*float radiusB1 = length(descPrim.pos[0] - descPrim.pos[1]) / 2 + delta;
                                   float radiusB2 = length(descPrim.pos[0] - descPrim.pos[2]) / 2 + delta;
                                   float radiusB = fmaxf(radiusB1, radiusB2);*/

    float     tmin;

    if (length(cent - pos_along_ray) < rayBoundSphere)
        tmin = fmaxf(0.0, t - rayBoundSphere);
    /*
    t1 = length(descPrim.pos[1] - theRay.origin);
    pos_along_ray = theRay.origin + theRay.direction * t1;

    if (length(descPrim.pos[1] - pos_along_ray) < radiusB1)
        caseN = 1; //left

    t2 = length(descPrim.pos[2] - theRay.origin);
    pos_along_ray = theRay.origin + theRay.direction * t2;
    if (length(descPrim.pos[2] - pos_along_ray) < radiusB2)
        caseN = 2; //left

    t3 = length(descPrim.pos[0] - theRay.origin);
    pos_along_ray = theRay.origin + theRay.direction * t3;
    if (length(descPrim.pos[0] - pos_along_ray) < radiusB)
        caseN = 0; //central intersection

                   //get min intersection
    t = fminf(fminf(t1, t2), t3);

    float     tmin = fmaxf(0.0, t - radiusB);
    */
    float3 params = make_float3(tmin, tmax, 0.0);
    if (caseN >= 0)
    {
        params.z = 1.0;
    }
    else  params.z = 0.0; //within bounding sphere

    return params;
}
RT_PROGRAM void intersection_molecules(int primIdx)
{
    float tmin, tmax;
    tmin = 0;
    tmax = RT_DEFAULT_MAX;

    float epsilon = 0.001;//delta;
    if (epsilon > sysSceneEpsilon) epsilon = sysSceneEpsilon;
    float eps = 0.001;
    if (eps > sysSceneEpsilon) eps = sysSceneEpsilon;

    /* ------------------
    /* 1) Reading data and accessing current positions for current time
    /---------------------------------------------------------------*/

    primParamDesc descPrim = getTimeData(primIdx);

    float3 interSectParams = boundIntersection(descPrim, theRay.origin, theRay.direction);
    //if within bounding volume intersection
    //float3 interSectParams = BoundingSubVolumesIntersect(descPrim);
    if (interSectParams.z > 0.0)
    {
        tmin = interSectParams.x;
        tmax = interSectParams.y;
        float totalDistance = tmin;
        // === Raymarching (Sphere Tracing) Procedure ===

        //totalDistance = SphereTraceForward(epsilon, tmin, tmax, descPrim);

        // Found potential intersection?
        //if (totalDistance < tmax) //we found intersection
        {
            //  float totalDistance2 = SphereTraceBack(epsilon, totalDistance, tmax, descPrim);
              //------------
              //if (totalDistance2 > epsilon)
            { //it is sufficiently large subvolume to ray-cast
                if (rtPotentialIntersection(totalDistance))
                {
                    //compute normal for primitive
                    float3 x = theRay.origin + theRay.direction*totalDistance;

                    //float3 cNormal = computeNormal(eps, x, descPrim);

                    //infoH.normal = cNormal;
                    infoH.hit_point = theRay.origin + theRay.direction * (totalDistance);
                    infoH.tmin = tmin;// totalDistance;

                    infoH.desc = descPrim;
                    infoH.maxDist = tmax - tmin;// totalDistance2;
                    rtReportIntersection(MaterialIndex);
                }
            }
        }
    }
}

//bounding box
RT_PROGRAM void boundingbox_molecules(int primIdx, float result[6])
{
    const int4 idsBonds = Mols4[primIdx];

    const int2 idsB1 = Bonds[idsBonds.x];
    const int2 idsB2 = Bonds[idsBonds.y];
    const float3 pos = Positions[idsB1.x - 1];

    float rad = 1.65;
    optix::Aabb* aabb = (optix::Aabb*)result;
    //increase for ao by 5
    aabb->m_min = pos - make_float3(rad);
    aabb->m_max = pos + make_float3(rad);
}