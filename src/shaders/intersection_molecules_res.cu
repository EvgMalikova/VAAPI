/*
All basic variables for SDFs visual-auditory ray-tracing
 */
#include "sdfGeometryVariables.h"
#include "renderer/random_number_generators.h"
using namespace optix;
rtDeclareVariable(float3, sysCameraPosition, , );
rtBuffer<float3>    Positions;
rtBuffer<int2>    Bonds;
rtBuffer<int2>    Mols;
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

typedef rtCallableProgramId<float(float3, float3, float3, float3, float, float, float)> callT;
rtDeclareVariable(callT, sdfPrim, , );

//for dynamic staff
rtDeclareVariable(int, PNum, , );
rtDeclareVariable(int, numFrames, , );
rtDeclareVariable(float, TimeSound, , );

//------------------------------------------------------
//---intersection with dynamic molecule, use of morphing
//------------------------------------------------------

inline __device__ float3 transfer_function(int t, float d)
{
    // return TFBuffer[t];
    switch (t)
    {
    case 1: //H
        return make_float3(1, 1, 1);
        break;
    case 2: //C
        return make_float3(0.5);
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
    }
    return make_float3(0, 0, 0);
}

inline __device__ float3 vibTF(int t, float d, float3 pos)
{
    //float3 p=pos;
    switch (t)
    {
    case 1: //H
        return pos + make_float3(-0.41259998, 0, 0.538)*2.0*sin(TimeSound * M_PI_2 * 10);
        break;
    case 2: //C
        return pos;
        break;
    case 3: //N
        return pos;
        break;
    case 4: //S
        return pos;
        break;
    case 5: //O
        return pos;
        break;
    case 6: //P
        return pos;
        break;
    }
    return  pos;
}

inline __device__ float4  blendColor(float dMd, float3 x, float3 dir, float3 pos, float3 pos2, float3 pos3, float r1, float r2, float r3, float3 col1, float3 col2, float3 col3)
{
    float dNorm = 10;

    float3 vib_color[3];
    vib_color[0] = make_float3(0, 1.0, 0); //green
    vib_color[1] = make_float3(0, 0.0, 1.0); //blue
    vib_color[2] = make_float3(1, 1, 0); //yellow

    float3 wt_col = make_float3(0);
    dNorm = length(pos - pos2) + length(pos3 - pos2) + length(pos - pos3);
    dNorm /= 3;

    float d1 = (length(x - pos) - r1*(1 + dMd)) / dNorm;
    float d2 = (length(x - pos2) - r2*(1 + dMd)) / dNorm;
    float d3 = (length(x - pos3) - r3*(1 + dMd)) / dNorm;

    d1 = optix::clamp(d1, 0.01, 1.0);
    d2 = optix::clamp(d2, 0.01, 1.0);
    d3 = optix::clamp(d3, 0.01, 1.0);

    float3 color = make_float3(0);
    float tr = abs(dMd - 0.1);
    color += (1 - d1)*col1;
    color += (1 - d2)*col2;
    color += (1 - d3)*col3;
    //color += dMd*make_float3(1);
    //color*=tr;

    //dMd=1 - rep1;
    //dMd=0 -rep2
    //dMd*10

    float4 color_sample = make_float4(color.x*(1 - tr), color.y*(1 - tr), color.z*(1 - tr), tr);
    //float4 color_sample = make_float4(tr);
        //volume rendering
    float3 xx = x;
    int maxSteps = 7;
    float step = 2 * r1 / maxSteps;
    for (int i = 0; i < maxSteps; i++) {
        float d1 = (length(xx - pos) - r1);
        float d2 = (length(xx - pos2) - r2);
        float d3 = (length(xx - pos3) - r3);

        //  d1 = optix::clamp(d1, -0.9, 1.0);
        //  d2 = optix::clamp(d2, -0.9, 1.0);
        //  d3 = optix::clamp(d3, -0.9, 1.0);
        float3 col = make_float3(0);
        if (d1 < 0.1)
            col += abs(d1 / r1)*col1;
        if (d2 < 0.1)
            col += abs(d2 / r2)*col2;
        if (d3 < 0.1)
            col += abs(d3 / r3)*col3;

        //------------
        //interpolate between two models
        float3 coll = make_float3(0);
        d1 = optix::clamp(d1, -0.9, 0.0);
        d2 = optix::clamp(d2, -0.9, 0.0);
        d3 = optix::clamp(d3, -0.9, 0.0);
        coll += abs(1 - d1)*col1*abs(d1);
        coll += abs(1 - d2)*col2*abs(d2);
        coll += abs(1 - d3)*col3*abs(d3);
        //coll ;
        col = dMd*coll + (1 - dMd)*col;

        //col+=dMd*make_float3(1);

        //d1 += 0.1;
        //d2 += 0.1;
        //d3 += 0.1;
        tr = abs(1 - min(min(d1, d2), d3)) / 2;//abs(min(min(d1 / r1, d2 / r2), d3 / r3)) / 2 + 
        //tr /= 3;
        //tr=clamp(tr,0,1);
        // tr = (3 - d1 - d2 - d3) / 3;
        float4 col2 = make_float4(col.x, col.y, col.z, tr);
        // pre-multiply alpha
       // col2.w = (1 - d);
        col2.x *= col2.w;
        col2.y *= col2.w;
        col2.z *= col2.w;
        // "over" operator for front-to-back blending
        color_sample = color_sample + col2*(1.0f - color_sample.w);
        color_sample.w = tr;
        xx += theRay.direction*step;
    }

    //volume rendering
    //----------
//color_sample=make_float4(color,1.0);
    //return make_float3(color_sample.x, color_sample.y, color_sample.z);
    return color_sample;
}

RT_PROGRAM void intersection_molecules(int primIdx)
{
    float tmin, tmax;
    tmin = 0;
    tmax = RT_DEFAULT_MAX;

    const int2 idsBonds = Mols[primIdx];

    const int2 idsB1 = Bonds[idsBonds.x];
    const int2 idsB2 = Bonds[idsBonds.y];

    //idsB1.x==idsB2.x;

    //we know there are only 3 atoms - TODO:
    const float rad1 = BSRadius[idsB1.x - 1];
    const float rad2 = BSRadius[idsB1.y - 1];
    const float rad3 = BSRadius[idsB2.y - 1];

    const int type = BSType[idsB1.x - 1];
    const int type2 = BSType[idsB1.y - 1];
    const int type3 = BSType[idsB2.y - 1];

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

    pos = Positions[idsB1.x - 1 + lower*PNum];
    pos2 = Positions[idsB1.y - 1 + lower*PNum];
    pos3 = Positions[idsB2.y - 1 + lower*PNum];

    if (numFrames > 0) //dynamic
    {
        pos21 = Positions[idsB1.x - 1 + upper*PNum];
        pos22 = Positions[idsB1.y - 1 + upper*PNum];
        pos23 = Positions[idsB2.y - 1 + upper*PNum];

        pos = time*pos21 + (1.0 - time)*pos11; //time interpolation
        pos2 = time*pos22 + (1.0 - time)*pos12; //time interpolation
        pos3 = time*pos23 + (1.0 - time)*pos13; //time interpolation
    }

    //--------------------

    //---level of detail
    float dist_cam = length(sysCameraPosition - (pos + pos2 + pos3) / 3);
    float d;
    if (dist_cam < 20.0)
    {
        float interp = (dist_cam - 10) / 10.0;
        d = optix::clamp(interp, 0.0, 1.0);
    }
    else d = 1;

    //MultiscaleParam=d;

    float3 col1 = transfer_function(type, d);
    float3 col2 = transfer_function(type2, d);
    float3 col3 = transfer_function(type3, d);

    //-----------------------------

    float leng = length(pos2 - pos) + length(pos3 - pos);
    const float3 cent = (pos2 + pos3 + pos) / 3;
    //const float rad = BSRadius[primIdx];

    //const float3 pp=pos2*TimeSound+(1-TimeSound)*pos;
    //float3 pp = 0.5*pos2 + 0.5*pos; //new center
    const float t = length(cent - theRay.origin);
    const float3 pos_along_ray = theRay.origin + theRay.direction * t;
    tmax = t + leng*4.0; //to stop sphere tracing

   // if (length(pp - pos_along_ray) < length(leng) + rad)
    { //TDO: return it && rtPotentialIntersection(t)) {
        //tmin = t;
        // === Raymarching (Sphere Tracing) Procedure ===
        optix::float3 ray_direction = theRay.direction;
        optix::float3 eye = theRay.origin;
        //    eye.x -= global_t * 1.2f;
        optix::float3 x = eye;// +tmin * ray_direction;

        float epsilon = 0.002;//delta;
        if (epsilon > sysSceneEpsilon) epsilon = sysSceneEpsilon;
        float eps = 0.0001;
        if (eps > sysSceneEpsilon) eps = sysSceneEpsilon;
        float dist = 0;

        float totalDistance = 0.0;//Jitter * tea<4>(current_prd.seed, frame_number);
        int i = 0;
        bool stop = false;
        float dist1, dist2;

        while (!stop)
        {
            dist = sdfPrim(x, pos, pos2, pos3, rad1, rad2, rad3);
            // dist2 = sdfPrim(x, pos, pos3, rad1, rad3);
             //take a union of them
             //dist = min(dist1, dist2);
             // Step along the ray and accumulate the distance from the origin.
            x += abs(dist) * ray_direction;
            //dist_from_origin += dist * fudgeFactor;
            totalDistance += abs(dist);

            // Check if we're close enough or too far.
            if (dist < epsilon || totalDistance > tmax)
            {
                stop = true;
            }
            else i++;
        }

        // Found intersection?
        if (abs(dist) < epsilon)
        {
            if (rtPotentialIntersection(totalDistance))
            {
                //compute normal for primitive
               // float dx = min(sdfPrim(x + make_float3(eps, 0, 0), pos, pos2, rad1, rad2),sdfPrim(x + make_float3(eps, 0, 0), pos, pos3, rad1, rad3)) - min(sdfPrim(x - make_float3(eps, 0, 0), pos, pos2, rad1, rad2),sdfPrim(x + make_float3(eps, 0, 0), pos, pos3, rad1, rad3));
              //  float dy = min(sdfPrim(x + make_float3(0, eps, 0), pos, pos2, rad1, rad2),sdfPrim(x + make_float3( 0,eps, 0), pos, pos3, rad1, rad3)) - min(sdfPrim(x - make_float3(0, eps, 0), pos, pos2, rad1, rad2),sdfPrim(x + make_float3( 0,eps, 0), pos, pos3, rad1, rad3));
              //  float dz = min(sdfPrim(x + make_float3(0, 0, eps), pos, pos2, rad1, rad2),sdfPrim(x + make_float3( 0, 0,eps), pos, pos3, rad1, rad3)) - min(sdfPrim(x - make_float3(0, 0, eps), pos, pos2, rad1, rad2),sdfPrim(x + make_float3( 0, 0,eps), pos, pos3, rad1, rad3));

               //compute normal for primitive
                float dx = sdfPrim(x + make_float3(eps, 0, 0), pos, pos2, pos3, rad1, rad2, rad3) - sdfPrim(x - make_float3(eps, 0, 0), pos, pos2, pos3, rad1, rad2, rad3);
                float dy = sdfPrim(x + make_float3(0, eps, 0), pos, pos2, pos3, rad1, rad2, rad3) - sdfPrim(x - make_float3(0, eps, 0), pos, pos2, pos3, rad1, rad2, rad3);
                float dz = sdfPrim(x + make_float3(0, 0, eps), pos, pos2, pos3, rad1, rad2, rad3) - sdfPrim(x - make_float3(0, 0, eps), pos, pos2, pos3, rad1, rad2, rad3);

                //varNormal = normalize(make_float3(dx, dy, dz));

                //compute color

                    //vibrations for color

                   //--------level of detail continue

                float4 color = make_float4(0);
                float3 vib_color[3];
                vib_color[0] = make_float3(0, 1.0, 0); //green
                vib_color[1] = make_float3(0, 0.0, 1.0); //blue
                vib_color[2] = make_float3(1.0, 1.0, 0); //yellow

                float3 col21;
                float3 col31;

                //return mod1*d + (1.0 - d)*mod2;
                float r1 = d*rad1 / 2 + (1 - d)*rad1;
                float r2 = d*rad2 / 2 + (1 - d)*rad2;
                float r3 = d*rad3 / 2 + (1 - d)*rad3;

                float3 weights[3];

                if (d <= 1.0)
                {
                    //first vibration vector in molecule
                    float3 vib2[3];
                    float3 vib3[3];

                    vib2[0] = (pos2 - pos) / 3.5 * -cos(TimeSound * 20)*(1 - d);
                    vib3[0] = (pos3 - pos) / 3.5 * (cos(TimeSound * 20))*(1 - d);

                    vib2[1] = (pos2 - pos) / 3.5 * sin(TimeSound * 20)*(1 - d);
                    vib3[1] = (pos3 - pos) / 3.5 * (sin(TimeSound * 20))*(1 - d);

                    float3 vib_dir = pos2 - 2 * pos + pos3;

                    vib2[2] = vib_dir / 3.5 * sin(TimeSound * 20)*(1 - d);
                    vib3[2] = vib_dir / 3.5 * (sin(TimeSound * 20))*(1 - d);

                    float d_min = 0;
                    int vib_min = 0;

                    for (int i = 0; i < 2; i++)
                    {
                        float3 pp3 = pos3 + vib3[i];
                        float3 pp2 = pos2 + vib2[i];

                        col31 = d*col3 + (1 - d)*vib_color[i] *1.5;
                        col21 = d*col2 + (1 - d)*vib_color[i] * 1.5;

                        color += blendColor(d, x, theRay.direction, pos, pp2, pp3, r1, r2, r3, col1, col21, col31);
                    }
                    color;///= 3.0;
                }
                else {
                    color = blendColor(d, x, theRay.direction, pos, pos2, pos3, r1, r2, r3, col1, col2, col3);
                }

                //-----------
                //For multiscale
                       /*   float r1 = MultiscaleParam*rad1/ 2 + (1 - MultiscaleParam)*rad1 ;
                           float r2 = MultiscaleParam*rad2/2 + (1 - MultiscaleParam)*rad2 ;

                            float d1 = length(x - pos) - r1;
                            float d2 = length(x - pos2) - r2;
                            float3 color;

                            {
                                float d = abs(d1) + abs(d2);
                                color = (d1 / d)*col2 + (d2 / d)*col1;
                            }*/
                            //length sdfPrim(x, pos, pos2, rad1, rad2);

                            //for material
                            //fill attribute data for material
                float2 inf = make_float2(totalDistance, __int_as_float(type));
                info.primInfo = inf;
                info.type = 0; //don't use mapping
                info.useScalar = color;
                info.normal = normalize(make_float3(dx, dy, dz));
                info.hit_point = theRay.origin + theRay.direction * (totalDistance);
                info.types = make_int2(type, type2);

                pr_pos = pos;
                pr_rad = rad1;
                //pr_type = type;

                //write ao
                //info.ao = ao;//optix::clamp(float(1.0 - ao), float(0.), float(1.0));

                rtReportIntersection(MaterialIndex);
            }
        }
    }
}

//bounding box
RT_PROGRAM void boundingbox_molecules(int primIdx, float result[6])
{
    const int2 idsBonds = Mols[primIdx];

    const int2 idsB1 = Bonds[idsBonds.x];
    const int2 idsB2 = Bonds[idsBonds.y];
    const float rad1 = BSRadius[idsB1.x - 1] * 2;
    const float rad2 = BSRadius[idsB1.y - 1] * 2;
    const float rad3 = BSRadius[idsB2.y - 1] * 2;

    float3 pos = Positions[idsB1.x - 1];
    float3 pos2 = Positions[idsB1.y - 1];
    float3 pos3 = Positions[idsB2.y - 1];

    float3 pos_min = fminf(pos, pos2);
    pos_min = fminf(pos_min, pos3);
    float3 pos_max = fmaxf(pos, pos2);
    pos_max = fmaxf(pos_max, pos3);
    if (numFrames > 0)
    {
        for (int i = 1; i < numFrames; i++)
        {
            pos = Positions[idsB1.x - 1 + i*PNum];
            pos2 = Positions[idsB1.y - 1 + i*PNum];
            pos3 = Positions[idsB2.y - 1 + i*PNum];

            pos_min = fminf(fminf(pos, pos2), pos_min);
            pos_min = fminf(pos_min, pos3);

            pos_max = fmaxf(fmaxf(pos, pos2), pos_max);
            pos_max = fmaxf(pos_max, pos3);
        }
    }

    float rad = fmaxf(rad1, rad2);
    rad = fmaxf(rad, rad3);
    optix::Aabb* aabb = (optix::Aabb*)result;
    //increase for ao by 5
    aabb->m_min = pos_min - make_float3(rad);
    aabb->m_max = pos_max + make_float3(rad);
}