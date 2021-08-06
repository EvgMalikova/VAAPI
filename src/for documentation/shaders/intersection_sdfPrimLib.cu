#include <optix_math.h>

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

//------------------------------------
// list of all used or unsued parameters

rtDeclareVariable(float3, varRadius, , );
rtDeclareVariable(float3, varCenter, , );
rtDeclareVariable(float, varK, , );
rtDeclareVariable(float2, varT, , );

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

RT_CALLABLE_PROGRAM float sdf_opSmoothUnion(float d1, float d2) {
    float k = 0.7;

    float val = 0.5f + 0.5f*(d2 - d1) / k;
    float h = clamp(val, 0.0f, 1.0f);
    float mix_res = (1.0 - h)*d2 + d1*h;
    return mix_res - k*h*(1.0 - h);
}

RT_CALLABLE_PROGRAM float sdfBondSphere(float3 p, float3 a, float3 b, float rad1, float rad2)
{
    //float bf = bond(p, a, b, 0.1);
    float f1 = length(p - a) - rad1;
    float f2 = length(p - b) - rad2;

    //float f = sdf_opSmoothUnion(f1, bf);
    //return sdf_opSmoothUnion(f2, f);
    return sdf_opSmoothUnion(f2, f1);
}

RT_CALLABLE_PROGRAM float sdfDynSphere(float3 p, float3 p2, float3 rad)
{
    //float3 pp = p2*TimeSound + (1.0 - TimeSound)*p;

    float f1 = length(p) - rad.x;
    float f2 = length(p2) - rad.x;// - make_float3(0.5, 0, 0)
    float f = sdf_opSmoothUnion(f1, f2);
    for (int i = 0; i <= 10; i++)
    {
        float3 pp = p2*i / 10.0 + (1.0 - i / 10.0)*p;
        f1 = length(pp) - rad.x;
        f = sdf_opSmoothUnion(f, f1);
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