/*
 */

#include "default.h"
#include "../../basic_lights.h"
#include "../../materials/transferFunction.h"

 //rtDeclareVariable(float, TimeSound, , );
 //for sdf
typedef rtCallableProgramId<float(float3, float3)> callTBackSDF;
rtDeclareVariable(callTBackSDF, sdfPrimBack, , );

rtDeclareVariable(float3, bbox_min, , );
rtDeclareVariable(float3, bbox_max, , );
rtDeclareVariable(float3, bbox_center, , );

rtDeclareVariable(float3, sCell1, , );
rtDeclareVariable(float3, sCell2, , );
rtDeclareVariable(float3, sCell3, , );

typedef rtCallableProgramId<float(float3, primParamDesc)> callM;
rtDeclareVariable(callM, evalF, , );

typedef rtCallableProgramId<float3(float3, primParamDesc)> callC;
rtDeclareVariable(callC, evalCol, , );
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

__device__
inline float smaxp(float a, float b, float k)
{
    float h = optix::max(k - abs(a - b), 0.0) / k;
    return optix::max(a, b) - h*h*k*(1.0 / 4.0);
}
__device__
inline float sminp(float a, float b, float k)
{
    float h = optix::max(k - abs(a - b), 0.0) / k;
    return optix::min(a, b) - h*h*k*(1.0 / 4.0);
}
__device__
inline float bond(float3 p, float3 a, float3 b, float r)
{
    float3 pa = p - a;
    float3 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba*h) - r;
}
__device__
inline float Tetra(float3 p, float3 v0, float3 v1, float3 v2, float3 v3)
{
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

    float a = plane(p, c0, n0);
    float b = plane(p, c1, n1);
    float c = plane(p, c2, n2);
    float d = plane(p, c3, n3);
    return fmaxf(fmaxf(a, b), fmaxf(c, d));
}
__device__
inline float TetraWire(float3 p, float3 v0, float3 v1, float3 v2, float3 v3)
{
    float f1 = bond(p, v0, v1, 0.4);
    float f2 = bond(p, v0, v2, 0.4);
    float f3 = bond(p, v0, v3, 0.4);
    float f4 = bond(p, v1, v2, 0.4);
    float f5 = bond(p, v1, v3, 0.4);

    f1 = sminp(f2, f1, 0.4);
    f1 = sminp(f3, f1, 0.4);
    f1 = sminp(f4, f1, 0.4);
    f1 = sminp(f5, f1, 0.4);
    return f1;
}

__device__
inline float TetraWire2(float3 p, float3 v0, float3 v1, float3 v2, float3 v3)
{
    float3 ct = (v0 + v1 + v2 + v3) / 4.0f;
    float3 c0 = getCenter(v0, v2, v1);
    float3 c1 = getCenter(v0, v3, v2);
    float3 c2 = getCenter(v1, v3, v0);
    float3 c3 = getCenter(v1, v2, v3);
    float r = length(ct - v0) / 20;

    float f1 = bond(p, ct, c1, r);
    float f2 = bond(p, ct, c2, r);
    float f3 = bond(p, ct, c3, r);
    float f4 = bond(p, ct, c0, r);

    f1 = sminp(f2, f1, 0.8);
    f1 = sminp(f3, f1, 0.8);
    f1 = sminp(f4, f1, 0.8);
    return f1;
}
__device__
inline float3 computeColTriangle(float3 p, float3 v0, float3 v1, float3 v2, float3 col10, float3 col11, float3 col12, float3 n0)
{
    float3 c0 = (v0 + v1 + v2) / 3;
    float dn = abs(plane(p, c0, n0));

    float3 p0 = v0 - p;
    float d0 = length(p0 - n0*dn);
    float3 p1 = v1 - p;
    float d1 = length(p1 - n0);

    float td = length(v0 - v1);
    //float cos2=normalize(dot(desc.pos[2] - desc.pos[0], dir));
    float3 colB1 = col10*d1 / td + col11*d0 / td;

    float3 p2 = v2 - p;
    float d2 = length(p2 - n0);

    float td2 = length(v0 - v2);
    float3 colB2 = col10*d2 / td2 + col12*d0 / td2;

    float td3 = length(v1 - v2);
    colB1 = colB1*d2 / td3 + colB2*d1 / td3;
    return colB1;
}
__device__
inline float computeTrTriangle(float3 p, float3 v0, float3 v1, float3 v2, float col10, float col11, float col12, float3 n0)
{
    float3 c0 = (v0 + v1 + v2) / 3;
    float dn = abs(plane(p, c0, n0));

    float3 p0 = v0 - p;
    float d0 = length(p0 - n0*dn);
    float3 p1 = v1 - p;
    float d1 = length(p1 - n0);

    float td = length(v0 - v1);
    //float cos2=normalize(dot(desc.pos[2] - desc.pos[0], dir));
    float colB1 = col10*d1 / td + col11*d0 / td;

    float3 p2 = v2 - p;
    float d2 = length(p2 - n0);

    float td2 = length(v0 - v2);
    float colB2 = col10*d2 / td2 + col12*d0 / td2;

    float td3 = length(v1 - v2);
    colB1 = colB1*d2 / td3 + colB2*d1 / td3;
    return colB1;
}

inline __device__  float3  GetColor(float3 x, float3 v0, float3 v1, float3 v2, float3 v3, float s0, float s1, float s2, float s3)
{
    float3 col10;
    float3 col11;
    float3 col12;
    float3 col13;
    float scale = 400;
    float4 col = translucent_grays(s0 / scale, s0 / scale, 1); //abs(f / 30), 1);////translucent_grays2(color_id, abs(f / 100));
    col10 = make_float3(col.x, col.y, col.z);
    col = translucent_grays(s1 / scale, s1 / scale, 1);
    col11 = make_float3(col.x, col.y, col.z);

    col = translucent_grays(s2 / scale, s2 / scale, 1);
    col12 = make_float3(col.x, col.y, col.z);

    col = translucent_grays(s3 / scale, s3 / scale, 1);
    col13 = make_float3(col.x, col.y, col.z);

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
    float3 colB1 = (col1 / d1 + col0 / d0 + col3 / d3 + col2 / d2) / (d1 + d0 + d3 + d2);

    return colB1;
}

inline __device__  float  GetIso(float3 x, float3 v0, float3 v1, float3 v2, float3 v3, float s0, float s1, float s2, float s3)
{
    float4 col;

    float scale = 450;
    col = make_float4(s0, s1, s2, s3);
    col /= scale;

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

    float col0 = computeTrTriangle(x, v0, v1, v2, col.x, col.y, col.z, n0);
    float col1 = computeTrTriangle(x, v0, v2, v3, col.x, col.z, col.w, n1);
    float col2 = computeTrTriangle(x, v1, v0, v3, col.y, col.x, col.w, n2);
    float col3 = computeTrTriangle(x, v1, v2, v3, col.y, col.z, col.w, n3);

    float d0 = abs(plane(x, c0, n0));
    float d1 = abs(plane(x, c1, n1));
    float d2 = abs(plane(x, c2, n2));
    float d3 = abs(plane(x, c3, n3));

    float dmax = max(d0, max(d1, (max(d2, d3))));
    float dmin = min(d0, min(d1, (min(d2, d3))));

    if (d0 >= 0.001) return col.x;
    if (d1 >= 0.001) return col.y;
    if (d2 >= 0.001) return col.z;
    if (d3 >= 0.001) return col.w;
    float colB1 = (col.y / d1 + col.x / d0 + col.w / d3 + col.z / d2) / (d1 + d0 + d3 + d2);

    return colB1;
}

inline __device__
float sdTetraD(float3 p, float3 v0, float3 v1, float3 v2, float3 v3)
{
    float f = Tetra(p, v0, v1, v2, v3); //compute tetra
    return f;
}

inline __device__
float sdTetra(float3 p, float3 v0, float3 v1, float3 v2, float3 v3)
{
    float t = TimeSound;// +1.0f;

    return Tetra(p, v0, v1, v2, v3); //compute tetra
}
inline __device__
float sdTetra2(float3 p, float3 v0, float3 v1, float3 v2, float3 v3)
{
    float t = TimeSound;// +1.0f;

    if (t < 2.0) {
        float f = Tetra(p, v0, v1, v2, v3); //compute tetra
        if (t < 1.0) {
            float3 ct = (v0 + v1 + v2 + v3) / 4.0f;
            float3 c0 = getCenter(v0, v2, v1);
            //float rad1 = length(ct - c0);
            float rad = length(ct - c0) / 2.0;
            float f1 = length(p - ct) - rad;
            //interpolation
            return (1 - t)*f1 + t*f;
        }
        else { //other time frame
            float f1 = TetraWire2(p, v0, v1, v2, v3);

            //interpolation
            float tt = t - 1;
            return (1 - (tt*tt*tt))*f + (tt*tt*tt)*f1;
        }
    }
    else {
        float f = TetraWire2(p, v0, v1, v2, v3);
        float3 ct = (v0 + v1 + v2 + v3) / 4.0f;
        float3 c0 = getCenter(v0, v2, v1);
        float rad = length(ct - c0);

        if (rad < 1.4) {
            //float f1 = Tetra(p, v0, v1, v2, v3);
            float3 ct = (v0 + v1 + v2 + v3) / 4.0f;
            float3 c0 = getCenter(v0, v2, v1);
            //float rad1 = length(ct - c0);
            float rad = length(ct - c0);
            float f1 = length(p - ct) - rad;
            //interpolation
            float tt = t - 2;
            return (1 - (tt))*f + (tt)*f1;
        }
        else {
            float f1 = TetraWire(p, v0, v1, v2, v3);
            float fs1 = length(p - v0) - 1.1;
            float fs2 = length(p - v1) - 1.1;
            float fs3 = length(p - v2) - 1.1;
            float fs4 = length(p - v3) - 1.1;

            float tt = (t - 2) * 4;

            float f2 = sminp(f, fs1, 0.8);
            f2 = sminp(f2, fs2, 0.8);
            f2 = sminp(f2, fs3, 0.8);
            f2 = sminp(f2, fs4, 0.8);
            f1 = sminp(f1, f2, 0.8);

            if (tt < 1.0)
            {
                return (1 - (tt))*f + (tt)*f2;
            }
            else {
                float tt2 = (tt - 1) / 3;
                //interpolation
                return (1 - (tt2*tt2))*f2 + (tt2*tt2)*f1;
            }
        }
        /*
       else {
           float3 c1 = getCenter(v0, v3, v2);
           float3 c2 = getCenter(v1, v3, v0);
           float3 c3 = getCenter(v1, v2, v3);

           float f1 = bond(p, ct, c1, 0.4);
           float f2 = bond(p, ct, c2, 0.4);
           float f3 = bond(p, ct, c3, 0.4);
           float f4 = bond(p, ct, c0, 0.4);

           f1 = sminp(f2, f1, 0.8);
           f1 = sminp(f3, f1, 0.8);
           f1 = sminp(f4, f1, 0.8);
           //interpolation
           return (1 - (t - 2))*f + (t - 2)*f1;
       }*/
    }
    /*if (TimeSound < 3.0) {
        float3 c0 = getCenter(v0, v2, v1);
        float3 c1 = getCenter(v0, v3, v2);
        float3 c2 = getCenter(v1, v3, v0);
        float3 c3 = getCenter(v1, v2, v3);

        float3 ct = (v0 + v1 + v2 + v3) / 4.0f;
        float rad1 = length(ct - c0);
        float rad = length(ct - v0);
        rad = (rad + rad1) / (2.0*t);
        float3 n0 = getNormal(v0, v2, v1, c0, ct);
        float3 n1 = getNormal(v0, v3, v2, c1, ct);
        float3 n2 = getNormal(v1, v3, v0, c2, ct);
        float3 n3 = getNormal(v1, v2, v3, c3, ct);

        float a = plane(p, c0, n0);
        float b = plane(p, c1, n1);
        float c = plane(p, c2, n2);
        float d = plane(p, c3, n3);
        float f1 = fmaxf(fmaxf(a, b), fmaxf(c, d));
        float3 ct2 = ct - (t - 1) / 10 * v0;
        float f = length(p - ct2) - rad;
        return smaxp(-f, f1, 1.8);
    }
    else*/
}

inline __device__  void sampleVolume(int pN, PerRayData& prd, float3 origin, float3 direction, float rayMin)
{
    float Ka = 0.5;
    float Kd = 0.9;
    float Ks = 0.9;
    cellPrimDesc cell = prd.cellPrimitives[pN];
    primParamDesc prim = prd.prims[pN];
    float initialStep = 0.1;
    float tstep = initialStep;
    float3 c = (prim.pos[0] + prim.pos[1] + prim.pos[2] + prim.pos[3]) / 4.0;
    float rad = length(c - prim.pos[0]);
    float primNorm = rad / ((TimeSound + 1) / 2);
    float dist = cell.intersectionDist - rad;

    if (dist >= rayMin)
    {
        float3 pos = origin + direction*dist;
        float3 step = direction*tstep;

        float4 sum = prd.result;// make_float4(prd.radiance.x, prd.radiance.y, prd.radiance.z, 0.1); //TODO: get background color here

        float s1 = sdTetra(pos, prim.pos[0], prim.pos[1], prim.pos[2], prim.pos[3]);
        //evalF(pos, prim);
        //int numSteps = 10;
        float i = 0.0;
        float max = cell.maxDist;// *2 + 0.4; //bounding box size

        while (i < max) //s2 < 0.01)
        {
            float ftr = GetIso(pos, prim.pos[0], prim.pos[1], prim.pos[2], prim.pos[3], prim.rad[0], prim.rad[1], prim.rad[2], prim.rad[3]);//cell.color;// make_float4(color2.x, color2.y, color2.z, trp*Ka);
            if (abs(ftr) < 2.2)
            {
                if (s1 < initialStep)
                {
                    //	optix::float3 hit_point = origin + theIntersectionDistance * direction;
                    //float3 cc = translucent_grays(ftr,ftr, 1); //GetColor(pos, prim.pos[0], prim.pos[1], prim.pos[2], prim.pos[3], prim.rad[0], prim.rad[1], prim.rad[2], prim.rad[3]);//cell.color;// make_float4(color2.x, color2.y, color2.z, trp*Ka);
                    float4 col = translucent_grays(ftr, ftr, 1); //make_float4(cc.x, cc.y, cc.z, cell.color.w);
                    col.w = cell.color.w;
                    if (tstep > initialStep)
                    {
                        float F = exp(-col.w*abs(s1) * 40 / primNorm);
                        col = col*(1.0 - F);
                        col.w = (1.0 - F);
                    }
                    else {
                        col.x *= col.w;
                        col.y *= col.w;
                        col.z *= col.w;
                    }

                    float t = sum.w;
                    // "over" operator for front-to-back blending
                    sum = sum + col*(1.0f - sum.w);
                }
            }
            tstep = fmax(initialStep, abs(s1));
            step = direction*tstep;
            i += tstep;
            pos += step;
            if (sum.w >= 1.0) {
                i = max + 1;
            }
            else
                s1 = sdTetra(pos, prim.pos[0], prim.pos[1], prim.pos[2], prim.pos[3]);
            //evalF(pos, prim);
        }

        prd.result = sum;
    }
}

inline __device__  void render_HeteroVolume(int pN, PerRayData& prd, float3 origin, float3 direction)
{
    float Ka = 0.5;
    float Kd = 0.9;
    float Ks = 0.5;
    cellPrimDesc cell = prd.cellPrimitives[pN];
    primParamDesc prim = prd.prims[pN];
    float tstep = 0.05;
    float dist = cell.intersectionDist;

    float3 pos = origin + direction*dist;
    float3 step = direction*tstep;

    float4 sum = prd.result;// make_float4(prd.radiance.x, prd.radiance.y, prd.radiance.z, 0.1); //TODO: get background color here
    float trp = 0.05;
    float trp0 = 0.1;

    float s1 = evalF(pos, prim);
    if (abs(s1) > tstep)
        step = direction*abs(s1);

    float i = 0.0;
    float max = cell.maxDist;// *2 + 0.4; //bounding box size
    float4 sumcol = make_float4(0.0);
    float tracedDist = 0;

    BasicLight lights2[2];
    lights2[0].color = optix::make_float3(1.0);
    lights2[0].pos = optix::make_float3(10.0);

    lights2[1].color = optix::make_float3(1.0);
    lights2[1].pos = optix::make_float3(0, 0, 10.0);

    //float4 col1 = translucent_grays(0.5, 0.01, 0);
    int VolInt = 1;

    while (i < max) //s2 < 0.01)
    {
        if (s1 < tstep)
        {
            // if (abs(s1) > tstep) //sum transparency
            {
                //is used to highlight isosurfaces
                //or create a more shell like effect
                //trp = trp0 + abs(s1) / 10;

                //----------------------
                //COLOR COMPUTATION
                //trp = trp0;

                VolInt = 1;
                //------------------

                //float3 color = Ka *  cell.color;// ambient_light_color;
                //float3 color2 = Ka *  cell.color;
                float3 c = evalCol(pos, prim);//	optix::float3 hit_point = origin + theIntersectionDistance * direction;
                float4 col = make_float4(c.x, c.y, c.z, cell.color.w);// make_float4(color2.x, color2.y, color2.z, trp*Ka);
                col *= Ka;
                if (i < tstep * 2) //surface
                {
                    float3 color = make_float3(col.x, col.y, col.z);                                          //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

                    for (int i = 0; i < 2; ++i)
                    {
                        BasicLight light = lights2[i];
                        float3 L = optix::normalize(light.pos - pos);
                        float nDl = optix::dot(cell.normal, L);

                        //if (nDl > 0)
                        //    color += Kd * nDl * light.color; // make_float3(1.0);//

                        float phong_exp = 0.1;
                        if (nDl > 0) {
                            color += Kd * nDl * light.color;

                            optix::float3 H = optix::normalize(L - direction);
                            float nDh = optix::dot(cell.normal, H);
                            if (nDh > 0)
                                color += Ks * light.color * pow(nDh, phong_exp);
                        }
                    }
                    col.x = color.x;
                    col.y = color.y;
                    col.z = color.z;
                }

                //Beer–Lambert law
                float F = exp(-cell.color.w*abs(s1) * 200);
                col = col*(1.0 - F);
                sum = sum + col*(1.0f - sum.w);

                // tracedDist += abs(s1);
            }
        }

        //s1 = s2;

        if (VolInt > 0) {
            if (abs(s1) > tstep) {
                step = direction*abs(s1);
                i += abs(s1);
            }
            else
            {
                i += tstep;
                step = direction*tstep;
            }
        }
        else { //volume sampling
            if (s1 > tstep) //employ space skipping
            {
                step = direction*abs(s1);
                i += abs(s1);
            }
            else {
                i += tstep;
                step = direction*tstep;
            }
        }

        pos += step;
        if (sum.w >= 1.0) {
            i = max + 1;
        }
        else
            s1 = evalF(pos, prim);
    }

    prd.result = sum;
}

inline __device__ void render_HeteroVolume_initial(int pN, PerRayData& prd, float3 origin, float3 direction)
{
    float Ka = 0.5;
    float Kd = 0.9;
    float Ks = 0.9;

    float tstep = 0.1;
    cellPrimDesc cell = prd.cellPrimitives[pN];
    primParamDesc prim = prd.prims[pN];
    float dist = cell.intersectionDist;

    if (prim.type == 4) {//tetra
        dist = fmaxf(0.0f, dist - prim.rad[0]);
    }
    //---------------
    float3 pos = origin + direction*dist;
    float3 step = direction*tstep;

    float4 sum = prd.result;// make_float4(prd.radiance.x, prd.radiance.y, prd.radiance.z, 0.1); //TODO: get background color here

    if (sum.w >= 1.0) return;
    //compute color

    float trp = 0.05;
    float segmLength = 0.0;

    float s1 = evalF(pos, prim); //sdTetra(pos,prim.pos[0],prim.pos[1],prim.pos[2],prim.pos[3]);
    if (abs(s1) > tstep)
        step = direction*abs(s1);

    float i = 0.0;
    float max = cell.maxDist;// *2 + 0.4; //bounding box size

    float tracedDist = 0;

    //float4 col1 = translucent_grays(0.5, 0.01, 0);
    int VolInt = 1;
    optix::float4 col = cell.color*Ka;

    while (i < max) //s2 < 0.01)
    {
        if (s1 < 0)
        {
            {
                VolInt = 1;
                //------------------
                float4 col2 = cell.color*Ka;// make_float4(col.x, col.y, col.z, trp*Ka);

                                            // if (abs(s1) > tstep / 2)
                {
                    //Beer–Lambert law
                    float F = exp(-col2.w*abs(s1) * 100);
                    col2 = col2*(1.0 - F);
                    col2.w = (1.0 - F);
                    sum = sum + col2*(1.0f - sum.w);
                }
                /*  else //sampling
                {
                trp = col2.w;
                col2 *= col2.w;
                col2.w = trp;
                sum = sum + col2*(1.0f - sum.w);
                }*/
                // tracedDist += abs(s1);
            }
        }

        //s1 = s2;

        if (abs(s1) > tstep) {
            step = direction*abs(s1);
            i += abs(s1);
        }
        else
        {
            i += tstep;
            step = direction*tstep;
        }

        pos += step;
        if (sum.w >= 1.0) {
            i = max + 1;
        }
        else
            s1 = evalF(pos, prim); //sdTetra(pos, prim);
    }

    prd.result = sum;
}

/* skips overlap up of two cells*/
__device__ int skipOverlap2(const int pN, PerRayData& prd)
{
    int i = pN;
    int j = pN + 1;

    int N = prd.cur_prim;
    if (j >= N) return i;

    const cellPrimDesc curCell = prd.cellPrimitives[i];
    const cellPrimDesc nextCell = prd.cellPrimitives[j];

    int type1 = prd.prims[i].type;
    int type2 = prd.prims[j].type;
    //compute boundary values
    float dmin1 = curCell.intersectionDist;
    float dmin2 = nextCell.intersectionDist;

    float dmax1 = curCell.maxDist;
    float dmax2 = nextCell.maxDist;
    if ((type1 == type2) && (type1 != 4))
    {
        //if ((curDist - dmin2 > 0.001f) && (dmax2 - curDist > 0.001f)) {//(abs(dmin1 - dmin2) < 0.1) &&
        if (abs(dmin1 - dmin2) < 0.3) //&& (abs(dmax1 - dmax2) < 0.001)) {
                                       //skip cell
        {
            if (dmax1 > dmax2) { //should trace the max cell
                prd.cellPrimitives[j] = prd.cellPrimitives[i];
                prd.prims[j] = prd.prims[i];
            }
            return j;
        }
    }
    return i;
}
/* volume ray cast*/

/*Sorts prim info and prim array by depth*/
__device__ void basic_sort(PerRayData& prd) {
    int N = prd.cur_prim;// -1;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N - i - 1; j++)
        {
            const cellPrimDesc tmp = prd.cellPrimitives[i];
            // const float2 tmp = prd.particles[i];
            if (tmp.intersectionDist < prd.cellPrimitives[j].intersectionDist) {
                const primParamDesc tmp2 = prd.prims[i];

                prd.cellPrimitives[i] = prd.cellPrimitives[j];
                prd.cellPrimitives[j] = tmp;

                //sort prim info as well
                prd.prims[i] = prd.prims[j];
                prd.prims[j] = tmp2;
            }
        }
}

__device__ void render_Tetra(PerRayData& prd, float3 origin, float3 direction, float tmin)
{
    if (prd.result.w > 0.7)
        return;
    //printf("Render tetra");
    int VolInt = 1;
    //-----------------
    /*depth sort*/
    //---------------
    int N = prd.cur_prim;
    basic_sort(prd);

    //integrate over cells
    int i = 0;

    //prd.result += make_float4(1, 0, 0, 1);
    while (i < N)
    {
        //skip overlap
        //int j = skipOverlap2(i, prd);

        sampleVolume(i, prd, origin, direction, tmin);

        i++;
        if (prd.result.w > 0.7) i = N;
    }
}
__device__ void render_Mol(PerRayData& prd, float3 origin, float3 direction)
{
    if (prd.result.w > 0.7)
        return;
    //printf("Render tetra");
    int VolInt = 1;
    //-----------------
    /*depth sort*/
    //---------------
    int N = prd.cur_prim;
    basic_sort(prd);

    //integrate over cells
    int i = 0;

    //prd.result += make_float4(1, 0, 0, 1);
    while (i < N)
    {
        //skip overlap
        int j = skipOverlap2(i, prd);

        render_HeteroVolume(j, prd, origin, direction);

        i = j + 1;
        if (prd.result.w > 0.7) i = N;
    }
}

static __device__ __inline__  optix::Ray ComputeDirPos(PerRayData& prd)
{
    //---------------
    //Linking to cuda threads. This is implemented as in CUDA ADVANCED SAMPLES
    //link pixel number to thread
    const float2 pixel = make_float2(theLaunchIndex);

    //no antializing
    const float2 fragment = pixel + 0.5;

    // The launch dimension (set with rtContextLaunch) is the full client window in this demo's setup.
    const float2 screen = make_float2(theLaunchDim);

    const float2 ndc = (fragment / screen) * 2.0f - 1.0f;

    const float3 origin = sysCameraPosition;
    const float3 direction = optix::normalize(ndc.x * sysCameraU + ndc.y * sysCameraV + sysCameraW);

    //if (isDynamic)
    prd.TimeSound = TimeSound;

    //TODO: we now just compute optical_LaunchDim/auditory_LaunchDim ratio

   /* if (computeAuditoryRendering>0) {
        int numS = 0;
        prd.isSoundRay = isSoundRay(numS, ndc, pixel, screen);
        prd.numS = numS;
    }*/

    // Create ray
    return optix::make_Ray(origin, direction, 0, 0.0f, RT_DEFAULT_MAX);
}
// Entry point for a pinhole camera.
RT_PROGRAM void raygeneration0()
{
    PerRayData prd;
    // Initialize the random number generator seed from the linear pixel index and the iteration index.
    //    prd.seed = tea<16>(theLaunchIndex.y * theLaunchDim.x + theLaunchIndex.x, 0);

    prd.radiance = make_float3(0.0f);
    prd.depth = 0;
    prd.result = make_float4(0.5);
    prd.cur_prim = 0;
    prd.isSoundRay = false;
    //   prd.rnd = rng(prd.seed);
    prd.TimeSound = 0.0f;
    prd.totalDist = 0;
    prd.radiance = sysBackground;
    prd.length = 0;
    prd.block = true;
    prd.renderType = 0;
    prd.result = make_float4(sysBackground);
    prd.result.w = 0.1;
    //compute normalized ray direction[-1,1]
    optix::Ray ray = ComputeDirPos(prd);

    // Start tracing ray from the camera and further
    rtTrace(sysTopObject, ray, prd);
    if (prd.renderType > 2)
    {
        //tetra or molecules
        render_Mol(prd, ray.origin, ray.direction);
    }
    //postprocessing and rendering
    //if (prd.renderType > 0)
    //    postRender(ray, prd, prd.normal, prd.last_hit_point);

    //prd.radiance *= 2.5 + make_float3(prd.result); //worked previously. instead
    // prd.result = prd.result + col*(1.0f - prd.result.w);

    //prd.result *= 1.5;
    //prd.result += col;
    prd.radiance = make_float3(prd.result.x, prd.result.y, prd.result.z);

    //float4 val = prd.result;// make_float4(prd.radiance, 0.0f);// +prd.result;//+ // *(1 - prd.result.w)//+make_float4(prd.radiance, 0.0f);// +;////;
    // val.w = 1;
    sysOutputBuffer[theLaunchIndex] = make_float4(prd.radiance, 1.0f);// +prd.result;//;
}
// Entry point for a pinhole camera.
RT_PROGRAM void raygeneration1()
{
    PerRayData prd;
    // Initialize the random number generator seed from the linear pixel index and the iteration index.
//    prd.seed = tea<16>(theLaunchIndex.y * theLaunchDim.x + theLaunchIndex.x, 0);

    prd.radiance = make_float3(0.0f);
    prd.depth = 0;
    prd.result = make_float4(0.5);
    prd.cur_prim = 0;
    prd.isSoundRay = false;
    //   prd.rnd = rng(prd.seed);
    prd.TimeSound = 0.0f;
    prd.totalDist = 0;
    prd.radiance = sysBackground;
    prd.length = 0;
    prd.depth = 0;
    prd.block = true;
    prd.renderType = 0;
    prd.result = make_float4(sysBackground);
    prd.result.w = 0.01;
    //compute ray direction[-1,1]
    optix::Ray ray = ComputeDirPos(prd);

    float3 t0, t1, tmin, tmax;
    t0 = (bbox_max - ray.origin) / ray.direction;
    t1 = (bbox_min - ray.origin) / ray.direction;
    tmax = fmaxf(t0, t1);
    tmin = fminf(t0, t1);
    float tenter = fmaxf(0.f, fmaxf(tmin.x, fmaxf(tmin.y, tmin.z)));
    float texit = fminf(tmax.x, fminf(tmax.y, tmax.z));

    if (tenter < texit)
    {
        float tbuffer = 0.f;
        float step = 5.0;
        while (tbuffer < texit && prd.result.w < 0.7)
        {
            ray.tmin = tenter;
            ray.tmax = texit;
            ray.tmin = fmaxf(tenter, tbuffer);
            ray.tmax = fminf(texit, tbuffer + step);

            if (ray.tmax > tenter)    //doing this will keep rays more coherent
            {
                prd.cur_prim = 0;
                // Start tracing ray from the camera and further
                rtTrace(sysTopObject, ray, prd);

                //postprocessing and rendering

                if (prd.renderType > 2)
                {
                    //tetra or molecules
                    render_Tetra(prd, ray.origin, ray.direction, tenter);
                }

                //if (prd.renderType > 0)
                //    postRender(ray, prd, prd.normal, prd.last_hit_point);
            }

            tbuffer += step;
        }
    }

    float sampOp = 1.0;

    prd.radiance = make_float3(sampOp*prd.result.x, sampOp*prd.result.y, sampOp*prd.result.z);
    /* TODO:Antialising is not implemented as far. So some small distortions might present*/
    sysOutputBuffer[theLaunchIndex] = make_float4(prd.radiance, 1.0f);// +prd.result;//;
}

RT_PROGRAM void auditory_raygeneration()
{
    PerAudioRayData prd;
    // Initialize the random number generator seed from the linear pixel index and the iteration index.
//    prd.seed = tea<16>(theLaunchIndex.y * theLaunchDim.x + theLaunchIndex.x, 0);

    prd.radiance = make_float3(0.0f);
    prd.depth = 0;
    prd.result = make_float4(0.5);
    prd.cur_prim = 0;
    prd.isSoundRay = false;
    //    prd.rnd = rng(prd.seed);
    prd.TimeSound = 0.0f;

    for (int i = 0; i < MAX_PRIM_ALONG_RAY; i++)
    {
        prd.primitives[i] = make_float2(0);
    }

    //compute normalized ray direction[-1,1]
/*    optix::Ray ray = ComputeDirPos(prd);

    prd.TimeSound = TimeSound;

    // Start tracing ray from the camera and further
    rtTrace(sysTopObject, ray, prd);

    int num = 0;
    for (int i = 0; i < MAX_PRIM_ALONG_RAY - 1; i++)
    {
        sysAuditoryOutputBuffer[theLaunchIndex][i] = prd.primitives[i];
        if (prd.primitives[i].y > 0) num++;
    }
    sysAuditoryOutputBuffer[theLaunchIndex][MAX_PRIM_ALONG_RAY - 1] = make_float2(prd.cur_prim, num);
    */
}

//This one for interactive widget
//--------------------------------------------------------------------------------------

//Buffers

rtBuffer<float3, 2> movingPoints; // registered point cloud
rtBuffer<float3, 2> fixedPoints; // to be alligned with point cloud
//for SDF

typedef rtCallableProgramId<float(float3, float3)> callT;
rtDeclareVariable(callT, sdfPrim, , );

//TODO: sphere tracing of the widget
RT_CALLABLE_PROGRAM optix::Ray SphereTraceGeometry(optix::Ray  ray, bool& found)
{
    //TODO: consider case when there is no intersection with geometry
    optix::Ray  ray2 = ray;

    float tmin, tmax;
    tmin = 0;
    tmax = RT_DEFAULT_MAX;

    // === Raymarching (Sphere Tracing) Procedure ===

    float eps = 0.001;
    float t = 0.0002;
    float3 rad = make_float3(1.5);

    for (int i = 0; i < 100; i++)
    {
        optix::float3 p = ray.origin + t* ray.direction;
        float hit = abs(sdfPrim(p, rad)); //make it always positive so we can step to border
        if (hit < 0.0002) //|| t > 20.0)
        {
            //there is an intersectuib

            float dx = sdfPrim(p + make_float3(eps, 0, 0), rad) - sdfPrim(p - make_float3(eps, 0, 0), rad);
            float dy = sdfPrim(p + make_float3(0, eps, 0), rad) - sdfPrim(p - make_float3(0, eps, 0), rad);
            float dz = sdfPrim(p + make_float3(0, 0, eps), rad) - sdfPrim(p - make_float3(0, 0, eps), rad);

            //info.normal = normalize(make_float3(dx, dy, dz));
            ray2.origin = p;// origin + t*direction;
            //make it sphere normal
            //ray2.direction = normalize(make_float3(dx, dy, dz));

            found = true;
            break;
        }

        if (t > 20.0) {
            found = false;
            break; //no intersection
        }
        t += abs(hit); //only positive direction
    }

    return ray2;
}

rtDeclareVariable(float3, widgetCenter, , );
//-----------
//For widget Ray generation
static __device__ __inline__  optix::Ray ComputeDirPosWidget(PerRayData& prd)
{
    //---------------
    //Linking to cuda threads. This is implemented as in CUDA ADVANCED SAMPLES
    //link pixel number to thread
    const float2 pixel = make_float2(theLaunchIndex);

    //no antializing
    const float2 fragment = pixel + 0.5;

    // The launch dimension (set with rtContextLaunch) is the full client window in this demo's setup.
    const float2 screen = make_float2(theLaunchDim);

    const float2 ndc = (fragment / screen) * 2.0f - 1.0f;

    const float3 origin = widgetCenter;
    const float3 direction = optix::normalize(ndc.x * sysCameraU + ndc.y * sysCameraV + sysCameraW);

    if (isDynamic)
        prd.TimeSound = TimeSound;

    //TODO: we now just compute optical_LaunchDim/auditory_LaunchDim ratio

   /* if (computeAuditoryRendering>0) {
        int numS = 0;
        prd.isSoundRay = isSoundRay(numS, ndc, pixel, screen);
        prd.numS = numS;
    }*/

    // Create ray
    return optix::make_Ray(origin, direction, 0, 0.0f, RT_DEFAULT_MAX);
}

// Entry point for a widget.
//First we should sphere trace it's geometry
RT_PROGRAM void audio_ray_cast()
{
    PerAudioRayData prd;
    // Initialize the random number generator seed from the linear pixel index and the iteration index.
//    prd.seed = tea<16>(theLaunchIndex.y * theLaunchDim.x + theLaunchIndex.x, 0);

    prd.radiance = make_float3(0.0f);
    prd.depth = 0;
    prd.result = make_float4(0.5);
    prd.cur_prim = 0;
    prd.isSoundRay = false;
    //    prd.rnd = rng(prd.seed);
    prd.TimeSound = 0.0f;
    prd.isDynamic = isDynamic;

    prd.dirCamera = optix::normalize(widgetCenter - sysCameraPosition);
    //----------
    //set fail value by default
    movingPoints[theLaunchIndex] = make_float3(-1000);
    fixedPoints[theLaunchIndex] = make_float3(-1000);

    for (int i = 0; i < MAX_PRIM_ALONG_RAY; i++)
    {
        prd.primitives[i] = make_float2(0);
    }

    //compute normalized ray direction[-1,1]
/*    optix::Ray ray = ComputeDirPosWidget(prd);

    bool found = false;
    optix::Ray ray2 = SphereTraceGeometry(ray, found);

    if (found) {
        prd.TimeSound = TimeSound;

        // Start tracing ray from the camera and further
        rtTrace(sysTopObject, ray2, prd);
        //to this point prd is filled with intersection info

        //conventional gathering of distance information for auditory rendering
        int num = 0;
        for (int i = 0; i < MAX_PRIM_ALONG_RAY - 1; i++)
        {
            sysAuditoryOutputBuffer[theLaunchIndex][i] = prd.primitives[i];
            if (prd.primitives[i].y > 0) num++;
        }
        sysAuditoryOutputBuffer[theLaunchIndex][MAX_PRIM_ALONG_RAY - 1] = make_float2(prd.cur_prim, num);

        //movingPoints[theLaunchIndex] = make_float3(-1000);

        if (num > 0) //there is an intersection
        {
            printf("%d NUM", num);
            //add a moving point and fixed if there is an
            //intersection
            if (prd.primitives[0].x < 1.5) {
                movingPoints[theLaunchIndex] = ray2.origin;
                fixedPoints[theLaunchIndex] = ray2.origin + ray2.direction*prd.primitives[0].x;
            }
        }
        //   else //there is no intersection, identification point
        //   {
        //       movingPoints[theLaunchIndex] = make_float3(-1000);
        //       fixedPoints[theLaunchIndex] = make_float3(-1000);
        //   }
    } //if (num > 0)

    //TODO: do something with outputsys buff as postprocessing
      //  }
      */
}