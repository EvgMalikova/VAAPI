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
#include <optix_math.h>
#include <optixu/optixu_matrix_namespace.h>
#include "../inc/data_structures/vertex_attributes.h"

rtBuffer<VertexAttributes> attributesBuffer;
rtBuffer<uint3>            indicesBuffer;

// Attributes.
rtDeclareVariable(optix::float3, varGeoNormal, attribute GEO_NORMAL, );
rtDeclareVariable(optix::float3, varTangent, attribute TANGENT, );
rtDeclareVariable(optix::float3, varNormal, attribute NORMAL, );
rtDeclareVariable(optix::float3, varTexCoord, attribute TEXCOORD, );

rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );

//for SDF

typedef rtCallableProgramId<float(float3, float)> callT;
rtDeclareVariable(callT, hit_hook, , );

// Intersection routine for indexed interleaved triangle data.
RT_PROGRAM void intersection_triangle_indexed(int primitiveIndex)
{
    const uint3 indices = indicesBuffer[primitiveIndex];

    VertexAttributes const& a0 = attributesBuffer[indices.x];
    VertexAttributes const& a1 = attributesBuffer[indices.y];
    VertexAttributes const& a2 = attributesBuffer[indices.z];

    const float3 v0 = a0.vertex;
    const float3 v1 = a1.vertex;
    const float3 v2 = a2.vertex;

    float3 n;
    float  t;
    float  beta;
    float  gamma;

    if (intersect_triangle(theRay, v0, v1, v2, n, t, beta, gamma))
    {
        if (rtPotentialIntersection(t))
        {
            // Barycentric interpolation:
            const float alpha = 1.0f - beta - gamma;

            // Note: No normalization on the TBN attributes here for performance reasons.
            //       It's done after the transformation into world space anyway.
            varGeoNormal = n;
            varTangent = a0.tangent  * alpha + a1.tangent  * beta + a2.tangent  * gamma;
            varNormal = a0.normal   * alpha + a1.normal   * beta + a2.normal   * gamma;
            varTexCoord = a0.texcoord * alpha + a1.texcoord * beta + a2.texcoord * gamma;

            rtReportIntersection(0);
        }
    }
}

__device__
inline float sdSphere(float3 p, float s)
{
    return length(p) - s;
}

RT_PROGRAM void intersection_sdf_sphere(int primIdx)
{
    bool shouldSphereTrace = false;
    float tmin, tmax;
    tmin = 0;
    tmax = RT_DEFAULT_MAX;

    const float sqRadius = 100;

    float distance;

    //if (shouldSphereTrace)
    {
        //      Mandelbulb sdf(max_iterations);
        //      MengerSponge sdf(max_iterations);
        //      IFSTest sdf(max_iterations);
        //      sdf.setTime(global_t);
        //      sdf.evalParameters();

        // === Raymarching (Sphere Tracing) Procedure ===
        optix::float3 ray_direction = theRay.direction;
        optix::float3 eye = theRay.origin;
        //    eye.x -= global_t * 1.2f;
        optix::float3 x = eye + tmin * ray_direction;

        float dist_from_origin = tmin;

        const float epsilon = 0.000001;//delta;
        const float eps = 0.0001;
        float dist = 0;

        const float NonLinearPerspective = 1.1;

        const float Jitter = 0.05f;
        float totalDistance = 0.0;//Jitter * tea<4>(current_prd.seed, frame_number);
        int i = 0;
        bool stop = false;
        while (!stop)
        {
            //        if(current_prd.depth == 0)
            //        {
            //            float delta = sin( relative_t * 0.1f ) * 10 + tan(relative_t  * 0.001f) * 4;
            //            float2 rot = rotate( make_float2(originalDir.z, originalDir.y),
            //                                          radians(dist_from_origin * delta) ) * NonLinearPerspective;
            //            ray_direction.z = -rot.x;
            //            ray_direction.y = rot.y;
            //        }

            //      sdf.setTranslateHook(0, make_float3( -global_t * 1.0f, 0.0f, 0.0f ) );
            //      sdf.setRotateHook( 0, make_float3( radians(-global_t / 18.0f), 0.0f, 0.0f) );

            //      float scale = 1.0f;
            //      float3 offset = make_float3(0.92858,0.92858,0.32858);
            //      sdf.setScaleHook( 0, x * scale - offset * (scale - 1.0f));

            //float4 result = hit_hook(x, max_iterations, global_t);
            dist = sdSphere(x, 0.6);
            //float3 trapValue = make_float3(result.y, result.z, result.w);

            // Step along the ray and accumulate the distance from the origin.
            x += dist * ray_direction;
            //dist_from_origin += dist * fudgeFactor;
            totalDistance += dist;

            // Check if we're close enough or too far.
            if (dist < epsilon || totalDistance > tmax)
            {
                stop = true;
            }
            else i++;
        }

        // Found intersection?
        if (dist < epsilon)
        {
            if (rtPotentialIntersection(totalDistance))
            {
                //        sdf.setMaxIterations(14); // more iterations for normal estimate, to fake some more detail
                // varNormal        = calculateNormal(sdf, x, DEL);

                float dx = sdSphere(x + make_float3(eps, 0, 0), 0.6) - sdSphere(x - make_float3(eps, 0, 0), 0.6);
                float dy = sdSphere(x + make_float3(0, eps, 0), 0.6) - sdSphere(x - make_float3(0, eps, 0), 0.6);
                float dz = sdSphere(x + make_float3(0, 0, eps), 0.6) - sdSphere(x - make_float3(0, 0, eps), 0.6);

                varNormal = normalize(make_float3(dx, dy, dz));

                rtReportIntersection(0);
            }
        }
    }
}