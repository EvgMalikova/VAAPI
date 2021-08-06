/*
All basic variables for SDFs visual-auditory ray-tracing
*/
#include "sdfGeometryVariables.h"

using namespace optix;
rtDeclareVariable(float3, sysCameraPosition, , );
rtBuffer<float3>    Positions;
rtBuffer<int2>    Bonds;
rtBuffer<float>    BSRadius;
rtBuffer<int>    BSType;
rtDeclareVariable(float, MultiscaleParam, , );

rtDeclareVariable(float, sysSceneEpsilon, , );

//for dynamic staff
rtDeclareVariable(int, PNum, , );
rtDeclareVariable(int, numFrames, , );
rtDeclareVariable(float, TimeSound, , );
//-----------------

//for ray-casting approach structure
typedef rtCallableProgramX<float3(primParamDesc, float3, float3)> callBoundT;
rtDeclareVariable(callBoundT, boundIntersection, , );

typedef rtCallableProgramX<primParamDesc(int)> callReadDataT;
rtDeclareVariable(callReadDataT, getTimeData, , );

//for sphere tracing of various primitives
//for SDF
typedef rtCallableProgramId<float(float3, float3, float)> callTSp;
rtDeclareVariable(callTSp, sdfPrimSp, , );
typedef rtCallableProgramId<float(float3, float3, float3, float3, float3)> callT1;
rtDeclareVariable(callT1, sdfPrim1, , );

typedef rtCallableProgramId<float(float3, float3, float3, float, float)> callT2;
rtDeclareVariable(callT2, sdfPrim2, , );

typedef rtCallableProgramId<float(float3, float3, float3, float3, float, float, float)> callT3;
rtDeclareVariable(callT3, sdfPrim3, , );

inline __device__ float evaluateSDF(float3 x, primParamDesc descPrim)
{
    int type = descPrim.type;
    float f = 10000.0;
    /* switch (type) {
    case 0: //sphere type
    {
    float3 pos = descPrim.pos[0];
    float rad1 = descPrim.rad[0];
    f = sdfPrim0(x, pos, rad1);
    break;
    }

    case 1://tetra type
    {
    float3 pos = descPrim.pos[0];
    float3 pos2 = descPrim.pos[1];
    float3 pos3 = descPrim.pos[2];
    float3 pos4 = descPrim.pos[3];
    f = sdfPrim1(x, pos, pos2, pos3, pos4);

    break;
    }
    case 2:
    {
    float3 pos = descPrim.pos[0];
    float3 pos2 = descPrim.pos[1];
    float rad1 = descPrim.rad[0];
    float rad2 = descPrim.rad[1];
    f = sdfPrim2(x, pos, pos2, rad1, rad2);

    break;
    }
    case 3:
    {
    float3 pos = descPrim.pos[0];
    float3 pos2 = descPrim.pos[1];
    float3 pos3 = descPrim.pos[2];
    float rad1 = descPrim.rad[0];
    float rad2 = descPrim.rad[1];
    float rad3 = descPrim.rad[2];
    f = sdfPrim3(x, pos, pos2, pos3, rad1, rad2, rad3);

    break;
    }
    }*/
    float3 pos = descPrim.pos[0];
    float rad1 = descPrim.rad[0];
    f = sdfPrimSp(x, pos, rad1);
    return f;
}

inline __device__ float3 computeNormal(float eps, float3 x, primParamDesc descPrim)
{
    /*float3 pos = descPrim.pos[0];
    float3 pos2 = descPrim.pos[1];
    float rad1 = descPrim.rad[0];
    float rad2 = descPrim.rad[1];*/

    float dx = evaluateSDF(x + make_float3(eps, 0, 0), descPrim) - evaluateSDF(x - make_float3(eps, 0, 0), descPrim);
    float dy = evaluateSDF(x + make_float3(0, eps, 0), descPrim) - evaluateSDF(x - make_float3(0, eps, 0), descPrim);
    float dz = evaluateSDF(x + make_float3(0, 0, eps), descPrim) - evaluateSDF(x - make_float3(0, 0, eps), descPrim);

    return normalize(make_float3(dx, dy, dz));
}

inline __device__ float SphereTraceForward(float epsilon, float t, float tmax, primParamDesc descPrim)
{
    optix::float3 ray_direction = theRay.direction;
    optix::float3 x = theRay.origin + theRay.direction*t;

    float dist;

    //TODO: getType

    /*float3 pos = descPrim.pos[0];
    float3 pos2 = descPrim.pos[1];
    float rad1 = descPrim.rad[0];
    float rad2 = descPrim.rad[1];*/

    float totalDistance = t;
    int i = 0;
    bool stop = false;
    while (!stop)
    {
        dist = evaluateSDF(x, descPrim);// sdfPrim(x, pos, pos2, rad1, rad2);

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
        //dist = sdfPrim(x, pos, pos2, rad1, rad2);
        dist = evaluateSDF(x, descPrim);// sdfPrim(x, pos, pos2, rad1, rad2);

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

//------------------------------------------------------
//---intersection with dynamic molecule, use of morphing
//------------------------------------------------------

inline __device__ float3 transfer_function(int t, float d)
{
    // return TFBuffer[t];
    switch (t)
    {
    case 1: //H
        return make_float3(1, 0, 0);
        break;
    case 2: //C
        return make_float3(0, 0, 1);// 0.5);
        break;
    case 3: //N
        return make_float3(0, 0, 0.5);
        break;
    case 4: //S
        return make_float3(1, 1, 0);
        break;
    case 5: //O
        return make_float3(1, 0, d);
        break;
    case 6: //P
        return make_float3(1, 0.5, 0);
        break;
    case 7: //Si
        return make_float3(1, 1, 1);//255/236, 255/246, 0);
        break;
    }
    return make_float3(0, 0, 0);
}

/*---------------

----------------*/
RT_CALLABLE_PROGRAM primParamDesc ReadData(int primIdx)
{
    const int2 ids = Bonds[primIdx];
    const float rad1 = BSRadius[ids.x - 1];
    const float rad2 = BSRadius[ids.y - 1];

    const int type = BSType[ids.x - 1];
    const int type2 = BSType[ids.y - 1];

    //---------------
    int lower = int(floorf(TimeSound));
    int upper = int(ceilf(TimeSound));

    float timeS = TimeSound;
    if (upper > numFrames) upper = numFrames;

    float time = timeS - float(lower);

    //int time = int(floorf(TimeSound)); //integer part
    //int upper=int(time);

    //if (time > numFrames) time = numFrames;

    //for bond interpolation
    float3 pos2 = make_float3(0);
    float3 pos = make_float3(0);

    //for frames
    float3 pos12 = make_float3(0);
    float3 pos11 = make_float3(0);

    float3 pos22 = make_float3(0);
    float3 pos21 = make_float3(0);

    //float3 pos1 = Positions[primIdx + time*PNum]; //getting correct frame

    pos11 = Positions[ids.x - 1 + lower*PNum];
    pos12 = Positions[ids.y - 1 + lower*PNum];

    pos = Positions[ids.x - 1 + lower*PNum];
    pos2 = Positions[ids.y - 1 + lower*PNum];

    if (numFrames > 0) //dynamic
    {
        pos21 = Positions[ids.x - 1 + upper*PNum];
        pos22 = Positions[ids.y - 1 + upper*PNum];

        pos = time*pos21 + (1.0 - time)*pos11; //time interpolation
        pos2 = time*pos22 + (1.0 - time)*pos12; //time interpolation
    }
    primParamDesc descPrim;

    descPrim.type = 2;
    descPrim.pos[0] = pos;
    descPrim.pos[1] = pos2;
    descPrim.rad[0] = rad1;
    descPrim.rad[1] = rad2;
    descPrim.types[0] = type;
    descPrim.types[1] = type2;

    return descPrim;
}

RT_CALLABLE_PROGRAM  float3 BVInt(primParamDesc  descPrim, float3 origin, float3 direction)
{
    const float3 cent = (descPrim.pos[1] + descPrim.pos[0]) / 2;

    float leng = length(descPrim.pos[1] - cent);
    const float t = length(cent - origin);
    const float3 pos_along_ray = origin + direction * t;

    float boundEps = 0.1; //for extradynamic delta
    float maxRad = fmaxf(descPrim.rad[0], descPrim.rad[1]);
    float traced_bound = leng + maxRad + boundEps;
    float tmax = t + traced_bound; //to stop sphere tracing

    float tt = clamp(t - traced_bound, 0.0f, t - traced_bound);
    float totalDist = tt;

    float tmin = fmaxf(0.0, t - maxRad);
    float3 params = make_float3(tmin, tmax, 0.0);
    (length(cent - pos_along_ray) < traced_bound) ? params.z = 1.0 : params.z = 0.0; //within bounding sphere

    return params;
}
/*-----------
/* Note: Influence a performance seriously if defined as callable
-------------------------/*/
RT_CALLABLE_PROGRAM  float3 BoundingSubVolumesIntersect(primParamDesc descPrim)
{
    const float3 cent = (descPrim.pos[1] + descPrim.pos[0]) / 2;
    float leng = length(descPrim.pos[1] - cent);
    const float t = length(cent - theRay.origin);
    const float3 pos_along_ray = theRay.origin + theRay.direction * t;

    float boundEps = 0.1; //for extradynamic delta
    float maxRad = fmaxf(descPrim.rad[0], descPrim.rad[1]);
    float traced_bound = leng + maxRad + boundEps;
    float tmax = t + traced_bound; //to stop sphere tracing

    float tt = clamp(t - traced_bound, 0.0f, t - traced_bound);
    float totalDist = tt;

    float tmin = fmaxf(0.0, t - maxRad);
    float3 params = make_float3(tmin, tmax, 0.0);
    (length(cent - pos_along_ray) < traced_bound) ? params.z = 1.0 : params.z = 0.0; //within bounding sphere
    /*if (length(cent - pos_along_ray) < traced_bound)
        params.z = 1.0;
    else
        params.z = 0.0;*/
    return params;
}

RT_PROGRAM void intersection_bond_mol(int primIdx)
{
    float tmin, tmax;
    tmin = 0;
    tmax = RT_DEFAULT_MAX;
    float epsilon = 0.001;//delta;
    if (epsilon > sysSceneEpsilon) epsilon = sysSceneEpsilon;
    float eps = 0.0001;
    if (eps > sysSceneEpsilon) eps = sysSceneEpsilon;

    /* ------------------
    /* 1) Reading data and accessing current positions for current time
    /---------------------------------------------------------------*/

    primParamDesc descPrim = getTimeData(primIdx);
    //--------------------

    /*
    //---level of detail simple test
    float dist_cam = length(sysCameraPosition - (pos + pos2) / 2);
    float d;
    if (dist_cam < 10.0)
    {
        float interp = dist_cam / 10.0;
        d = optix::clamp(interp, 0.0, 1.0);
    }
    else d = 1;

    //block multiscale test in current implementation
    d = 1;
    */

    //----------------------------

       /* ------------------
    /* 2) Compute intersection with constructive tree subdivisions
    /* one or several bounding spheres
    /---------------------------------------------------------------*/

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
            if (totalDistance2 > epsilon) { //it is sufficiently large subvolume to ray-cast
                if (rtPotentialIntersection(totalDistance))
                {
                    //compute normal for primitive
                    float3 x = theRay.origin + theRay.direction*totalDistance;

                    float3 cNormal = computeNormal(eps, x, descPrim);

                    //compute color

                    //color at input point
                     /* ------------------
        /* DELETE: initial color
        /---------------------------------------------------------------*/
                    float d = 1;
                    float3 pos = descPrim.pos[0];
                    float3 pos2 = descPrim.pos[1];
                    float rad1 = descPrim.rad[0];
                    float rad2 = descPrim.rad[1];
                    float3 col1 = transfer_function(descPrim.types[0], d);
                    float3 col2 = transfer_function(descPrim.types[1], d);
                    //--------level of detail continue
                    //return mod1*d + (1.0 - d)*mod2;
                    float r1 = d*rad1 / 2 + (1 - d)*rad1;
                    float r2 = d*rad2 / 2 + (1 - d)*rad2;

                    float d1 = length(x - pos) - r1;
                    float d2 = length(x - pos2) - r2;
                    float3 color1;
                    float d_l = abs(d1) + abs(d2);
                    color1 = (d1 / d_l)*col2 + (d2 / d_l)*col1;

                    //color at output point
                    x = theRay.origin + theRay.direction*(totalDistance + totalDistance2);
                    d1 = length(x - pos) - r1;
                    d2 = length(x - pos2) - r2;
                    float3 color2;
                    d_l = abs(d1) + abs(d2);
                    color2 = (d1 / d_l)*col2 + (d2 / d_l)*col1;

                    //avaraging color
                    float3 color = (color1 + color2) / 2;

                    /*------------------
                    /* Fill attributes array for material
                    -------------------------*/

                    // float2 inf = make_float2(totalDistance, __int_as_float(type));
                    // info.primInfo = inf;
                    info.type = 0; //don't use mapping
                    info.useScalar = make_float4(color.x, color.y, color.z, 1.0);
                    info.normal = cNormal;
                    info.hit_point = theRay.origin + theRay.direction * (totalDistance);
                    info.types = make_int2(descPrim.types[0], descPrim.types[1]);
                    info.pos[0] = pos;
                    info.pos[1] = pos2;
                    info.rad[0] = rad1;
                    info.rad[1] = rad2;

                    info.maxDist = totalDistance2;

                    rtReportIntersection(MaterialIndex);
                }
            }
        }
    }
}

//bounding box
RT_PROGRAM void boundingbox_bond_mol(int primIdx, float result[6])
{
    const int2 ids = Bonds[primIdx];
    const float rad1 = BSRadius[ids.x - 1];
    const float rad2 = BSRadius[ids.y - 1];

    float3 pos = Positions[ids.x - 1];
    float3 pos2 = Positions[ids.y - 1];

    float3 pos_min = fminf(pos, pos2);
    float3 pos_max = fmaxf(pos, pos2);

    if (numFrames > 0)
    {
        for (int i = 1; i < numFrames; i++)
        {
            pos = Positions[ids.x - 1 + i*PNum];
            pos2 = Positions[ids.y - 1 + i*PNum];

            pos_min = fminf(fminf(pos, pos2), pos_min);
            pos_max = fmaxf(fmaxf(pos, pos2), pos_max);
        }
    }

    float rad = fmaxf(rad1, rad2);
    optix::Aabb* aabb = (optix::Aabb*)result;
    //increase for ao by 5
    aabb->m_min = pos_min - make_float3(rad + 0.1);
    aabb->m_max = pos_max + make_float3(rad + 0.1);
}