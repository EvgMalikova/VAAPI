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

#include "DRand48.h"
#include "../renderer/random_number_generators.h"

inline __device__ optix::float3 random_in_unit_disk(DRand48 &local_rand_state) {
  optix::float3 p;
  do {
    p = 2.0f*optix::make_float3(local_rand_state(), local_rand_state(), 0) - optix::make_float3(1, 1, 0);
  } while (optix::dot(p, p) >= 1.0f);
  return p;
}


//#define RANDVEC3F optix::make_float3(rnd(),rnd(),rnd())

inline __device__ optix::float3 random_in_unit_sphere(unsigned int &seed) {
  optix::float3 p;
  //do {
  p = 2*optix::make_float3(rng(seed), rng(seed), rng(seed))-optix::make_float3(1);
 // } while (optix::dot(p,p) >= 1.0f);
  return p;
}

