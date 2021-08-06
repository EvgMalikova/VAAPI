#pragma once
#include "Generator.h"
#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>

namespace stkSound {
	typedef struct soundDesc {
		ALuint source;
		ALuint buffer;
		soundDesc(ALuint s, ALuint b) {
			this->source=s;
			this->buffer=b;
		};
	};
	stk::StkFrames GeneratePlucked(int time, double srate2, double freq, double amplitude);
	stk::StkFrames GenerateNoise(double duration, double srate2, float seed);
	void ApplyADSR(stk::StkFrames& frames, int releaseCount);
	soundDesc PlayBuffer(stk::StkFloat* ptr, int time,int size, int j, optix::float2 xpos);
	void InitEnv();
	void ExitEnv();
	void RotateAngle(double angle, ALuint& source);
	void PlaySelSound(ALuint source);
	void MoveSounds(std::vector<stkSound::soundDesc>& descSs, std::vector<float>& xpos, int scale);
	void WaitforSound(float scale);
	//test not working
	void RotateTest(int scale);
	//void CreateWave(StkFloat* ptr, int srate2, int duration, ALuint& buffer);
}