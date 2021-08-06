#include <optix_math.h>
#include "primDesc.h"

rtDeclareVariable(float, spRad, , );
rtDeclareVariable(float3, center, , );

rtTextureSampler<float, 3> texSDF0;
rtDeclareVariable(float, shift0, , );
rtDeclareVariable(float, size0, , );

rtTextureSampler<float, 3> texSDF1;
rtDeclareVariable(float, shift1, , );
rtDeclareVariable(float, size1, , );

rtTextureSampler<float, 3> texSDF2;
rtDeclareVariable(float, shift2, , );
rtDeclareVariable(float, size2, , );

rtDeclareVariable(int, numTexDefined, , );
rtDeclareVariable(float, TimeSound, , );
//for SDF

typedef rtCallableProgramId<float(float3, float3)> callT;
rtDeclareVariable(callT, sdfOpPrim, , );

typedef rtCallableProgramId<float(float3, float3)> callT;
rtDeclareVariable(callT, sdfOpPrim2, , );

rtDeclareVariable(float3, sCell1, , );
rtDeclareVariable(float3, sCell2, , );
rtDeclareVariable(float3, sCell3, , );

//------------------------------------
// list of all used or unsued parameters

rtDeclareVariable(float3, varRadius, , );
rtDeclareVariable(float3, varCenter, , );

rtDeclareVariable(float3, varCenter0, , );
rtDeclareVariable(float3, varCenter1, , );
rtDeclareVariable(float3, varCenter2, , );
rtDeclareVariable(float3, varCenter3, , );
rtDeclareVariable(float3, varCenter4, , );

rtDeclareVariable(float, varK, , );
rtDeclareVariable(float2, varT, , );
rtDeclareVariable(float, MultiscaleParam, , );
rtDeclareVariable(float3, sysCameraPosition, , );
rtDeclareVariable(float, varBlob, , );

//#include "sdfGeometryVariables.h"

__device__
inline float3 max(float3 a, float3 b)
{
    return make_float3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}

__device__
inline float3 abs(float3 a)
{
    return max(-a, a);
}

__device__
inline float  plane(float3 p, float3 c, float3 n)
{
    return dot(p - c, n);
}
__device__
inline float3 getNormal(float3 v1, float3 v2, float3 v3, float3 c, float3 ct)
{
    float3 a = v3 - v2;
    float3 b = v1 - v2;
    float3 n = cross(a, b);

    float3 nt = c - ct;

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

RT_CALLABLE_PROGRAM
float sdfTetra(float3 p, float3 v0, float3 v1, float3 v2, float3 v3)
{
    float3 c0 = getCenter(v0, v2, v1);
    float3 c1 = getCenter(v0, v3, v2);
    float3 c2 = getCenter(v1, v3, v0);
    float3 c3 = getCenter(v1, v2, v3);

    float3 ct = (v0 + v1 + v2 + v3) / 4.0;
    // float rad = length(ct - c0);

    //  return length(p - ct) - rad;

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

RT_CALLABLE_PROGRAM float sdfSphere2(float3 p, float rad)
{
    return length(p) - rad;
}

//for molecules
RT_CALLABLE_PROGRAM float sdfSphere(float3 p, float3 rad)
{
    return length(p) - rad.x;
}
//for molecules
RT_CALLABLE_PROGRAM float bond(float3 p, float3 a, float3 b, float r)
{
    float3 pa = p - a;
    float3 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba*h) - r;
}

RT_CALLABLE_PROGRAM float sdf_opSmoothUnion(float d1, float d2, float k) {
    //float k = 0.2; //0.7

    float val = 0.5f + 0.5f*(d2 - d1) / k;
    float h = clamp(val, 0.0f, 1.0f);
    float mix_res = (1.0 - h)*d2 + d1*h;
    return mix_res - k*h*(1.0 - h);
}

RT_CALLABLE_PROGRAM float sdfBondSphereBlob(float3 p, float3 a, float3 b, float rad1, float rad2)
{
    //float bf = bond(p, a, b, 0.1);
    float f1 = length(p - a) - rad1 / 2;
    float f2 = length(p - b) - rad2 / 2;

    // if (f1 <= 0) return f1;
    // if (f2 <= 0) return f2;
    // float3 dir = normalize(a - b);
    float rb = length((a - b) / 2);// + fmax(rad1, rad2);
    //float f = sdf_opSmoothUnion(f1, bf,0.8);
    //return sdf_opSmoothUnion(f2, f);
    float bf = bond(p, a, b, 0.1);
    if (f1 < rb)
        // bf = fminf(f1, bf);
        bf = sdf_opSmoothUnion(f1, bf, varBlob);
    if (f2 < rb)
        bf = sdf_opSmoothUnion(f2, bf, varBlob);
    //bf = fminf(f2, bf);//sdf_opSmoothUnion(f2, bf, 0.2);//
//float f11 = sdfMicrostructure2(p - a, rad1);*/
    return bf;
    /*bf = sdf_opSmoothUnion(f1, bf, 0.1);
    return sdf_opSmoothUnion(f2, bf, 0.1);*/
}

RT_CALLABLE_PROGRAM float sdfBondSphereMolBlob(float3 p, float3 a, float3 b, float3 c, float rad1, float rad2, float rad3)
{
    //float bf = bond(p, a, b, 0.1);
    float f1 = length(p - a) - rad1;
    float f2 = length(p - b) - rad2;
    float f3 = length(p - c) - rad3;

    //float f = sdf_opSmoothUnion(f1, bf,0.8);
    //return sdf_opSmoothUnion(f2, f);
    float f = sdf_opSmoothUnion(f2, f1, 1.0);
    return  sdf_opSmoothUnion(f3, f, 1.0);
}

RT_CALLABLE_PROGRAM float sminp(float a, float b, float k)
{
    float h = optix::max(k - abs(a - b), 0.0) / k;
    return optix::min(a, b) - h*h*k*(1.0 / 4.0);
}
RT_CALLABLE_PROGRAM float sdfMicrostructure4(float3 p, float rad1)
{
    float t = 2;

    float3 scale = 0.3*make_float3(abs(sin(t)), abs(cos(t)), abs(cos(t)));
    float dens_scale = abs(cos(t)) / 2;

    // float sphere1 = optix::length(p) - rad.x;// sdSphere(p, rad);
    // float sphere2 = optix::length(p) - rad.x / 2.2;// sdSphere(p, rad / 20);

    float rad2 = rad1 - 0.1;
    float sphere1 = optix::length(p) - rad2;
    float sphere2 = optix::length(p) - (rad2 - 0.1);
    float shell = max(sphere1, -sphere2);
    shell = max(shell, p.z);

    float3 tiled = make_float3(dens_scale);
    float3 tiled2 = 0.2 + tiled;

    float3 x = p + 0.5*tiled;
    // x - y * floor(x / y).
    float3 mod = x - tiled2*floor(x / tiled2);//modf(p + 0.5*tiled, tiled)
    float3 inX = mod - 0.5*tiled2;

    float3 c = make_float3(0., 0., 0.03 + 0.06*dens_scale);
    float cyly = length(make_float2(inX.x, inX.z) - make_float2(c.x, c.y)) - c.z;
    float cylx = length(make_float2(inX.y, inX.z) - make_float2(c.x, c.y)) - c.z;
    float cylz = length(make_float2(inX.x, inX.y) - make_float2(c.x, c.y)) - c.z;

    float mics = sminp(cylx, sminp(cyly, cylz, 0.08), 0.08);

    float res = sminp(shell, mics, 0.1);
    res = max(max(res, sphere1), x.z);
    // res = max(res, x.z);
    return res;
}

RT_CALLABLE_PROGRAM float sdfMicrostructure2(float3 p, float rad1)
{
    float t = TimeSound * 4;

    float3 scale = 0.3*make_float3(abs(sin(t)), abs(cos(t)), abs(cos(t)));
    float dens_scale = abs(cos(t)) / 2;

    // float sphere1 = optix::length(p) - rad.x;// sdSphere(p, rad);
    // float sphere2 = optix::length(p) - rad.x / 2.2;// sdSphere(p, rad / 20);

    float rad2 = rad1 - 0.1;
    float sphere1 = optix::length(p) - rad2;
    float sphere2 = optix::length(p) - (rad2 - 0.1);
    float shell = max(sphere1, -sphere2);
    shell = max(shell, p.z);

    float3 tiled = make_float3(dens_scale);
    float3 tiled2 = 0.2 + tiled;

    float3 x = p + 0.5*tiled;
    // x - y * floor(x / y).
    float3 mod = x - tiled2*floor(x / tiled2);//modf(p + 0.5*tiled, tiled)
    float3 inX = mod - 0.5*tiled2;

    float3 c = make_float3(0., 0., 0.03 + 0.06*dens_scale);
    float cyly = length(make_float2(inX.x, inX.z) - make_float2(c.x, c.y)) - c.z;
    float cylx = length(make_float2(inX.y, inX.z) - make_float2(c.x, c.y)) - c.z;
    float cylz = length(make_float2(inX.x, inX.y) - make_float2(c.x, c.y)) - c.z;

    float mics = sminp(cylx, sminp(cyly, cylz, 0.08), 0.08);

    float res = sminp(shell, mics, 0.1);
    res = max(max(res, sphere1), x.z);
    // res = max(res, x.z);
    return res;
}
RT_CALLABLE_PROGRAM float sdfBondSphere1(float3 p, float3 a, float3 b, float rad1, float rad2)
{
    //  float f11 = length(p - a + 0.2*make_float3(sin(TimeSound * 20), cos(TimeSound * 20), 0)) - rad1 / 2;
    //  float f21 = length(p - b + 0.2*make_float3(cos(TimeSound * 20), -sin(TimeSound), 0)) - rad2 / 2;

     // float f12 = length(p - a + 0.2*make_float3(-sin(TimeSound * 20), cos(TimeSound * 20), 0)) - rad1 / 2;
     // float f22 = length(p - b + 0.2*make_float3(-cos(TimeSound * 20), sin(TimeSound * 20), 0)) - rad2 / 2;

    //  float f1 = sdf_opSmoothUnion(f11, f12, 0.2);
    //  float f2 = sdf_opSmoothUnion(f21, f22, 0.2);

    float3 vib1 = make_float3(0);
    float3 vib2 = make_float3(0);
    float3 up = make_float3(0, 0, 1);

    float bf = bond(p, a, b, 0.1);

    float f1 = length(p - (a)) - rad1 / 2; //first sphere
    float f2 = length(p - (b)) - rad2 / 2; //second sphere

    //-----

    float f = sdf_opSmoothUnion(f1, bf, 0.2);
    float mod1 = sdf_opSmoothUnion(f2, f, 0.1);

    float mod2 = sdfBondSphereBlob(p, a, b, rad1, rad2);

    //manual interpolation
    //return mod1*MultiscaleParam +(1.0-MultiscaleParam)*mod2;

    return mod1;
    //return sdf_opSmoothUnion(f2, f1, 0.5);
    //return f2;
}

RT_CALLABLE_PROGRAM float sdfBondSphere_simple(float3 p, float3 a, float3 b, float rad1, float rad2)
{
    //  float f11 = length(p - a + 0.2*make_float3(sin(TimeSound * 20), cos(TimeSound * 20), 0)) - rad1 / 2;
    //  float f21 = length(p - b + 0.2*make_float3(cos(TimeSound * 20), -sin(TimeSound), 0)) - rad2 / 2;

    // float f12 = length(p - a + 0.2*make_float3(-sin(TimeSound * 20), cos(TimeSound * 20), 0)) - rad1 / 2;
    // float f22 = length(p - b + 0.2*make_float3(-cos(TimeSound * 20), sin(TimeSound * 20), 0)) - rad2 / 2;

    //  float f1 = sdf_opSmoothUnion(f11, f12, 0.2);
    //  float f2 = sdf_opSmoothUnion(f21, f22, 0.2);

    float3 vib1 = make_float3(0);
    float3 vib2 = make_float3(0);
    float3 up = make_float3(0, 0, 1);

    float t = sin(TimeSound * 4);
    vib2 = (b - a) / 2 * t;

    float3 c = cross(a, b);
    // vib2 += -t*optix::normalize(c) / 10;

    float3 cent1 = a - vib2;
    float3 cent2 = b + vib2;
    float len = length(b - a);
    // len = 1;

     //todo: evaluate withi sphre bounds
    // const float t = length(p - theRay.origin);
    // const float3 pos_along_ray = theRay.origin + theRay.direction * t;
    int eval = 0;
    float f = -100000.0;
    //if (length(p - (a + b) / 2) <= len / 2)// && rtPotentialIntersection(tt))
    f = bond(p, a, b, 0.1);

    //float f = bf;
    if (length(p - a) < rad1 / 1.5) {
        float f1 = length(p - (a)) - rad1 / 2;// *len* (abs(t)+1)/6; //first sphere //sdfMicrostructure2 (p-a, len*rad1 * (abs(t)+1)/6); //
       // if (f > -0.5)
        f = sdf_opSmoothUnion(f1, f, 0.1);
        //  else f = f1;
    }

    if (length(p - b) < rad2 / 1.5)
    {
        float f2 = length(p - (b)) - rad2 / 2;//*len* (abs(t)+1)/6; //second sphere   //sdfMicrostructure2 (p-b, len*rad1 * (abs(t)+1)/6);//
       // if (f > -0.5)
        f = sdf_opSmoothUnion(f2, f, 0.1);
        //  else f = f2;
    }
    //f1 = length(p - (a + vib2)) - rad1 / 1.2;
    return f;
}

RT_CALLABLE_PROGRAM float sdfCrazyBond(float3 p, float3 a, float3 b, float rad1, float rad2)
{
    float3 vib1 = make_float3(0);
    float3 vib2 = make_float3(0);
    float3 up = make_float3(0, 0, 1);

    float t = sin(TimeSound * 4);
    vib2 = (b - a) / 2.2 * t;

    float3 c = cross(a, b);
    // vib2 += -t*optix::normalize(c) / 10;

    float bf = bond(p, a - vib2, b + vib2, 0.1);
    float len = length(b - a);
    len = 1;

    float f1 = length(p - (a)) - rad1;// *len* (abs(t)+1)/6; //first sphere //sdfMicrostructure2 (p-a, len*rad1 * (abs(t)+1)/6); //
    float f2 = length(p - (b)) - rad2;//*len* (abs(t)+1)/6; //second sphere   //sdfMicrostructure2 (p-b, len*rad1 * (abs(t)+1)/6);//

    if (f1 < 0.5) {
        if (t > 1)
            f1 = sdfMicrostructure4(p - a, rad1);
    }
    if (f2 < 0.5) {
        if (t > 1)
            f2 = sdfMicrostructure4(p - b, rad2);
    }

    return sdf_opSmoothUnion(f1, f2, 0.2);
}
RT_CALLABLE_PROGRAM float sdfBondSphere(float3 p, float3 a, float3 b, float rad1, float rad2)
{
    float3 vib1 = make_float3(0);
    float3 vib2 = make_float3(0);
    float3 up = make_float3(0, 0, 1);

    float t2 = sin(TimeSound);
    float t = sin(TimeSound * 4);
    vib2 = (b - a) / 2.5 * t;

    float3 c = cross(a, b);
    // vib2 += -t*optix::normalize(c) / 10;

    float bf = bond(p, a - vib2, b + vib2, 0.1);
    float len = length(b - a);
    len = 1;

    float f1 = length(p - (a)) - rad1 / 2;// *len* (abs(t)+1)/6; //first sphere //sdfMicrostructure2 (p-a, len*rad1 * (abs(t)+1)/6); //
    float f2 = length(p - (b)) - rad2 / 2;//*len* (abs(t)+1)/6; //second sphere   //sdfMicrostructure2 (p-b, len*rad1 * (abs(t)+1)/6);//

   /* if (f1 < 0.5) {
        if (TimeSound > 2)
            f1 = sdfMicrostructure4(p - a, rad1);
    }
    if (f2 < 0.5) {
        if (TimeSound > 2)
            f2 = sdfMicrostructure4(p - b, rad2);
    }*/

    //-----
    float f3 = length(p - (b + vib2)) - rad2 / 4;
    float f4 = length(p - (a - vib2)) - rad1 / 4;

    //float f3 = sdfMicrostructure2(p - (b + vib2), rad2/2);
   //float f4 = sdfMicrostructure2(p - (a - vib2), rad1/2);

    f1 = abs(t)*f3 + (1 - abs(t))*f1;
    f2 = abs(t)*f4 + (1 - abs(t))*f2;

    float f = bf;
    if (f1 <= 0.5)
        f = sdf_opSmoothUnion(f1, f, 0.1);
    if (f2 <= 0.5)
        f = sdf_opSmoothUnion(f2, f, 0.1);

    float dB = length(p - (a + b) / 2) - rad2;
    if (dB <= 0.5) {
        float mod2 = sdfMicrostructure4(p - (a + b) / 2, rad2); //sdf_opSmoothUnion(f3, f4, 0.2); //sdfBondSphereBlob(p, a, b, rad1, rad2);
        f = abs(t2)*f + (1 - abs(t2))*mod2;
    }
    else {
        if (TimeSound < 1.0)
            f = abs(t2)*f + (1 - abs(t2))*dB;
    }
    return f;
}

/*------------*/
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

/* For multi-scale molecule*/
RT_CALLABLE_PROGRAM float sdfMicroCell(float3 p, float3 b, float3 c, float3 d, float3 e, float rad1, float rad2)
{
    float3 a = (b + c + d + e) / 4.0;
    float f1 = length(p - a) - rad1 / 2;
    float f;
    //if (TimeSound < 1.0) {
    float bf1 = bond(p, a, b, 0.1);
    float bf2 = bond(p, a, c, 0.1);
    float bf3 = bond(p, a, d, 0.1);
    float bf4 = bond(p, a, e, 0.1);

    float f2 = length(p - b) - rad2 / 2;
    float f3 = length(p - c) - rad2 / 2;
    float f4 = length(p - d) - rad2 / 2;
    float f5 = length(p - e) - rad2 / 2;

    f = sdf_opSmoothUnion(bf2, bf1, 0.1);
    f = sdf_opSmoothUnion(f, bf3, 0.1);
    f = sdf_opSmoothUnion(f, bf4, 0.1);
    f = sdf_opSmoothUnion(f, f1, 0.4);
    f = sdf_opSmoothUnion(f, f2, 0.1);
    f = sdf_opSmoothUnion(f, f3, 0.1);
    f = sdf_opSmoothUnion(f, f4, 0.1);
    f = sdf_opSmoothUnion(f, f5, 0.1);
    //  }
    //  else f = f1;

      //--------first frame
    float totalRad = length(sCell2 - sCell3) / 2 - 3.3;
    float inputRad = abs(abs(sCell2.z - sCell3.z) / 2 - 9.9);
    float cel1Rad = 3.3 * 2;// 1.65 + 3.3;
    float cel1Rad2 = 3.3 + 1.65;

    float t = clamp(TimeSound, 1.0, 4.0);// -1.0;
    float3 cCell3 = sCell3 - make_float3(3.3*t, 3.3, 3.3);
    float3 cCell2 = sCell2 + make_float3(3.3*t, 3.3, 3.3*t);
    float3 cCell1 = sCell1 - make_float3(3.3*t, 3.3, 3.3);

    float cel1Rad3 = 3.3 + 1.65*t;

    float3 cCell4 = (cCell3 + cCell1) / 2;

    int comp1 = 0;

    if ((length(p - cCell1) - cel1Rad) <= 0)
        comp1 = 1;
    if ((length(p - cCell2) - cel1Rad2) <= 0)
        comp1 = 2;
    if ((length(p - cCell3) - cel1Rad) <= 0)
        comp1 = 3;
    //if ((length(p - cCell4) - cel1Rad) <= 0)
    //    comp1 = 4;

    float rad = length(a - b);
    if ((length(a - cCell4) - cel1Rad3 / 2 - rad) <= 0) //-rad fix
        comp1 = 4;

    if ((length(a - cCell2) - cel1Rad2 + rad) <= 0)
        comp1 = 5;

    if (comp1 > 0)
    {
        //check for central only

        float bB = 3.3f / 2.0f;

        float rB = length(make_float3(bB));
        // float3 bmax = a + make_float3(bB);
         //float3 bmax = a + make_float3(bB);
         //float rB = length(bmax);
        float3 nb = normalize(b - a);
        float lb = length(b - a);
        float3 cb = a + nb*(rB - lb);

        float ft1 = length(p - cb) - rad2 / 2;
        float bt1 = bond(p, b, cb, 0.1);

        //----------
        float3 nc = normalize(c - a);
        float3 cc = a + nc*(rB - lb);

        float ft2 = length(p - cc) - rad2 / 2;
        float bt2 = bond(p, c, cc, 0.1);

        //----------
        float3 nd = normalize(d - a);
        float3 cd = a + nd*(rB - lb);

        float ft3 = length(p - cd) - rad2 / 2;
        float bt3 = bond(p, d, cd, 0.1);

        //----------
        float3 ne = normalize(e - a);
        float3 ce = a + ne*(rB - lb);

        float ft4 = length(p - ce) - rad2 / 2;
        float bt4 = bond(p, e, ce, 0.1);

        float fb = f;
        if (comp1 == 5) {
            fb = Tetra(p, b, c, d, e);
            fb = sdf_opSmoothUnion(fb, f, 0.4);
        }
        if (comp1 == 4)
        {
            float blScale = 1.0;
            float3 c1 = a;

            c1.x += bB;
            float rad = length(a - b);
            fb = length(p - a) - bB / 2;// sdfMicrostructure4(p - a, bB);

            if ((length(c1 - cCell4) - cel1Rad3 / 2) <= 0)
            {
                float f1 = length(p - c1) - bB / 4;
                fb = sdf_opSmoothUnion(fb, f1, blScale);
            }
            c1 = a;
            c1.x -= bB;
            if ((length(c1 - cCell4) - cel1Rad3 / 2) <= 0)
            {
                float f1 = length(p - c1) - bB / 4;
                fb = sdf_opSmoothUnion(fb, f1, blScale);
            }
            c1 = a;
            c1.y += bB;

            if ((length(c1 - cCell4) - cel1Rad3 / 2) <= 0)
            {
                float f1 = length(p - c1) - bB / 4;
                fb = sdf_opSmoothUnion(fb, f1, blScale);
            }

            c1 = a;
            c1.y -= bB;

            if ((length(c1 - cCell4) - cel1Rad3 / 2) <= 0)
            {
                float f1 = length(p - c1) - bB / 4;
                fb = sdf_opSmoothUnion(fb, f1, blScale);
            }
            c1 = a;
            c1.z += bB;

            if ((length(c1 - cCell4) - cel1Rad3 / 2) <= 0)
            {
                float f1 = length(p - c1) - bB / 4;
                fb = sdf_opSmoothUnion(fb, f1, blScale);
            }
            c1 = a;
            c1.z -= bB;

            if ((length(c1 - cCell4) - cel1Rad3 / 2) <= 0)
            {
                float f1 = length(p - c1) - bB / 4;
                fb = sdf_opSmoothUnion(fb, f1, blScale);
            }

            c1 = cb;

            if ((length(c1 - cCell4) - cel1Rad3 / 2) <= 0)
            {
                float f1 = length(p - c1) - bB / 4;
                fb = sdf_opSmoothUnion(fb, f1, blScale);
            }

            c1 = cd;

            if ((length(c1 - cCell4) - cel1Rad3 / 2) <= 0)
            {
                float f1 = length(p - c1) - bB / 4;
                fb = sdf_opSmoothUnion(fb, f1, blScale);
            }

            c1 = cc;

            if ((length(c1 - cCell4) - cel1Rad3 / 2) <= 0)
            {
                float f1 = length(p - c1) - bB / 4;
                fb = sdf_opSmoothUnion(fb, f1, blScale);
            }
            c1 = ce;

            if ((length(c1 - cCell4) - cel1Rad3 / 2) <= 0)
            {
                float f1 = length(p - c1) - bB / 4;
                fb = sdf_opSmoothUnion(fb, f1, blScale);
            }

            //---------------
            float sphere1 = fb;
            float tm = 2;

            float3 scale = 0.3*make_float3(abs(sin(tm)), abs(cos(tm)), abs(cos(tm)));
            float dens_scale = abs(cos(tm)) / 2;

            float fb2 = fb + 0.1;
            float shell = max(fb, -fb2);

            fb = shell;
            /*
            float3 tiled = make_float3(dens_scale);
            float3 tiled2 = 0.2 + tiled;

            float3 x = p + 0.5*tiled;
            // x - y * floor(x / y).
            float3 mod = x - tiled2*floor(x / tiled2);//modf(p + 0.5*tiled, tiled)
            float3 inX = mod - 0.5*tiled2;

            float3 c = make_float3(0., 0., 0.03 + 0.06*dens_scale);
            float cyly = length(make_float2(inX.x, inX.z) - make_float2(c.x, c.y)) - c.z;
            float cylx = length(make_float2(inX.y, inX.z) - make_float2(c.x, c.y)) - c.z;
            float cylz = length(make_float2(inX.x, inX.y) - make_float2(c.x, c.y)) - c.z;

            float mics = sminp(cylx, sminp(cyly, cylz, 0.08), 0.08);

            fb = sminp(shell, mics, 0.1);
            fb = max(max(fb, sphere1), x.z);*/
        }
        float dB = sdf_opSmoothUnion(fb, ft1, 0.1);
        dB = min(dB, bt1);

        dB = sdf_opSmoothUnion(dB, ft2, 0.1);
        dB = min(dB, bt2);

        dB = sdf_opSmoothUnion(dB, ft3, 0.1);
        dB = min(dB, bt3);

        dB = sdf_opSmoothUnion(dB, ft4, 0.1);
        dB = min(dB, bt4);

        if (TimeSound < 1.0) {
            float t2 = TimeSound;
            f = abs(t2)*dB + (1 - abs(t2))*f;
        }
        else f = dB;
    }

    if (comp1 > 1) {
        float bB = 3.3f / 2.0f;

        float3 cb1 = a;
        float3 cb = a;
        cb.y += bB;
        cb1.y -= bB;

        float bt1 = bond(p, cb1, cb, 0.1);
        float ft3 = length(p - cb1) - rad2 / 2;
        float ft2 = length(p - cb) - rad2 / 2;
        //float fb = Tetra(p, b, c, d, e);
        float dB = sdf_opSmoothUnion(f, bt1, 0.2);
        dB = sdf_opSmoothUnion(dB, ft2, 0.2);
        dB = sdf_opSmoothUnion(dB, ft3, 0.2);
        if (TimeSound < 1.0) {
            float t2 = TimeSound;
            f = abs(t2)*dB + (1 - abs(t2))*f;
        }
        else f = dB;
    }
    if (comp1 > 2) {
        float bB = 3.3f / 2.0f;

        float3 cb1 = a;
        float3 cb = a;
        cb.x += bB;
        cb1.x -= bB;

        float bt1 = bond(p, cb1, cb, 0.1);
        float ft3 = length(p - cb1) - rad2 / 2;
        float ft2 = length(p - cb) - rad2 / 2;
        float dB = sdf_opSmoothUnion(f, bt1, 0.2);
        dB = sdf_opSmoothUnion(dB, ft2, 0.2);
        dB = sdf_opSmoothUnion(dB, ft3, 0.2);
        if (TimeSound < 1.0) {
            float t2 = TimeSound;
            f = abs(t2)*dB + (1 - abs(t2))*f;
        }
        else f = dB;
    }

    return f;
}

RT_CALLABLE_PROGRAM float sdfMolBondSphere(float3 p, float3 a, float3 b, float3 c, float rad1, float rad2, float rad3)
{
    //first vibration vector in molecule
    float3 vib2[3];
    float3 vib3[3];
    float f1[3];
    float f2[3];
    float f3[3];

    float bf[3];
    float bf2[3];

    vib2[0] = (b - a) / 3.5 * -cos(TimeSound * 20);
    vib3[0] = (c - a) / 3.5 * (cos(TimeSound * 20));

    vib2[1] = (b - a) / 3.5 * sin(TimeSound * 20);
    vib3[1] = (c - a) / 3.5 * (sin(TimeSound * 20));

    float3 vib_dir = b - 2 * a + c;

    vib2[2] = vib_dir / 3.5 * sin(TimeSound * 20);
    vib3[2] = vib_dir / 3.5 * (sin(TimeSound * 20));

    float f[3];
    float num = 3;
    if (MultiscaleParam <= 1)  num = 1;
    else
    {
        if (MultiscaleParam <= 2)
            num = 2;
    }
    for (int i = 0; i < num; i++)
    {
        //computation of atoms
        f1[i] = length(p - a) - rad1 / 2;
        f2[i] = length(p - (b + vib2[i])) - rad2 / 2;
        f3[i] = length(p - (c + vib3[i])) - rad3 / 2;

        //chose subspace

        f[i] = fminf(fminf(f1[i], f2[i]), f3[i]);

        if (f[i] == f1[i]) //trace central part
        {
            //f[i]+=rad1/2; //get proper radius
            bf[i] = bond(p, a, (b + vib2[i]), 0.06);
            bf2[i] = bond(p, a, (c + vib3[i]), 0.06);
            f[i] = sdf_opSmoothUnion(f[i], bf[i], 0.09);
            f[i] = sdf_opSmoothUnion(f[i], bf2[i], 0.09);
        }
        else {
            if (f[i] == f2[i]) //trace left part
            {
                //f[i]+=rad2/2; //get proper radius
                bf[i] = bond(p, a, (b + vib2[i]), 0.06);
                f[i] = sdf_opSmoothUnion(f[i], bf[i], 0.09);
            }
            else {
                //f[i]+=rad3/2; //get proper radius
                bf2[i] = bond(p, a, (c + vib3[i]), 0.06);
                f[i] = sdf_opSmoothUnion(f[i], bf2[i], 0.09);
            }
        }
    }
    float mod1 = f[0];
    if (MultiscaleParam <= 1)
    {
        mod1 = f[0];
    }
    else
    {
        if (MultiscaleParam <= 2)
        {
            mod1 = sdf_opSmoothUnion(f[0], f[1], 0.1);
        }
        else
        {
            if (MultiscaleParam <= 3)
            {
                mod1 = sdf_opSmoothUnion(f[0], f[1], 0.1);
                mod1 = sdf_opSmoothUnion(mod1, f[2], 0.1);
            }
        }
    }

    // mod1 = sdf_opSmoothUnion(f[0], f[1], 0.1);
    // mod1 = sdf_opSmoothUnion(mod1, f[2], 0.1);

    float mod2 = sdfBondSphereMolBlob(p, a, b, c, rad1, rad2, rad3);

    //manual interpolation
    //return mod1*MultiscaleParam +(1.0-MultiscaleParam)*mod2;
    float dist = length(sysCameraPosition - (a + b + c) / 3);

    if (dist < 20.0)
    {
        //consider the center=0;
       // float3 camV = make_float3(0) - sysCameraPosition;
      //  float3
      //  if (dot(sysCameraPosition, (a + b) / 2) > 0)
        {
            float interp = (dist - 10) / 10.0;
            float d = optix::clamp(interp, 0.0, 1.0);
            return mod2*d + (1.0 - d)*mod1;
        }
        //    else return mod2;
    }
    else
        return mod2;
    //return sdf_opSmoothUnion(f2, f1, 0.2);
}

RT_CALLABLE_PROGRAM float sdfDynSphere(float3 p, float3 p2, float3 rad)
{
    //float3 pp = p2*TimeSound + (1.0 - TimeSound)*p;

    float f1 = length(p) - rad.x;
    float f2 = length(p2) - rad.x;// - make_float3(0.5, 0, 0)
    float f = sdf_opSmoothUnion(f1, f2, 0.7);
    for (int i = 0; i <= 10; i++)
    {
        float3 pp = p2*i / 10.0 + (1.0 - i / 10.0)*p;
        f1 = length(pp) - rad.x;
        f = sdf_opSmoothUnion(f, f1, 0.7);
    }
    return  f;//f2*TimeSound + (1.0 - TimeSound)*f1;
}

RT_CALLABLE_PROGRAM float sdfDynSphere2(float3 p, float3 p2, float3 rad)
{
    float3 pp = p2*TimeSound + (1.0 - TimeSound)*p;
    return  length(pp) - rad.x;
}

//primitives
RT_CALLABLE_PROGRAM float sdSphere(float3 p, float3 rad)
{
    return length(p - varCenter) - varRadius.x;
}

RT_CALLABLE_PROGRAM float sdfHand(float3 p, float3 rad)
{
    float3 shift = make_float3(-1, -1, -1);
    float f_m = length(p - (varCenter + shift)) - varRadius.x / 14;
    float f_0 = length(p - (varCenter0 + varCenter) / 2 - shift) - varRadius.x / 16;
    float f_1 = length(p - (varCenter1 + varCenter) / 2 - shift) - varRadius.x / 16;
    float f_2 = length(p - (varCenter2 + varCenter) / 2 - shift) - varRadius.x / 16;
    float f_3 = length(p - (varCenter3 + varCenter) / 2 - shift) - varRadius.x / 16;
    float f_4 = length(p - (varCenter4 + varCenter) / 2 - shift) - varRadius.x / 16;

    float3 b = (varCenter0 + varCenter) / 2.0 + shift;
    f_m = bond(p, varCenter + shift, b, 0.02);
    f_m = sdf_opSmoothUnion(f_m, f_0, 0.1);

    f_m = sdf_opSmoothUnion(f_m, f_1, 0.1);
    b = (varCenter1 + varCenter) / 2.0 + shift;
    f_m = sdf_opSmoothUnion(f_m, bond(p, varCenter + shift, b, 0.02), 0.1);

    f_m = sdf_opSmoothUnion(f_m, f_2, 0.1);
    b = (varCenter2 + varCenter) / 2.0 + shift;
    f_m = sdf_opSmoothUnion(f_m, bond(p, varCenter + shift, b, 0.02), 0.1);

    f_m = sdf_opSmoothUnion(f_m, f_3, 0.1);
    b = (varCenter3 + varCenter) / 2.0 + shift;
    f_m = sdf_opSmoothUnion(f_m, bond(p, varCenter + shift, b, 0.02), 0.1);

    f_m = sdf_opSmoothUnion(f_m, f_4, 0.1);
    b = (varCenter4 + varCenter) / 2.0 + shift;
    f_m = sdf_opSmoothUnion(f_m, bond(p, varCenter + shift, b, 0.02), 0.1);

    return f_m;
}

RT_CALLABLE_PROGRAM float sdfMicrostructure_test(float3 pos, float3 rad)
{
    float radc = 5.0;
    float scale = 1.8;
    float3 cent1 = make_float3(radc, 0, 0);
    float3 cent2 = make_float3(-radc, 0, 0);
    float3 cent3 = make_float3(0, radc, 0);
    float3 cent4 = make_float3(0, -radc, 0);
    float d1 = length(pos - cent1) - radc / scale;
    float d2 = length(pos - cent2) - radc / scale;
    float d3 = length(pos - cent3) - radc / scale;
    float d4 = length(pos - cent4) - radc / scale;

    float d5 = min(d1, d2);
    d5 = min(d5, d3);
    d5 = min(d5, d4);
    d5 -= scale * 1.5;
    d5 = -d5;

    float f = min(d5, d4);
    f = min(f, d3);
    f = min(f, d2);
    f = min(f, d1);

    float3 b = make_float3(5.0);// optix::make_float3(rad);
    float3 d = abs(pos) - b;
    float box = length(max(d, make_float3(0.0f)))
        + min(max(d.x, max(d.y, d.z)), 0.0f);
    f = max(f, box);
    return box;
}

RT_CALLABLE_PROGRAM float sdfMicrostructure(float3 pp, float3 rad)
{
    float t = TimeSound * 4;

    float3 p = pp - varCenter;
    float3 scale = 0.3*make_float3(abs(sin(t)), abs(cos(t)), abs(cos(t)));
    float dens_scale = abs(cos(t)) / 2;

    // float sphere1 = optix::length(p) - rad.x;// sdSphere(p, rad);
    // float sphere2 = optix::length(p) - rad.x / 2.2;// sdSphere(p, rad / 20);

    float rad2 = varRadius.x - 0.1;
    float sphere1 = optix::length(p) - rad2;
    float sphere2 = optix::length(p) - (rad2 - 0.1);
    float shell = max(sphere1, -sphere2);
    shell = max(shell, p.z);

    float3 tiled = make_float3(dens_scale);
    float3 tiled2 = 0.2 + tiled;

    float3 x = p + 0.5*tiled;
    // x - y * floor(x / y).
    float3 mod = x - tiled2*floor(x / tiled2);//modf(p + 0.5*tiled, tiled)
    float3 inX = mod - 0.5*tiled2;

    float3 c = make_float3(0., 0., 0.03 + 0.06*dens_scale);
    float cyly = length(make_float2(inX.x, inX.z) - make_float2(c.x, c.y)) - c.z;
    float cylx = length(make_float2(inX.y, inX.z) - make_float2(c.x, c.y)) - c.z;
    float cylz = length(make_float2(inX.x, inX.y) - make_float2(c.x, c.y)) - c.z;

    float mics = sminp(cylx, sminp(cyly, cylz, 0.08), 0.08);

    float res = sminp(shell, mics, 0.1);
    res = max(max(res, sphere1), x.z);
    // res = max(res, x.z);
    return res;
}

RT_CALLABLE_PROGRAM float sdfBox(float3 p, float3 rad)
{
    float3 b = varRadius;// optix::make_float3(rad);
    float3 d = abs(p - varCenter) - b;
    return length(max(d, make_float3(0.0f)))
        + min(max(d.x, max(d.y, d.z)), 0.0f); // remove this line for an only partially signed sdf
}
RT_CALLABLE_PROGRAM float sdfRoundBox(float3 p, float3 rad)
{
    float3 b = rad;// optix::make_float3(rad);
    float r = 0.1;
    float3 d = abs(p - varCenter) - b;
    return length(max(d, make_float3(0.0f))) - r
        + min(max(d.x, max(d.y, d.z)), 0.0f); // remove this line for an only partially signed sdf
}

RT_CALLABLE_PROGRAM float sdfTorus(float3  p, float3 rad)
{
    float2 t = varT;
    float3 pp = p - varCenter;
    float2 xz = make_float2(pp.x, pp.z);
    float2 q = make_float2(length(xz) - t.x, pp.y);
    return length(q) - t.y;
}

//operation
RT_CALLABLE_PROGRAM float opRound(float3 p, float3 rad)
{
    //TODO: the rad should be from primitive
    return sdfOpPrim(p, rad) - varK;
}

RT_CALLABLE_PROGRAM float opElongate(float3 p, float3 rad)
{
    float3 h = varRadius;
    float3 q = p - clamp(p, -h, h);
    //should be initial primitive Rad
    return sdfOpPrim(q, rad);
}

RT_CALLABLE_PROGRAM float opSmoothUnion(float3 p, float3 rad) {
    float k = varK;
    float d1 = sdfOpPrim(p, rad);
    float d2 = sdfOpPrim2(p, rad);
    float val = 0.5f + 0.5f*(d2 - d1) / k;
    float h = clamp(val, 0.0f, 1.0f);
    float mix_res = (1.0 - h)*d2 + d1*h;
    return mix_res - k*h*(1.0 - h);
}

RT_CALLABLE_PROGRAM float opSmoothIntersection(float3 p, float3 rad) {
    float k = varK;
    float d1 = sdfOpPrim(p, rad);
    float d2 = sdfOpPrim2(p, rad);
    float val = 0.5f - 0.5f*(d2 - d1) / k;
    float h = clamp(val, 0.0f, 1.0f);
    float mix_res = (1.0 - h)*d2 + d1*h;
    return mix_res + k*h*(1.0 - h);
}

RT_CALLABLE_PROGRAM float opSmoothSubtraction(float3 p, float3 rad) {
    float k = varK;
    float d1 = sdfOpPrim(p, rad);
    float d2 = sdfOpPrim2(p, rad);
    float val = 0.5f - 0.5f*(d2 - d1) / k;
    float h = clamp(val, 0.0f, 1.0f);
    float mix_res = (1.0 - h)*d2 - d1*h;
    return mix_res + k*h*(1.0 - h);
}

RT_CALLABLE_PROGRAM float opSubtraction(float3 p, float3 rad)
{
    float d1 = sdfOpPrim2(p, rad);
    float d2 = sdfOpPrim(p, rad);
    return max(-d1, d2);
}

RT_CALLABLE_PROGRAM float sdfField(float3 p, float3 rad) {
    float s1;

    switch (numTexDefined) {
    case 1:
    {
        s1 = tex3D<float>(texSDF0, p.x*0.5f + 0.5f, p.y*0.5f + 0.5f, p.z*0.5f + 0.5f);
        //trace back to iso value and shift

        s1 += shift0;
        //dist=index*spacing
        // and devide by texture size so 0,1 range

        //Chamber distance neglect spacing.
        //Distance is computed in voxels

        s1 /= size0;// 138.0f;
                    //s1 /= size;
        s1 *= 2.0f;
        break;
    }
    case 2:
    {
        float s_min = tex3D(texSDF0, p.x*0.5f + 0.5f, p.y*0.5f + 0.5f, p.z*0.5f + 0.5f);
        float s_max = tex3D(texSDF1, p.x*0.5f + 0.5f, p.y*0.5f + 0.5f, p.z*0.5f + 0.5f);

        s1 = TimeSound*s_max + (1.0 - TimeSound)*s_min;//trace back to iso value and shift
        float shift = TimeSound*shift1 + (1 - TimeSound)*shift0;
        float size = TimeSound*size1 + (1 - TimeSound)*size0;

        s1 += shift;
        //dist=index*spacing
        // and devide by texture size so 0,1 range

        //Chamber distance neglect spacing.
        //Distance is computed in voxels

        //s1 /= 139.0f;
        s1 /= size;
        s1 *= 2.0f;
        break;
    }
    }
    //float size of texture is currently unsused

    return s1;
}

/* ------------
/* for heterogeneous objects
--------------*/

//Molecules
RT_CALLABLE_PROGRAM float sdfAtom(float3 p, primParamDesc desc)
{
    float3 pos = desc.pos[0];
    float rad1 = desc.rad[0];
    return length(p - pos) - rad1;
}

//Microstructure like test
RT_CALLABLE_PROGRAM float sdfMicro(float3 pp, primParamDesc desc)
{
    float3 cent = desc.pos[0];
    float rad1 = desc.rad[0];

    float t = TimeSound * 4;

    float3 p = pp - cent;
    float3 scale = 0.3*make_float3(abs(sin(t)), abs(cos(t)), abs(cos(t)));
    float dens_scale = abs(cos(t)) / 2;

    // float sphere1 = optix::length(p) - rad.x;// sdSphere(p, rad);
    // float sphere2 = optix::length(p) - rad.x / 2.2;// sdSphere(p, rad / 20);

    float rad2 = rad1 - 0.1;
    float sphere1 = optix::length(p) - rad2;
    float sphere2 = optix::length(p) - (rad2 - 0.1);
    float shell = max(sphere1, -sphere2);
    shell = max(shell, p.z);

    float3 tiled = make_float3(dens_scale);
    float3 tiled2 = 0.2 + tiled;

    float3 x = p + 0.5*tiled;
    // x - y * floor(x / y).
    float3 mod = x - tiled2*floor(x / tiled2);//modf(p + 0.5*tiled, tiled)
    float3 inX = mod - 0.5*tiled2;

    float3 c = make_float3(0., 0., 0.03 + 0.06*dens_scale);
    float cyly = length(make_float2(inX.x, inX.z) - make_float2(c.x, c.y)) - c.z;
    float cylx = length(make_float2(inX.y, inX.z) - make_float2(c.x, c.y)) - c.z;
    float cylz = length(make_float2(inX.x, inX.y) - make_float2(c.x, c.y)) - c.z;

    float mics = sminp(cylx, sminp(cyly, cylz, 0.08), 0.08);

    float res = sminp(shell, mics, 0.1);
    res = max(max(res, sphere1), x.z);
    // res = max(res, x.z);
    return res;
}

//----Bonding
RT_CALLABLE_PROGRAM float sdfBondBlob(float3 p, primParamDesc desc)
{
    float3 a = desc.pos[0];
    float rad1 = desc.rad[0];
    float3 b = desc.pos[1];
    float rad2 = desc.rad[1];
    float bf = bond(p, a, b, 0.1);
    float f1 = length(p - a) - rad1 / 2;
    float f2 = length(p - b) - rad2 / 2;

    float f = sdf_opSmoothUnion(f1, bf, 0.4);
    return sdf_opSmoothUnion(f2, f, 0.4);
    //return sdf_opSmoothUnion(f2, f1, 0.8);
}