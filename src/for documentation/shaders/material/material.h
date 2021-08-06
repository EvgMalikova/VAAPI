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

#pragma once

//#include "prd.h"
#include "DRand48.h"


__device__ float schlick(float cosine, float ref_idx) {
  float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
  r0 = r0 * r0;
  return r0 + (1.0f - r0)*powf((1.0f - cosine), 5.0f);
}

__device__ bool refract(const optix::float3& v, const optix::float3& n, float ni_over_nt, optix::float3& refracted) {
  optix::float3 uv = optix::normalize(v);
  float dt = optix::dot(uv, n);
  float discriminant = 1.0f - ni_over_nt * ni_over_nt*(1 - dt * dt);
  if (discriminant > 0) {
    refracted = ni_over_nt * (uv - n * dt) - n * sqrtf(discriminant);
    return true;
  }
  else
    return false;
}

inline __device__ optix::float3 reflect(const optix::float3 &v, const optix::float3 &n)
{
  return v - 2.0f*optix::dot(v, n)*n;
}

