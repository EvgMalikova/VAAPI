/*
 * Copyright (c) 2013-2018, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "../per_ray_data.h"
#include "../../basic_lights.h"

rtDeclareVariable(PerRayData, thePrd, rtPayload, );
rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );

//array of lights
//rtBuffer<BasicLight> lights;

rtDeclareVariable(float3, sysBackground, , );

//for sdf
typedef rtCallableProgramId<float(float3, float3)> callTBackSDF;
rtDeclareVariable(callTBackSDF, sdfPrimBack, , );

typedef rtCallableProgramId<float(float3, float3, float3, float, float)> callT4;
rtDeclareVariable(callT4, sdfPrim4, , );

inline __device__
float3 pal(float t, float3 a, float3 b, float3 c, float3 d)
{
    float3 x = 6.28318f*(c*t + d);
    x.x = cosf(x.x);
    x.y = cosf(x.y);
    x.z = cosf(x.z);

    return a + b*x;
}

__device__ void render_Surface21_old()
{
    float Ka = 0.5;
    float Kd = 0.5;
    float Ks = 0.2;
    float4 col = make_float4(0, 0, 0, 1);// translucent_grays(0.5, 0.1, 0);

    float tstep = 0.01;
    float3 pos = thePrd.last_hit_point - theRay.direction*tstep;// eyeRay.o + eyeRay.d*tnear;
    float3 step = theRay.direction*tstep;

    float4 sum = thePrd.result;// make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here
    BasicLight lights2[2];
    lights2[0].color = optix::make_float3(1.0);
    lights2[0].pos = optix::make_float3(10.0);

    lights2[1].color = optix::make_float3(1.0);
    lights2[1].pos = optix::make_float3(0, 0, 10.0);

    float s1 = sdfPrimBack(pos, make_float3(thePrd.maxDist));
    pos += step;
    float s2 = sdfPrimBack(pos, make_float3(thePrd.maxDist));
    float i = 0;
    float trp = 0.1;
    float max = thePrd.maxDist * 2 + 0.4; //bounding box size
    float3 color = Ka *  make_float3(0, 1, 0);// ambient_light_color;
    float3 color2 = Ka *  make_float3(0, 1, 0);                                          //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

    for (int i = 0; i < 2; ++i)
    {
        BasicLight light = lights2[i];
        float3 L = optix::normalize(light.pos - thePrd.last_hit_point);
        float nDl = optix::dot(thePrd.normal, L);

        //if (nDl > 0)
        //    color += Kd * nDl * light.color; // make_float3(1.0);//

        float phong_exp = 0.1;
        if (nDl > 0) {
            color += Kd * nDl * light.color;

            /*  optix::float3 H = optix::normalize(L - theRay.direction);
            float nDh = optix::dot(normal, H);
            if (nDh > 0)
            color += Ks * light.color * pow(nDh, phong_exp);
            */
        }
    }

    while (i < max) //s2 < 0.01)
    {
        if (s2 < 0.01) {
            //col *= Ka;
            //col.w = 0.5; //s1 is very small

            //initial blend
            if (col.x == 0)
                col = make_float4(color);
            else
                col = make_float4(color2);
            col.w = trp;
            col.x *= col.w;
            col.y *= col.w;
            col.z *= col.w;
            // "over" operator for front-to-back blending
            sum = sum + col*(1.0f - sum.w);
        }
        i += tstep;
        s1 = s2;
        pos += step;

        if (sum.w >= 1.0) i = max + 1;
        else
            s2 = sdfPrimBack(pos, make_float3(thePrd.maxDist)); //interpolateSDF(time, pos, texSDF, texSDF_F);
                                                            // if (abs(s1) <= eps)
    }
    //or it should be
    // thePrd.radiance = make_float3(sum);
   // thePrd.radiance += make_float3(sum);
    thePrd.result = sum;
}
__device__ float  compute_SDF(optix::float3 pos, cellPrimDesc cell, int type)
{
    if (type == 0) {
        return sdfPrimBack(pos, make_float3(1.1));
    }
    else
    {
        return sdfPrim4(pos, cell.center, cell.center2, cell.rad1, cell.rad2);
    }
}
__device__ void  VolumeRaycast(int pN)
{
    float Ka = 0.5;
    float Kd = 0.2;
    float Ks = 0.2;
    float4 col = make_float4(0, 0, 0, 1);// translucent_grays(0.5, 0.1, 0);

    float tstep = 0.005;

    cellPrimDesc cell = thePrd.cellPrimitives[pN];
    float dist = cell.intersectionDist;
    //---------------
    float3 pos = theRay.origin + theRay.direction*dist;
    float3 step = theRay.direction*tstep;

    float4 sum = thePrd.result;// make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here

    if (sum.w >= 1.0) return;
    //compute color

    BasicLight lights2[2];
    lights2[0].color = optix::make_float3(1.0);
    lights2[0].pos = optix::make_float3(10.0);

    lights2[1].color = optix::make_float3(1.0);
    lights2[1].pos = optix::make_float3(0, 0, 10.0);

    float s1 = compute_SDF(pos, cell, cell.type); //sdfPrimBack(pos, make_float3(1.1));
    if (abs(s1) > tstep)
        step = theRay.direction*abs(s1);
    else step = theRay.direction*tstep;

    pos += step;
    float s2 = compute_SDF(pos, cell, cell.type); //sdfPrimBack(pos, make_float3(1.1));

    //creates a holes of the molecule form
    //max = abs(distF - dist);

    float3 color = Ka *  make_float3(cell.color.x, cell.color.y, cell.color.z);// ambient_light_color;
    float3 color2 = color;                                          //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

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
        }
    }

    float i = 0;
    float trp = cell.color.w;
    float max = cell.maxDist; //bounding box size

    //start volume integration
    while (i < max) //s2 < 0.01)
    {
        if (s2 < 0.0) {
            if (abs(s1) > tstep) //sum transparency
            {
                //col *= Ka;
               //col.w = 0.5; //s1 is very small
               // float3 color = Ka *  make_float3(0, 1, 0);// ambient_light_color;

    //initial blend
                if (col.x == 0)
                    col = make_float4(color);
                else
                    col = make_float4(color2);

                //Beer–Lambert law
                float F = exp(-trp*abs(s1) * 200);
                col = col*(1.0 - F);
                sum = sum + col*(1.0f - sum.w);

                // tracedDist += abs(s1);
            }
            else {
                //Beer–Lambert law
                //col = make_float4(color);
               // col = col*exp(-trp*abs(s1) / 100);
               // tracedDist = 0;
            }
        }

        s1 = s2;
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
        if (sum.w >= 1.0) i = max + 1;
        else
            s2 = compute_SDF(pos, cell, cell.type);// sdfPrimBack(pos, make_float3(thePrd.maxDist)); //interpolateSDF(time, pos, texSDF, texSDF_F);
    }
    thePrd.result = sum;
}
__device__ void  VolumeRaycast(int pN, int fN)
{
    float Ka = 0.5;
    float Kd = 0.2;
    float Ks = 0.2;
    float4 col = make_float4(0, 0, 0, 1);// translucent_grays(0.5, 0.1, 0);

    float tstep = 0.005;

    cellPrimDesc cell = thePrd.cellPrimitives[pN];
    cellPrimDesc cellSec = thePrd.cellPrimitives[fN];
    float dist = cellSec.intersectionDist;
    //---------------
    float3 pos = theRay.origin + theRay.direction*dist;
    float3 step = theRay.direction*tstep;

    float4 sum = thePrd.result;// make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here

    if (sum.w >= 1.0) return;
    //compute color

    float s1 = compute_SDF(pos, cell, cell.type); //sdfPrimBack(pos, make_float3(1.1));

    if (abs(s1) > tstep)
        step = theRay.direction*abs(s1);
    else step = theRay.direction*tstep;

    pos += step;
    float s2 = compute_SDF(pos, cell, cell.type); //sdfPrimBack(pos, make_float3(1.1));

    float material = compute_SDF(pos, cellSec, cellSec.type); //sdfPrimBack(pos, make_float3(1.1));
                                                              //creates a holes of the molecule form
    float i = 0;
    float trp = cell.color.w*Ka;
    float max = cellSec.maxDist; //bounding box size

                              //start volume integration
    while (i < max) //s2 < 0.01)
    {
        if (s2 < 0.0) {
            if (abs(s1) > tstep) //sum transparency
            {
                //col *= Ka;
                //col.w = 0.5; //s1 is very small
                // float3 color = Ka *  make_float3(0, 1, 0);// ambient_light_color;

                //initial blend
                if (material > 0) {
                    col = (cellSec.color)*Ka; //(make_float4(0, 0, 1, 0.001) +
                    trp = col.w*Ka;
                }
                else
                    col = cell.color*Ka;

                //Beer–Lambert law
                float F = exp(-trp*abs(s1) * 200);
                col = col*(1.0 - F);
                sum = sum + col*(1.0f - sum.w);

                // tracedDist += abs(s1);
            }
            else {
                //Beer–Lambert law
                //col = make_float4(color);
                // col = col*exp(-trp*abs(s1) / 100);
                // tracedDist = 0;
            }
        }

        s1 = s2;

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
        if (sum.w >= 1.0) i = max + 1;
        else
        {
            s2 = compute_SDF(pos, cell, cell.type);// sdfPrimBack(pos, make_float3(thePrd.maxDist)); //interpolateSDF(time, pos, texSDF, texSDF_F);
            material = compute_SDF(pos, cellSec, cellSec.type); //sdfPrimBack(pos, make_float3(1.1));
        }
    }
    thePrd.result = sum;
}

__device__ float  VolumeIntegration(float startDist, float maxDist, int pN)
{
    float Ka = 0.5;
    float Kd = 0.5;
    float Ks = 0.2;
    float4 col = make_float4(0, 0, 0, 1);// translucent_grays(0.5, 0.1, 0);

    float tstep = 0.005;

    float tracedDist = startDist;
    cellPrimDesc cell = thePrd.cellPrimitives[pN];
    float dist = cell.intersectionDist;
    //---------------
    float3 pos = theRay.origin + theRay.direction*startDist;
    float3 step = theRay.direction*tstep;

    float4 sum = thePrd.result;// make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here
    //break if there is 1 opacity
    if (sum.w >= 1.0) return;
    //compute color

    BasicLight lights2[2];
    lights2[0].color = optix::make_float3(1.0);
    lights2[0].pos = optix::make_float3(10.0);

    lights2[1].color = optix::make_float3(1.0);
    lights2[1].pos = optix::make_float3(0, 0, 10.0);

    float s1 = compute_SDF(pos, cell, cell.type); //sdfPrimBack(pos, make_float3(1.1));
    if (abs(s1) > tstep)
        step = theRay.direction*abs(s1);
    else step = theRay.direction*tstep;

    pos += step;
    float s2 = compute_SDF(pos, cell, cell.type); //sdfPrimBack(pos, make_float3(1.1));

                                                  //creates a holes of the molecule form
                                                  //max = abs(distF - dist);

    float3 color = Ka *  make_float3(cell.color.x, cell.color.y, cell.color.z);// ambient_light_color;
    float3 color2 = color;                                          //	optix::float3 hit_point = theRay.origin + theIntersectionDistance * theRay.direction;

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
        }
    }

    float i = 0;
    float trp = cell.color.w*Ka;

    //start volume integration
    while (i < maxDist) //s2 < 0.01)
    {
        if (s2 < 0.0) {
            if (abs(s1) > tstep) //sum transparency
            {
                //save current position
                tracedDist = startDist + i;
                //col *= Ka;
                //col.w = 0.5; //s1 is very small
                // float3 color = Ka *  make_float3(0, 1, 0);// ambient_light_color;

                //initial blend
               // if (abs(startDist - dist) < 0.005)
               //     col = make_float4(color);
               // else
                col = make_float4(color2);

                //Beer–Lambert law
                float F = exp(-trp*abs(s1) * 100);
                col = col*(1.0 - F);
                sum = sum + col*(1.0f - sum.w);

                // tracedDist += abs(s1);
            }
            else {
                //Beer–Lambert law
                //col = make_float4(color);
                // col = col*exp(-trp*abs(s1) / 100);
                // tracedDist = 0;
            }
        }

        s1 = s2;
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
        // if (sum.w >= 1.0) i = maxDist + 1;
        // else
        s2 = compute_SDF(pos, cell, cell.type);// sdfPrimBack(pos, make_float3(thePrd.maxDist)); //interpolateSDF(time, pos, texSDF, texSDF_F);
    }
    thePrd.result = sum;

    return tracedDist;
}
__device__ float  VolumeIntegrationInternalCell(float startDist, float maxDist, int pN, int fN)

{
    float Ka = 0.5;
    float4 col = make_float4(0, 0, 0, 1);// translucent_grays(0.5, 0.1, 0);

    float tstep = 0.01;

    float tracedDist = startDist;
    cellPrimDesc cellSec = thePrd.cellPrimitives[fN];
    cellPrimDesc cell = thePrd.cellPrimitives[pN];
    float dist = cell.intersectionDist;
    //---------------
    float3 pos = theRay.origin + theRay.direction*startDist;
    float3 step = theRay.direction*tstep;

    float4 sum = thePrd.result;// make_float4(thePrd.radiance.x, thePrd.radiance.y, thePrd.radiance.z, 0.1); //TODO: get background color here
                               //break if there is 1 opacity
    if (sum.w >= 1.0) return;

    float s1 = compute_SDF(pos, cell, cell.type); //sdfPrimBack(pos, make_float3(1.1));
    float materialCol = compute_SDF(pos, cellSec, cellSec.type); //sdfPrimBack(pos, make_float3(1.1));

    if (abs(s1) > tstep)
        step = theRay.direction*abs(s1);
    else step = theRay.direction*tstep;

    pos += step;
    float s2 = compute_SDF(pos, cell, cell.type); //sdfPrimBack(pos, make_float3(1.1));

                                                  //creates a holes of the molecule form
                                                  //max = abs(distF - dist);

    float3 color = Ka *  make_float3(cell.color.x, cell.color.y, cell.color.z);// ambient_light_color;

    float i = 0;
    float trp = cell.color.w;

    //start volume integration
    while (i < maxDist) //s2 < 0.01)
    {
        if (s2 < 0.0) {
            if (abs(s1) > tstep) //sum transparency
            {
                //save current position
                tracedDist = startDist + i;
                //col *= Ka;
                //col.w = 0.5; //s1 is very small
                // float3 color = Ka *  make_float3(0, 1, 0);// ambient_light_color;

                //initial blend
                if (materialCol <= 0.0) //secondary material
                {
                    col = cellSec.color*Ka;
                    trp = cellSec.color.w*Ka;
                }
                else
                    col = cell.color;

                //Beer–Lambert law
                float F = exp(-trp*abs(s1) * 200);
                col = col*(1.0 - F);
                sum = sum + col*(1.0f - sum.w);

                // tracedDist += abs(s1);
            }
            else {
                //Beer–Lambert law
                //col = make_float4(color);
                // col = col*exp(-trp*abs(s1) / 100);
                // tracedDist = 0;
            }
        }

        s1 = s2;
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

        // if (sum.w >= 1.0) i = maxDist + 1;
        // else
        {
            s2 = compute_SDF(pos, cell, cell.type);// sdfPrimBack(pos, make_float3(thePrd.maxDist)); //interpolateSDF(time, pos, texSDF, texSDF_F);
            materialCol = compute_SDF(pos, cellSec, cellSec.type); //sdfPrimBack(pos, make_float3(1.1));
        }
    }
    thePrd.result = sum;

    return tracedDist;
}

/*
__device ComputeCellsRelation(int i, int j)
{
    float relation_val = prd[i].dmax - prd[j].dmin;
int caseN = 0;
if (relation_val > 2 * eps) {
    //positive and big
    //inclusion
    caseN = 1;
}
else {
    if (relation_val < -2 * eps) {
        //negative and big
        //non adjucency
        caseN = 2;
    }
    //else case = 3; //adjucency =dist=eps;
}
}*/

/*
int i = pN + 1;
while (i < N)
{
cellPrimDesc curCell = thePrd.cellPrimitives[i];// .intersectionDist;

// if(prevCell.type==curCell.type)
// {
if (abs(prevCell.intersectionDist - curCell.intersectionDist) > 0.005)
{
//if (curCell.type > 0)
VolumeRaycast(i);
}
i++;
prevCell = curCell;
}
*/
__device__ void VolumeRayCasting2(float curDist, int pN, int pF, int NCell)
{
    float eps = 0.001;
    int i, j;
    i = pN;
    j = pF;

    if (pF >= NCell) //all cells are computed
    {
        if (pN < NCell) {
            float dmin1 = thePrd.cellPrimitives[pN].intersectionDist;
            float dmax1 = thePrd.cellPrimitives[pN].intersectionDist
                + thePrd.cellPrimitives[pN].maxDist;

            float curDist2 =
                VolumeIntegration(curDist, dmax1, pN);
        }

        return;
    }
    const cellPrimDesc prevCell = thePrd.cellPrimitives[pN];
    const cellPrimDesc curCell = thePrd.cellPrimitives[pF];

    //compute boundary values
    float dmax1 = thePrd.cellPrimitives[pN].intersectionDist + thePrd.cellPrimitives[pN].maxDist;
    float dmax2 = thePrd.cellPrimitives[pF].intersectionDist + thePrd.cellPrimitives[pF].maxDist;
    float dmin1 = thePrd.cellPrimitives[pN].intersectionDist;
    float dmin2 = thePrd.cellPrimitives[pF].intersectionDist;

    float curDist2 = curDist;
    if (curDist2 < dmin1) curDist2 = dmin1;
    /*----
    compute Cells relation
    ------*/

    float relation_val = dmax1 - dmin1;
    int caseN = 0;
    if (relation_val > eps) {
        //positive and big
        //inclusion
        caseN = 1;
    }
    else caseN = 2;

    /*----
    raycast cells for current ray-casting case
    ------*/

    switch (caseN) {
    case 1: {
        //Volume integration dist up to cell j boundary
        //curDist is set internally to prd[j].dmin
        //VolumeRaycast(pN);
        curDist2 = VolumeIntegration(curDist, dmin2, i);

        //curDist2 = VolumeIntegration(dmin1, dmax1, pN);

    //Volume integration within internal cell j with check of concave case
    //check concave case if volume ray-casting reaches prd[j].dmax according to current SDF function value
        //curDist2 = VolumeIntegrationInternalCell(dmin2, dmax2, pN, pF);
        //curDist2 = VolumeIntegration(dmin2, dmax2, pN);

        VolumeRaycast(i, j);
        curDist2 = dmax2;
        // curDist2 = VolumeIntegrationInternalCell(dmin2, dmax2, pN, pF);

    //increase j to check next internal cell

        j++;
        //check overlapping cells

        int ll = j - 1;
        int stop = 0;
        while (stop == 0) {
            if (j < NCell) {
                if (thePrd.cellPrimitives[ll].type == thePrd.cellPrimitives[j].type)
                {
                    if (abs(thePrd.cellPrimitives[ll].intersectionDist - thePrd.cellPrimitives[j].intersectionDist) < 0.01)
                    {
                        j++;
                    }
                    else stop = 1;
                }
                else stop = 1;
            }
            else stop = 1; //break;
        }

        break;
    }

    case 2:
    {
        //volume integration of cell i
        //curDist2 = VolumeIntegration(dmin1, dmax1, i);
        VolumeIntegration(curDist, dmax1, i);

        curDist2 = dmax1;
        //increase i and j to move to next neibouring cells
        i = j;
        //check overlapping cells

        int ll = j - 1;
        int stop = 0;
        while (stop == 0) {
            if (i < NCell) {
                if (thePrd.cellPrimitives[ll].type == thePrd.cellPrimitives[i].type)
                {
                    if (abs(thePrd.cellPrimitives[ll].intersectionDist - thePrd.cellPrimitives[i].intersectionDist) < 0.01)
                    {
                        i++;
                    }
                    else stop = 1;
                }
                else stop = 1;
            }
            else stop = 1; //break;
        }

        j = i + 1;
    }
    }

    VolumeRayCasting2(curDist2, i, j, NCell);
}

__device__ void render_Surface21_new()
{
    //-----------------
    //TODO: depth sort
    //---------------
    int N = thePrd.cur_prim - 1;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N - i - 1; j++)
        {
            const cellPrimDesc tmp = thePrd.cellPrimitives[i];
            // const float2 tmp = prd.particles[i];
            if (tmp.intersectionDist < thePrd.cellPrimitives[j].intersectionDist) {
                thePrd.cellPrimitives[i] = thePrd.cellPrimitives[j];
                thePrd.cellPrimitives[j] = tmp;
            }
        }

    int j = 0;
    int pN = -1;

    while (j < N) {
        const cellPrimDesc cell = thePrd.cellPrimitives[j];
        if (cell.type == 0) //main
        {
            pN = j;
            j = N;
        }
        j++;
    }
    /* if (pN < 0)
    //     return;//to catch error

     int pF = pN + 1;
     cellPrimDesc prevCell = thePrd.cellPrimitives[pN];
     float dist = prevCell.intersectionDist + prevCell.maxDist;
     //call main prog
     */
    if (N > 0) {
        float dist = thePrd.cellPrimitives[0].intersectionDist;
        VolumeRayCasting2(dist, 0, 1, N);
    }
}

__device__ void render_Surface21()
{
    //-----------------
    //TODO: depth sort
    //---------------
    int N = thePrd.cur_prim;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N - i - 1; j++)
        {
            const cellPrimDesc tmp = thePrd.cellPrimitives[i];
            // const float2 tmp = prd.particles[i];
            if (tmp.intersectionDist < thePrd.cellPrimitives[j].intersectionDist) {
                thePrd.cellPrimitives[i] = thePrd.cellPrimitives[j];
                thePrd.cellPrimitives[j] = tmp;
            }
        }

    int j = 0;
    int pN = -1;

    while (j < N) {
        const cellPrimDesc cell = thePrd.cellPrimitives[j];
        if (cell.type == 0) //main
        {
            pN = j;
            j = N;
        }
        j++;
    }
    if (pN < 0)
        return;//to catch error
    /*int finalN = thePrd.cur_prim - 2; //-1 doesn't cause an error
    if (finalN > 0)*/
    //float prevDist = (thePrd.cellPrimitives[pN]).intersectionDist;

   /* cellPrimDesc prevCell = thePrd.cellPrimitives[pN];
    int i = pN - 1;
    while (i > 0)
    {
        cellPrimDesc curCell = thePrd.cellPrimitives[i];// .intersectionDist;

       // if(prevCell.type==curCell.type)
       // {
        if (abs(prevCell.intersectionDist - curCell.intersectionDist) > 0.01)
        {
            VolumeRaycast(i);
        }
        i--;
        prevCell = curCell;
    }
   */

    int pF = pN + 1;
    cellPrimDesc prevCell = thePrd.cellPrimitives[pN];
    float dist = prevCell.intersectionDist + prevCell.maxDist;
    //call main prog

    VolumeRayCasting2(dist, 0, 1, N);
}

//int cellsType= computeCellRelation(pN, pN + 1);
/*
cellPrimDesc prevCell = thePrd.cellPrimitives[pN];

int i = pN + 1;
while (i < N)
{
    cellPrimDesc curCell = thePrd.cellPrimitives[i];// .intersectionDist;

                                                    // if(prevCell.type==curCell.type)
                                                    // {
    if (abs(prevCell.intersectionDist - curCell.intersectionDist) > 0.005)
    {
        //if (curCell.type > 0)
        VolumeRaycast(i);
    }
    i++;
    prevCell = curCell;
}
*/

RT_PROGRAM void miss_environment_constant()
{
    if (thePrd.renderType > 0) {
        render_Surface21_new();
        // printf("crossed prim %d ", thePrd.cur_prim);
    }
    if (thePrd.renderType == 0)
        thePrd.radiance += sysBackground; // Constant white emission.
}

RT_PROGRAM void auditory_miss_environment_constant()
{
    thePrd.result = make_float4(1.0f);
}