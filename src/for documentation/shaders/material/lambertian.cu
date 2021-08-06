// ======================================================================== //
// Copyright 2018 Ingo Wald                                                 //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //
#include <optix.h>
#include <optixu/optixu_math_namespace.h>


#include "material.h"
#include "../renderer/per_ray_data.h"
#include "sampling.h"

/*! the implicit state's ray we will intersect against */
rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );
rtDeclareVariable(float, theIntersectionDistance, rtIntersectionDistance, );
/*! the per ray data we operate on */
rtDeclareVariable(PerRayData, prd, rtPayload, );
rtDeclareVariable(rtObject, sysTopObject, , );


/*! the attributes we use to communicate between intersection programs and hit program */
rtDeclareVariable(optix::float3, varNormal,    attribute NORMAL, );
rtDeclareVariable(optix::float3, varHit, attribute hit_point, );


/*! and finally - that particular material's parameters */
rtDeclareVariable(float3, albedo, , );


// Helper functions for sampling a cosine weighted hemisphere distrobution as needed for the Lambert shading model.

RT_FUNCTION void alignVector(float3 const& axis, float3& w)
{
    // Align w with axis.
    const float s = copysign(1.0f, axis.z);
    w.z *= s;
    const float3 h = make_float3(axis.x, axis.y, axis.z + s);
    const float  k = optix::dot(w, h) / (1.0f + fabsf(axis.z));
    w = k * h - w;
}

RT_FUNCTION void unitSquareToCosineHemisphere(const float2 sample, float3 const& axis, float3& w, float& pdf)
{
    // Choose a point on the local hemisphere coordinates about +z.
    const float theta = 2.0f * M_PIf * sample.x;
    const float r = sqrtf(sample.y);
    w.x = r * cosf(theta);
    w.y = r * sinf(theta);
    w.z = 1.0f - w.x * w.x - w.y * w.y;
    w.z = (0.0f < w.z) ? sqrtf(w.z) : 0.0f;

    pdf = w.z * M_1_PIf;

    // Align with axis.
    alignVector(axis, w);
}




/*! the actual scatter function - in Pete's reference code, that's a
  virtual function, but since we have a different function per program
  we do not need this here */
inline __device__ bool scatter(const optix::Ray &ray_in,
                               optix::float3 &attenuation,
                               optix::Ray &scattered)
{
  optix::float3 normal    = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varNormal));
  

 // optix::float3 jit = optix::make_float3(rng(prd.seed), rng(prd.seed), rng(prd.seed));// / prd.screen) * 2.0f - 1.0f;
 // jit = (jit / 10)*2-1.0;
  optix::float3 hit = ray_in.origin+ theIntersectionDistance*ray_in.direction;
  //+optix::make_float3(rng(seed))  - fixes the problem

  // Lambert sampling: Cosine weighted hemisphere sampling above the shading normal.
  // This calculates the ray.direction for the next path segment in wi and its probability density function value in pdf.
  unitSquareToCosineHemisphere(rng2(prd.seed), normal, prd.wi, prd.pdf);


  optix::float3 target = hit + normal + prd.wi*prd.pdf;
  //if (theIntersectionDistance != 0) printf("intersection %f,", theIntersectionDistance);
  scattered    = optix::Ray(hit,
                            (target- hit),
                            /*type*/0,
                            /*tmin*/1e-3f,
                            /*tmax*/RT_DEFAULT_MAX);
  attenuation  = albedo;
  return true;
}

RT_PROGRAM void closesthit_sdf()
{
  optix::Ray scattered;
  float3     attenuation=make_float3(0);
  if (prd.depth < 100 && scatter(theRay,attenuation,scattered)) {
    PerRayData rec;
    rec.depth = prd.depth+1;
    rec.seed = prd.seed;
    rtTrace(sysTopObject,scattered,rec);
    prd.radiance = attenuation * rec.radiance;
  } else {
    prd.radiance = make_float3(0);
  }
}
