/*

 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "../renderer/per_ray_data.h"
#include "../basic_lights.h"
 //#include "per_ray_data.h"
#include "../attributeInfo.h"
#include "../sdfPrimPrograms.h"
#include "transferFunction.h"
using namespace optix;

rtDeclareVariable(attributeInfo2, infoH, attribute infoH, );
// Context global variables provided by the renderer system.
rtDeclareVariable(rtObject, sysTopObject, , );

rtDeclareVariable(float, MultiscaleParam, , );
rtDeclareVariable(float, TimeSound, , );
// Semantic variables.
rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );
rtDeclareVariable(float, theIntersectionDistance, rtIntersectionDistance, );

rtDeclareVariable(PerRayData, thePrd, rtPayload, );

//type of rendering
rtDeclareVariable(int, Type, , );
rtDeclareVariable(int, HeteroObjType, , );

//array of lights
rtBuffer<BasicLight> lights;

rtDeclareVariable(float3, sysCameraPosition, , );

typedef rtCallableProgramId<float(float3, primParamDesc)> callM;
rtDeclareVariable(callM, evalF, , );

rtDeclareVariable(float3, sCell1, , );
rtDeclareVariable(float3, sCell2, , );
rtDeclareVariable(float3, sCell3, , );
/*
For heterogeneous objects
*/

//TODO: set as buffer
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
    case 7: //Si
        return make_float3(1, 1, 1);//255/236, 255/246, 0);
        break;
    }
    return make_float3(0, 0, 0);
}

RT_CALLABLE_PROGRAM  float eval3(float3 x, primParamDesc descPrim)
{
    int type = descPrim.type;
    float f = 10000.0;
    float3 pos1 = descPrim.pos[0];
    float3 pos2 = descPrim.pos[1];
    float3 pos3 = descPrim.pos[2];

    float rad1 = descPrim.rad[0];
    float rad2 = descPrim.rad[1];
    float rad3 = descPrim.rad[2];

    f = sdfPrim3(x, pos1, pos2, pos3, rad1, rad2, rad3);//length(x - pos1) - rad1; //sdfPrim1(x, pos1, pos2, rad1, rad2);
    return f;
}
RT_CALLABLE_PROGRAM  float eval5(float3 x, primParamDesc descPrim)
{
    int type = descPrim.type;
    float f = 10000.0;
    float3 pos1 = descPrim.pos[0];
    float3 pos2 = descPrim.pos[1];
    float3 pos3 = descPrim.pos[2];
    float3 pos4 = descPrim.pos[3];

    float rad1 = descPrim.rad[0];
    float rad2 = descPrim.rad[1];

    f = sdfPrim5(x, pos1, pos2, pos3, pos4, rad1, rad2);//length(x - pos1) - rad1; //sdfPrim1(x, pos1, pos2, rad1, rad2);
    return f;
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

    d1 = (length(x - pos) - r1);
    d2 = (length(x - pos2) - r2);
    d3 = (length(x - pos3) - r3);

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
    return make_float4(col.x, col.y, col.z, tr);
}

inline __device__ float3 InterpolateColorRad(float3 x, float3 pos, float3 pos2, float r1, float r2, float3 col1, float3 col2)
{
    float d1 = length(x - pos) - r1;
    float d2 = length(x - pos2) - r2;
    float d = length(pos - pos2);
    if (d1 <= 0) return col1;
    if (d2 <= 0) return col2;
    float3 col = d1 / d*col2 + d2 / d*col1;

    return col;
}

RT_CALLABLE_PROGRAM float3 GetColorBlend(float3 x, primParamDesc desc)
{
    //---level of detail
    float dist_cam = length(sysCameraPosition - (desc.pos[0] + desc.pos[1] + desc.pos[2]) / 3);
    float d;
    if (dist_cam < 20.0)
    {
        float interp = (dist_cam - 10) / 10.0;
        d = optix::clamp(interp, 0.0, 1.0);
    }
    else d = 1;

    //MultiscaleParam=d;

    float3 col1 = transfer_function(desc.types[0], d);
    float3 col2 = transfer_function(desc.types[1], d);
    float3 col3 = transfer_function(desc.types[2], d);

    float3 col11 = InterpolateColorRad(x, desc.pos[0], desc.pos[1], desc.rad[0] / 2, desc.rad[1] / 2, col1, col2);
    float3 col12 = InterpolateColorRad(x, desc.pos[0], desc.pos[2], desc.rad[0] / 2, desc.rad[2] / 2, col1, col3);

    float3 dir = normalize(x - desc.pos[0]);
    float3 dir2 = normalize(desc.pos[1] - desc.pos[0]);
    float3 dir3 = normalize(desc.pos[2] - desc.pos[0]);
    float cos1 = abs(dot(dir2, dir));
    float cos2 = abs(dot(dir3, dir));

    float d1 = length(x - desc.pos[1]);
    float d2 = length(x - desc.pos[2]);
    float td = length(desc.pos[1] - desc.pos[2]);
    //float cos2=normalize(dot(desc.pos[2] - desc.pos[0], dir));
    float3 colB1 = col12*d1 / td + col11*d2 / td;
    float3 colB2 = make_float3(0);

    //return col;

    //--------level of detail continue
    float rad1 = desc.rad[0];
    float rad2 = desc.rad[1];
    float rad3 = desc.rad[2];
    float4 color = make_float4(0);
    float3 vib_color[3];
    vib_color[0] = make_float3(0, 1.0, 0); //green
    vib_color[1] = make_float3(0, 0.0, 1.0); //blue
    vib_color[2] = make_float3(1.0, 1.0, 0); //yellow

   // float3 col21;
   // float3 col31;

    float weights[3];

    float3 resCol = colB1;

    //block currently for cells
    if (desc.type != 5) {
        if (d <= 1.0)
        {
            float3 pos = desc.pos[0];
            float3 pos2 = desc.pos[1];
            float3 pos3 = desc.pos[2];
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

            int l = 0;
            float dmax = 0;
            float dmin = 100;
            float dminprev = 100;

            float num = 3;
            if (MultiscaleParam <= 1)  num = 1;
            else
            {
                if (MultiscaleParam <= 2)
                    num = 2;
            }

            for (int i = 0; i < num; i++)
            {
                float3 pp3 = pos3 + vib3[i];
                float3 pp2 = pos2 + vib2[i];
                float d1 = length(x - pp2);
                float d2 = length(x - pp3);

                dmin = fminf(fminf(d1, dmin), d2);
                if (dmin < dminprev) {
                    l = i;
                    dminprev = dmin;
                }
                dmax = fmaxf(fmaxf(d1, dmin), d2);
            }
            weights[0] = 0.1;
            weights[1] = 0.1;
            weights[2] = 0.1;
            weights[l] = 1.5;

            float3 cols[3];
            for (int j = 0; j < num; j++)
            {
                cols[j] = make_float3(0);
                //Get current position
                float3 pp3 = pos3 + vib3[j];
                float3 pp2 = pos2 + vib2[j];

                float3 col21 = InterpolateColorRad(x, desc.pos[0], pp2, desc.rad[0] / 2, desc.rad[1] / 2, col1, vib_color[j]);
                float3 col31 = InterpolateColorRad(x, desc.pos[0], pp3, desc.rad[0] / 2, desc.rad[2] / 2, col1, vib_color[j]);

                //col31 = d*col3 + (1 - d)*vib_color[i] * 1.5;
                //col21 = d*col2 + (1 - d)*vib_color[i] * 1.5;

                float3 dir = normalize(x - desc.pos[0]);
                float3 dir2 = normalize(pp2 - desc.pos[0]);
                float3 dir3 = normalize(pp3 - desc.pos[0]);
                cos1 = dot(dir2, dir);
                float cos2 = dot(dir3, dir);
                //float cos2=normalize(dot(desc.pos[2] - desc.pos[0], dir));
                colB2 = (1 - cos2)*col31 + (1 - cos1)*col21;

                bool interpolate = true;
                float d1 = length(x - pp2);// -rad2 / 4;
                float d2 = length(x - pp3);// -rad3 / 4;
                if (d1 <= 0) {
                    cols[j] = col21;
                    interpolate = false;
                }
                if (d2 <= 0) {
                    cols[j] = col31;
                    interpolate = false;
                }

                if (interpolate) {
                    float dt = length(pp2 - pp3);

                    float dmin1 = fminf(d1, d2);
                    cols[j] = d1 / dt*col31 + d2 / dt*col21;
                    weights[j] = dmin / (dmin1*dmin1);
                }
                else {
                    //d1 = length(x - pp2);
                    //d2 = length(x - pp3);
                    weights[j] = 1.0;// dmin / fminf(d1, d2);
                }

                // colB2 += colB21;
                 //color += blendColor(d, x, theRay.direction, pos, pp2, pp3, r1, r2, r3, col1, col21, col31);
            }
            for (int j = 0; j < num; j++)
            {
                colB2 += weights[j] * cols[j];
            }
            colB2 /= 3.0;

            resCol = d*colB1 + (1 - d)*colB2;
        }
    }
    return resCol;
}

__device__
inline float  plane(float3 p, float3 c, float3 n)
{
    return optix::dot(p - c, n);
}
__device__
inline float3 getNormal(float3 v1, float3 v2, float3 v3, float3 c, float3 ct)
{
    float3 a = v3 - v2;
    float3 b = v1 - v2;
    float3 n = cross(a, b);

    float3 nt = c - ct;

    //normalize(n);
    //normalize(nt);

    n = n*dot(n, nt);

    return normalize(n);
}
__device__
inline float3 getCenter(float3 p1, float3 p2, float3 p3)

{
    float3 center = (p1 + p2 + p3) / 3.0;
    return center;
}

__device__
inline float3 getCenterTetra(float4 p0, float4 p1, float4 p2, float4 p3)

{
    float3 center = make_float3((p0 + p1 + p2 + p3) / 4.0);
    return center;
}

RT_CALLABLE_PROGRAM float3 GetColorBlendCell(float3 x, primParamDesc desc)
{
    //---level of detail
    float dist_cam = length(sysCameraPosition - (desc.pos[0] + desc.pos[1] + desc.pos[2]) / 3);
    float d;
    if (dist_cam < 20.0)
    {
        float interp = (dist_cam - 10) / 10.0;
        d = optix::clamp(interp, 0.0, 1.0);
    }
    else d = 1;

    float3 a = (desc.pos[0] + desc.pos[1] + desc.pos[2] + desc.pos[3]) / 4.0;

    //--------
    float cel1Rad = 3.3 * 2;// 1.65 + 3.3;
    float cel1Rad2 = 3.3;

    float t = clamp(TimeSound, 2.0, 4.0) - 1.0;
    float3 cCell3 = sCell3 - make_float3(3.3*t, 3.3, 3.3);
    float3 cCell2 = sCell2 + make_float3(3.3*t, 3.3, 3.3*t);
    float3 cCell1 = sCell1 - make_float3(3.3, 3.3*t, 3.3*t);

    int comp1 = 0;
    float3 addCol = make_float3(0);

    if ((length(x - cCell1) - cel1Rad) <= 0)
        addCol = make_float3(0.5, 0.5, 0);
    if ((length(x - cCell2) - cel1Rad2) <= 0)
        addCol = make_float3(0, 0.5, 0);
    if ((length(x - cCell3) - cel1Rad) <= 0)
        addCol = make_float3(0, 0, 0.5);

    addCol *= clamp(TimeSound, 0.0, 1.0f);

    //MultiscaleParam=d;

    float3 colE1 = transfer_function(desc.types[0], d);
    float3 colE2 = transfer_function(desc.types[1], d);

    float3 v0 = desc.pos[0];
    float3 v1 = desc.pos[1];
    float3 v2 = desc.pos[2];
    float3 v3 = desc.pos[3];

    float r = desc.rad[1];
    float radBB = length(a - desc.pos[0]) + r / 2;
    if (length(x - a) >= radBB) {
        // col10 = colE1;
        // col11 = colE1;
        // col12 = colE1;
        // col13 = colE1;

        return colE2;
    }

    float3 p0 = v0 - x;
    float d0 = length(p0);
    float3 p1 = v1 - x;
    float d1 = length(p1);
    float3 p2 = v2 - x;
    float d2 = length(p2);
    float3 p3 = v3 - x;
    float d3 = length(p3);

    float dmin = min(d0, min(d1, (min(d2, d3))));
    float3 col;
    if (d0 == dmin)
        col = InterpolateColorRad(x, a, desc.pos[0], desc.rad[0] / 2, desc.rad[1] / 2, colE1, colE2);
    if (d1 == dmin)
        col = InterpolateColorRad(x, a, desc.pos[1], desc.rad[0] / 2, desc.rad[1] / 2, colE1, colE2);

    if (d2 == dmin)
        col = InterpolateColorRad(x, a, desc.pos[2], desc.rad[0] / 2, desc.rad[1] / 2, colE1, colE2);

    if (d3 == dmin)
        col = InterpolateColorRad(x, a, desc.pos[3], desc.rad[0] / 2, desc.rad[1] / 2, colE1, colE2);

    /*float3 col11 = InterpolateColorRad(x, a, desc.pos[1], desc.rad[0], desc.rad[1], colE1, colE2);
    float3 col12 = InterpolateColorRad(x, a, desc.pos[2], desc.rad[0], desc.rad[1], colE1, colE2);
    float3 col13 = InterpolateColorRad(x, a, desc.pos[3], desc.rad[0], desc.rad[1], colE1, colE2);
    float3 col10 = InterpolateColorRad(x, a, desc.pos[0], desc.rad[0], desc.rad[1], colE1, colE2);
    */

    // float3 colB1 = (col11 / d1 + col10 / d0 + col13 / d3 + col12 / d2) / (d1 + d0 + d3 + d2);

    return col + addCol;
    /*
    float3 c0 = getCenter(v0, v2, v1);
    float3 c1 = getCenter(v0, v3, v2);
    float3 c2 = getCenter(v1, v3, v0);
    float3 c3 = getCenter(v1, v2, v3);

    float3 ct = (v0 + v1 + v2 + v3) / 4.0f;
    //float rad1 = length(ct - c0);
    float rad = length(ct - v0);
    //rad = (rad + rad1) / (2.0*t);
    float3 n0 = getNormal(v0, v2, v1, c0, ct);
    float3 n1 = getNormal(v0, v3, v2, c1, ct);
    float3 n2 = getNormal(v1, v3, v0, c2, ct);
    float3 n3 = getNormal(v1, v2, v3, c3, ct);

    float3 col0 = computeColTriangle(x, v0, v1, v2, col10, col11, col12, n0);
    float3 col1 = computeColTriangle(x, v0, v2, v3, col10, col12, col13, n1);
    float3 col2 = computeColTriangle(x, v1, v0, v3, col11, col10, col13, n2);
    float3 col3 = computeColTriangle(x, v1, v2, v3, col11, col12, col13, n3);

    float d0 = abs(plane(x, c0, n0));
    float d1 = abs(plane(x, c1, n1));
    float d2 = abs(plane(x, c2, n2));
    float d3 = abs(plane(x, c3, n3));

    float dmax = max(d0, max(d1, (max(d2, d3))));
    float dmin = min(d0, min(d1, (min(d2, d3))));

    if (d0 >= 0.001) return col0;
    if (d1 >= 0.001) return col1;
    if (d2 >= 0.001) return col2;
    if (d3 >= 0.001) return col3;
    */

    //    float3 colB1 = (col11 / d1 + col10 / d0 + col13 / d3 + col12 / d2) / (d1 + d0 + d3 + d2);

      //  return colB1;

        /*
        float d01 = length(d0*n0 - d1*n1);
        float d23 = length(d2*n2 - d3*n3);
        float d03 = length(d0*n0 - d3*n3);

        if (length )
        float3  colB1 = col1*d0 / (d01)+col0*d1 / (d01);
        float3 colB2 = col3*d2 / (d23)+col2*d3 / (d23);
        colB1 = colB1*d3 / d03 + colB2*d0 / d03;
        return colB1;*/
        /* float3 p0 = v0 - x;
        float d0 = length(p0 - n0);
        float3 p1 = v1 - x;
        float d1 = length(p1 - n1);
        float3 p2 = v2 - x;
        float d2 = length(p2 - n2);
        float3 p3 = v3 - x;
        float d3 = length(p3 - n3);

        //  triangle interp
        float dmax = max(d0, max(d1, (max(d2, d3))));

            float3 colB1 = col1*(1 - d1 / dmax) + col0*(1 - d0 / dmax) + col2*(1 - d2 / dmax) + col3*(1 - d3 / dmax);
            */
            /*
            float3 p0 = v0 - x;
            float d0 = length(p0);
            float3 p1 = v1 - x;
            float d1 = length(p1);
            float3 p2 = v2 - x;
            float d2 = length(p2);
            float3 p3 = v3 - x;
            float d3 = length(p3);
            float dmax = max(d0, max(d1, (max(d2, d3))));
            float dmin = min(d0, min(d1, (min(d2, d3))));
            float3 colB1 = make_float3(1, 1, 1);*/
            /* if (d0 == 0) return col0;
             if (d1 == 0) return col1;
             if (d2 == 0) return col2;
             if (d3 == 0) return col3;

             // colB1 = col1*(dmin / d1) + col0*(dmin / d2) + col2*(dmin / d2) + col3*(dmin / d3);
             // colB1 /= 4.0;

             float3  colB1 = col1*d0 / (d1 + d0) + col0*d1 / (d1 + d0);
             float3 colB2 = col3*d0 / (d3 + d0) + col0*d3 / (d3 + d0);
             float3 colB3 = colB1*d1 / (d3 + d1) + colB2*d3 / (d3 + d1);

             colB1 = col1*d2 / (d1 + d2) + col2*d1 / (d1 + d2);
             colB2 = col3*d2 / (d3 + d2) + col2*d3 / (d3 + d2);
             float3 colB4 = colB1*d1 / (d3 + d1) + colB2*d3 / (d3 + d1);

             colB1 = (colB3 + colB4) / 2;

             return colB3;*/
}

inline __device__  float4  GetColor(float3 x)
{
    //---level of detail
    float dist_cam = length(sysCameraPosition - (infoH.desc.pos[0] + infoH.desc.pos[1] + infoH.desc.pos[2]) / 3);
    float d;
    if (dist_cam < 20.0)
    {
        float interp = (dist_cam - 10) / 10.0;
        d = optix::clamp(interp, 0.0, 1.0);
    }
    else d = 1;

    //MultiscaleParam=d;

    float3 col1 = transfer_function(infoH.desc.types[0], d);
    float3 col2 = transfer_function(infoH.desc.types[1], d);
    float3 col3 = transfer_function(infoH.desc.types[2], d);

    //--------level of detail continue
    float rad1 = infoH.desc.rad[0];
    float rad2 = infoH.desc.rad[1];
    float rad3 = infoH.desc.rad[2];
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
        float3 pos = infoH.desc.pos[0];
        float3 pos2 = infoH.desc.pos[1];
        float3 pos3 = infoH.desc.pos[2];
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

            col31 = d*col3 + (1 - d)*vib_color[i] * 1.5;
            col21 = d*col2 + (1 - d)*vib_color[i] * 1.5;

            color += blendColor(d, x, theRay.direction, pos, pp2, pp3, r1, r2, r3, col1, col21, col31);
        }
        color;///= 3.0;
    }
    else {
        color = blendColor(d, x, theRay.direction, infoH.desc.pos[0], infoH.desc.pos[1], infoH.desc.pos[2], r1, r2, r3, col1, col2, col3);
    }

    return color;
}

__device__ void render_HeteroVolume(float3 normal, float3 hit_point)
{
    float Ka = 0.5;
    float Kd = 0.9;
    float Ks = 0.9;

    float4 col = make_float4(0, 0, 0, 1);// translucent_grays(0.5, 0.1, 0);

    float tstep = 0.1;
    float3 pos = hit_point;// eyeRay.o + eyeRay.d*tnear;
    float3 step = theRay.direction*tstep;

    float4 sum = thePrd.result;// make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here
    float trp = 0.05;
    float trp0 = 0.1;

    float s1 = evalF(pos, infoH.desc);
    if (abs(s1) > tstep)
        step = theRay.direction*abs(s1);

    float i = 0.0;
    float max = thePrd.maxDist;// *2 + 0.4; //bounding box size
    float4 sumcol = make_float4(0.0);
    float tracedDist = 0;

    //float4 col1 = translucent_grays(0.5, 0.01, 0);
    int VolInt = 1;

    while (i < max) //s2 < 0.01)
    {
        if (s1 < tstep / 2)
        {
            // if (abs(s1) > tstep) //sum transparency
            {
                VolInt = 1;
                //------------------
                                       //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;
                col = GetColor(pos);

                //Beer–Lambert law
                float F = exp(-trp*abs(s1) * 200);
                col = col*(1.0 - F);
                sum = sum + col*(1.0f - sum.w);
            }
        }

        if (abs(s1) > tstep) {
            step = theRay.direction*abs(s1);
            i += abs(s1);
        }
        else
        {
            i += tstep;
            step = theRay.direction*tstep;
        }

        pos += step;
        if (sum.w >= 1.0) {
            i = max + 1;
        }
        else
            s1 = evalF(pos, infoH.desc);
    }

    thePrd.result = sum;
}
/* Compiles various types of programs depending on primType*/
RT_PROGRAM void volume_hetero_close()
{
    float3 normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, infoH.normal));
    optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;
    thePrd.maxDist = infoH.maxDist;
    thePrd.normal = normal;
    thePrd.last_hit_point = hit_point;

    thePrd.result = GetColor(hit_point);
    // render_HeteroVolume(normal, hit_point); //for defalt rendering primitive

    thePrd.renderType = 0;
    thePrd.depth++;

    if (thePrd.totalDist > 20.0) {//TODO set max trace depth as parameter
        thePrd.depth = 10;
    }

    thePrd.totalDist += length(theRay.origin - hit_point);
    /*if (thePrd.result.w < 0.5) {
        if (thePrd.depth < 5) //TODO set max trace depth as parameter
        {
            //further ray
            optix::Ray ray = optix::make_Ray(hit_point + theRay.direction *(abs(infoH.maxDist) + 0.1), theRay.direction, 0, 0.0f, RT_DEFAULT_MAX);
            rtTrace(sysTopObject, ray, thePrd);
        }
    }*/
}

RT_PROGRAM void volume_hetero_any()
{
    //No volume integration
    //just saving preliminary data to array
    thePrd.renderType = 3; //for postprocessing
    if (thePrd.cur_prim < MAX_PRIM_ALONG_RAY)
    { //push intersections
        thePrd.cur_prim++;
        cellPrimDesc cell;
        cell.intersectionDist = infoH.tmin;
        cell.type = 3; //tracing with primitives and compute bounds
        cell.normal = infoH.normal;
        int typeC = int(infoH.desc.rad[0]);
        //float3 col = GetColor(infoH.hit_point);

        //thePrd.result += make_float4(col.x, col.y, col.z, 0.1);
        cell.color = GetColor(infoH.hit_point);
        cell.color.w = 0.009;
        cell.maxDist = infoH.maxDist;

        thePrd.cellPrimitives[thePrd.cur_prim - 1] = cell;
        thePrd.prims[thePrd.cur_prim - 1] = infoH.desc;
        rtIgnoreIntersection();
    }
    else {
        rtTerminateRay();
    }
}