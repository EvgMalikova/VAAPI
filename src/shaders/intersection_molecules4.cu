/*
All basic variables for SDFs visual-auditory ray-tracing
 */
#include "sdfGeometryVariables.h"

#include "sdfPrimPrograms.h"
using namespace optix;
rtDeclareVariable(float3, sysCameraPosition, , );
rtBuffer<float3>    Positions;
rtBuffer<int2>    Bonds;
rtBuffer<int4>    Mols;

rtBuffer<float>    BSRadius;
rtBuffer<int>    BSType;
rtDeclareVariable(float, MultiscaleParam, , );
rtDeclareVariable(float3, pr_pos, attribute primitive_pos, );
rtDeclareVariable(float, pr_rad, attribute primitive_rad, );
//rtDeclareVariable(int, pr_type, attribute primitive_type, );

rtDeclareVariable(float, sysSceneEpsilon, , );

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

    float3 cent = (pos1 + pos2 + pos3) / 3.0;
    f = sdfPrim5(x, pos1, pos2, pos3, pos4, rad1, rad2); //length(x-cent)-rad1;//

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
    const int4 idsBonds = Mols[primIdx];

    const int2 idsB1 = Bonds[idsBonds.x];
    const int2 idsB2 = Bonds[idsBonds.y];
    const int2 idsB3 = Bonds[idsBonds.z];
    const int2 idsB4 = Bonds[idsBonds.w];

    //idsB1.x==idsB2.x;

    //we know there are only 3 atoms - TODO:
    const float rad1 = BSRadius[idsB1.x - 1];
    const float rad2 = BSRadius[idsB1.y - 1];
    const float rad3 = BSRadius[idsB2.y - 1];

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

    //int time = int(floorf(TimeSound)); //integer part
    //int upper=int(time);

    //if (time > numFrames) time = numFrames;

    //for bond interpolation
    float3 pos2 = make_float3(0);
    float3 pos3 = make_float3(0);
    float3 pos = make_float3(0);
    float3 pos14, pos15, pos4, pos5, pos24, pos25;
    //for frames
    float3 pos12 = make_float3(0);
    float3 pos13 = make_float3(0);
    float3 pos11 = make_float3(0);

    float3 pos22 = make_float3(0);
    float3 pos23 = make_float3(0);
    float3 pos21 = make_float3(0);

    //float3 pos1 = Positions[primIdx + time*PNum]; //getting correct frame

    pos11 = Positions[idsB1.x - 1 + lower*PNum];
    pos12 = Positions[idsB1.y - 1 + lower*PNum];
    pos13 = Positions[idsB2.y - 1 + lower*PNum];
    pos14 = Positions[idsB3.y - 1 + lower*PNum];
    pos15 = Positions[idsB4.y - 1 + lower*PNum];

    pos = Positions[idsB1.x - 1 + lower*PNum];
    pos2 = Positions[idsB1.y - 1 + lower*PNum];
    pos3 = Positions[idsB2.y - 1 + lower*PNum];
    pos4 = Positions[idsB3.y - 1 + lower*PNum];
    pos5 = Positions[idsB4.y - 1 + lower*PNum];
    if (numFrames > 0) //dynamic
    {
        pos21 = Positions[idsB1.x - 1 + upper*PNum];
        pos22 = Positions[idsB1.y - 1 + upper*PNum];
        pos23 = Positions[idsB2.y - 1 + upper*PNum];
        pos24 = Positions[idsB3.y - 1 + upper*PNum];
        pos25 = Positions[idsB4.y - 1 + upper*PNum];

        pos = time*pos21 + (1.0 - time)*pos11; //time interpolation
        pos2 = time*pos22 + (1.0 - time)*pos12; //time interpolation
        pos3 = time*pos23 + (1.0 - time)*pos13; //time interpolation
        pos4 = time*pos24 + (1.0 - time)*pos14; //time interpolation
        pos5 = time*pos25 + (1.0 - time)*pos15; //time interpolation
    }

    primParamDesc descPrim;

    descPrim.type = 5; //sphere data type
    descPrim.pos[0] = pos2;
    descPrim.pos[1] = pos3;
    descPrim.pos[2] = pos4;
    descPrim.pos[3] = pos5;

    descPrim.rad[0] = rad1;
    descPrim.rad[1] = rad2;
    descPrim.rad[2] = rad3;

    descPrim.types[0] = type;
    descPrim.types[1] = type2;
    descPrim.types[2] = type3;

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
    const float3 cent = (descPrim.pos[2] + descPrim.pos[1] + descPrim.pos[0] + descPrim.pos[3]) / 4;
    const float delta = fmax(fmaxf(descPrim.rad[1], descPrim.rad[2]), descPrim.rad[0]);
    const float rayBoundSphere = length(cent - descPrim.pos[0]) + delta * 2 + descPrim.rad[0] * 4;//include rad*2 for vibrations movement

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

    float radiusB = length(cent - descPrim.pos[1]) / 1.0 + delta;
    // float radiusB2 = length(cent - descPrim.pos[2]) / 2 + delta;
    // float radiusB2 = length(cent - descPrim.pos[2]) / 2 + delta;

   //  float radiusB = fmaxf(radiusB1, radiusB2);

     //compute center of attraction as there may be several intersections with central part
    float t1, t2, t3, t4, t5, t6, t7, t8, t9;
    t1 = 0;
    t2 = 0;
    t3 = 0;
    t4 = 0;
    t5 = 0;

    t1 = length(descPrim.pos[1] - theRay.origin);
    pos_along_ray = theRay.origin + theRay.direction * t1;

    if (length(descPrim.pos[1] - pos_along_ray) < radiusB)
        caseN = 1; //left

    t2 = length(descPrim.pos[2] - theRay.origin);
    pos_along_ray = theRay.origin + theRay.direction * t2;
    if (length(descPrim.pos[2] - pos_along_ray) < radiusB)
        caseN = 2; //left

    t3 = length(descPrim.pos[0] - theRay.origin);
    pos_along_ray = theRay.origin + theRay.direction * t3;
    if (length(descPrim.pos[0] - pos_along_ray) < radiusB)
        caseN = 0; //central intersection

    t4 = length(descPrim.pos[3] - theRay.origin);
    pos_along_ray = theRay.origin + theRay.direction * t4;
    if (length(descPrim.pos[3] - pos_along_ray) < radiusB)
        caseN = 4; //central intersection

    t5 = length(cent - theRay.origin);
    pos_along_ray = theRay.origin + theRay.direction * t5;
    if (length(cent - pos_along_ray) < radiusB)
        caseN = 5; //central intersection

    float3 c1 = cent;
    float3 c2 = cent;
    float3 c3 = cent;
    float3 c4 = cent;
    c1.y -= 3.3 / 2;
    c2.y += 3.3 / 2;
    c3.x -= 3.3 / 2;
    c4.x += 3.3 / 2;

    t6 = length(c1 - theRay.origin);
    pos_along_ray = theRay.origin + theRay.direction * t6;
    if (length(c1 - pos_along_ray) < radiusB)
        caseN = 6; //cell2 intersection

    t7 = length(c2 - theRay.origin);
    pos_along_ray = theRay.origin + theRay.direction * t7;
    if (length(c2 - pos_along_ray) < radiusB)
        caseN = 7; //cel2 intersection

    t8 = length(c3 - theRay.origin);
    pos_along_ray = theRay.origin + theRay.direction * t8;
    if (length(c3 - pos_along_ray) < radiusB)
        caseN = 8; //cell2 intersection

    t9 = length(c4 - theRay.origin);
    pos_along_ray = theRay.origin + theRay.direction * t9;
    if (length(c4 - pos_along_ray) < radiusB)
        caseN = 9; //cel2 intersection

                   //get min intersection
    t = fminf(fminf(t1, t2), t3);
    t = fminf(t, fminf(t4, t5));
    t = fminf(t, fminf(t6, t7));
    t = fminf(t, fminf(t8, t9));

    float     tmin = fmaxf(0.0, t - radiusB);
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

    //--------------------

    //---level of detail
    float dist_cam = length(sysCameraPosition - (descPrim.pos[0] + descPrim.pos[1] + descPrim.pos[2] + descPrim.pos[3]) / 4);
    float d;
    if (dist_cam < 20.0)
    {
        float interp = (dist_cam - 10) / 10.0;
        d = optix::clamp(interp, 0.0, 1.0);
    }
    else d = 1;

    //-----------------------------

    float3 interSectParams = boundIntersection(descPrim, theRay.origin, theRay.direction);
    //if within bounding volume intersection
    //float3 interSectParams = BoundingSubVolumesIntersect(descPrim);
    if (interSectParams.z > 0.0)
    {
        tmin = interSectParams.x;
        tmax = interSectParams.y;
        float totalDistance = tmin;
        // === Raymarching (Sphere Tracing) Procedure ===

        totalDistance = SphereTraceForward(epsilon, tmin, tmax, descPrim);

        // Found potential intersection?
        if (totalDistance < tmax) //we found intersection
        {
            float totalDistance2 = SphereTraceBack(epsilon, totalDistance, tmax, descPrim);
            //------------
            if (totalDistance2 > epsilon)
            { //it is sufficiently large subvolume to ray-cast
                if (rtPotentialIntersection(totalDistance))
                {
                    //compute normal for primitive
                    float3 x = theRay.origin + theRay.direction*totalDistance;

                    float3 cNormal = computeNormal(eps, x, descPrim);

                    infoH.normal = cNormal;
                    infoH.hit_point = theRay.origin + theRay.direction * (totalDistance);
                    infoH.tmin = totalDistance;

                    infoH.desc = descPrim;
                    infoH.maxDist = totalDistance2;
                    rtReportIntersection(MaterialIndex);
                }
            }
        }
    }
}

//bounding box
RT_PROGRAM void boundingbox_molecules(int primIdx, float result[6])
{
    const int4 idsBonds = Mols[primIdx];

    const int2 idsB1 = Bonds[idsBonds.x];
    const int2 idsB2 = Bonds[idsBonds.y];
    const int2 idsB3 = Bonds[idsBonds.z];
    const int2 idsB4 = Bonds[idsBonds.w];

    const float rad1 = BSRadius[idsB1.x - 1] * 5;
    const float rad2 = BSRadius[idsB1.y - 1] * 5;

    float3 pos = Positions[idsB1.x - 1];
    float3 pos2 = Positions[idsB1.y - 1];
    float3 pos3 = Positions[idsB2.y - 1];
    float3 pos4 = Positions[idsB3.y - 1];
    float3 pos5 = Positions[idsB4.y - 1];

    float3 pos_min = fminf(pos, pos2);
    pos_min = fminf(pos_min, pos3);
    pos_min = fminf(pos_min, pos4);
    pos_min = fminf(pos_min, pos5);

    float3 pos_max = fmaxf(pos, pos2);
    pos_max = fmaxf(pos_max, pos3);
    pos_max = fmaxf(pos_max, pos4);
    pos_max = fmaxf(pos_max, pos5);
    if (numFrames > 0)
    {
        for (int i = 1; i < numFrames; i++)
        {
            pos = Positions[idsB1.x - 1 + i*PNum];
            pos2 = Positions[idsB1.y - 1 + i*PNum];
            pos3 = Positions[idsB2.y - 1 + i*PNum];
            pos4 = Positions[idsB3.y - 1 + i*PNum];
            pos5 = Positions[idsB4.y - 1 + i*PNum];

            pos_min = fminf(fminf(pos, pos2), pos_min);
            pos_min = fminf(pos_min, pos3);
            pos_min = fminf(pos_min, pos4);
            pos_min = fminf(pos_min, pos5);

            pos_max = fmaxf(fmaxf(pos, pos2), pos_max);
            pos_max = fmaxf(pos_max, pos3);
            pos_max = fmaxf(pos_max, pos4);
            pos_max = fmaxf(pos_max, pos5);
        }
    }

    float rad = fmaxf(rad1, rad2);

    optix::Aabb* aabb = (optix::Aabb*)result;
    //increase for ao by 5
    aabb->m_min = pos_min - make_float3(rad);
    aabb->m_max = pos_max + make_float3(rad);
}