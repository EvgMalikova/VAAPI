/*
Basic headers for callable primitive programs
*/
//-----------------
//for sphere tracing of various primitives
//for SDF, todo
typedef rtCallableProgramId<float(float3, primParamDesc)> callT;
rtDeclareVariable(callT, sdfPrimDefault, , );

typedef rtCallableProgramId<float(float3, float)> callT0;
rtDeclareVariable(callT0, sdfPrim0, , );

typedef rtCallableProgramId<float(float3, float3, float3, float, float)> callT1;
rtDeclareVariable(callT1, sdfPrim1, , );

typedef rtCallableProgramId<float(float3, float3, float3, float3, float3)> callT4;
rtDeclareVariable(callT4, sdfPrim4, , );

typedef rtCallableProgramId<float(float3, float3, float3, float3, float, float, float)> callT3;
rtDeclareVariable(callT3, sdfPrim3, , );