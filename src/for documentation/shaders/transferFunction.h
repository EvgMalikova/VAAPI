
#pragma once

#include <optixu/optixu_math_namespace.h>
using namespace optix;
inline __device__ float4 lerp4f(float4 a, float4 b, float c)
{
    return a * (1.f - c) + b * c;
}

inline __device__ float lerp1f(float a, float b, float c)
{
    return a * (1.f - c) + b * c;
}

inline __device__
float3 pal(float t, float3 a, float3 b, float3 c, float3 d)
{
	float3 x = 6.28318f*(c*t + d);
	x.x = cosf(x.x);
	x.y = cosf(x.y);
	x.z = cosf(x.z);

	return a + b*x;
}


inline __device__
float4 translucent_grays(float drbf,float t, int type) {

	//return vec4(t, t, t, t*0.05);
	float3 x1 = make_float3(0.5f, 0.5f, 0.5f);
	float3 x2 = make_float3(0.5, 0.5, 0.5);
	float3 x3 = make_float3(2.0, 1.0, 0.0);
	float3 x4 = make_float3(0.5, 0.20, 0.25);
	switch (type) {
	case 1:
		x3 = make_float3(2.0, 1.0, 0.0);
		x4 = make_float3(0.5, 0.20, 0.25);
		break;
	case 2:
		x3 = make_float3(2.0, 1.0, 1.0);
		x4 = make_float3(0.0, 0.25, 0.25);
		break;
	case 3:
		x3 = make_float3(1.0, 0.7, 0.4);
		x4 = make_float3(0.0, 0.15, 0.20);
		//1.0, 0.7, 0.4), vec3(0.0, 0.15, 0.20
		break;
	case 4:
		x3 = make_float3(1.0, 1.0, 0.5);
		x4 = make_float3(0.8, 0.90, 0.30);
		//1.0, 1.0, 0.5), vec3(0.8, 0.90, 0.30)
		break;
	case 5:
		x3 = make_float3(1.0, 1.0, 1.5);
		x4 = make_float3(0.3, 0.20, 0.20);
		//1.0,1.0,1.0),vec3(0.3,0.20,0.20)
		break;
	}
	//if (p.y>(4.0 / 7.0)) col = pal(p.x, vec3(0.5, 0.5, 0.5), vec3(0.5, 0.5, 0.5), vec3(1.0, 0.7, 0.4), vec3(0.0, 0.15, 0.20));

	float3 col = pal(t+0.5*drbf, x1,  x2, x3, x4);
	float4 color = make_float4(col.x, col.y, col.z, drbf);//t*1.8);

	return color;
}

inline __device__
float4 translucent_grays2(float t, float t2) {

	//return vec4(t, t, t, t*0.05);
	float valx = t2;
	float trp = t;
	float3 x1= make_float3(0.5f, 0.5f, 0.5f);
		float3 x2 = make_float3(1.0f, 1.0f, 1.0f);
		float3 x3 = make_float3(0.0f, 0.33f, 0.67f);
		if (t2 >= 1.0)
		{

			x3 = make_float3(0.0, 0.10, 0.20);
			valx = abs(t2) - 1.0;
			if(t2<0)
			trp = -trp/2.0;

		}
		float3 col = pal(valx, x1, x1, x2, x3);
		float4 color = make_float4(col.x, col.y, col.z, trp);//t*1.8);
		
	return color;
}

inline __device__ float4 tf(float v, float t, const int tf_type)
{
  float4 color;

  if (tf_type == 1)
  {
    if (v < .5f)
      color = lerp4f( make_float4(1,1,0,0), make_float4(1,1,1,0.5f), v * 2.f);
    else
      color = lerp4f( make_float4(1,1,1,0.5f), make_float4(1,0,0,1), v * 2.f - 1.f);
  }
  else if (tf_type == 2)
  {
    if (v < .5f)
      color = lerp4f( make_float4(0,0,1,0), make_float4(1,1,1,0.5f), v * 2.f);
    else
      color = lerp4f( make_float4(1,1,1,0.5f), make_float4(1,0,0,1), v * 2.f - 1.f);
  }
  else if (tf_type == 3)
  {
    if (v < .33f)
      color = lerp4f( make_float4(1.f,0.f,1.f,0.f), make_float4(0.f,0.f,1.f,0.33f), (v-0.f) * 3.f);
    else if (v < .66f)
      color = lerp4f( make_float4(0.f,0.f,1.f,.33f), make_float4(0.f,1.f,1.f,0.66f), (v-0.33f) * 3.f);
    else
      color = lerp4f( make_float4(0.f,1.f,0.f,0.5f), make_float4(1.f,1.f,1.f,1.f), (v-0.66f) * 3.f);
  }
  else
  {
    color = lerp4f( make_float4(0,0,1,0), make_float4(1,0,0,1), v);
  }

  //redshift
#if 1
  if (t > 0.f)
  {
    const float alpha = color.w;
    const float r = 1.f - expf(-t);
    if (r < .5f)
      color = lerp4f( make_float4(1,1,1,0), color, r * 2.f);
    else
      color = lerp4f( color, make_float4(1,0,0,1), r * 2.f - 1.f);
    color.w = alpha;
  }
#endif

  return color;

}
