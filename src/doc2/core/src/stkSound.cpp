
#include "SineWave.h"
#include "ADSR.h"
#include "Plucked.h"
#include "FileWvOut.h"
#include "Noise.h"
#include "Modulate.h"
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"
#include <windows.h>
#include <mmsystem.h>
#include <thread>
#include <chrono>
#include <stdlib.h>

#include "AL/alut.h"

#include "stkSound.h"
//#include "common/alhelpers.h"

using namespace stk;
namespace stkSound {

	//double srate2 = 48000;

	int InitAL(char ***argv, int *argc)
	{
		const ALCchar *name;
		ALCdevice *device;
		ALCcontext *ctx;

		/* Open and initialize a device */
		device = NULL;
		if (argc && argv && *argc > 1 && strcmp((*argv)[0], "-device") == 0)
		{
			device = alcOpenDevice((*argv)[1]);
			if (!device)
				fprintf(stderr, "Failed to open \"%s\", trying default\n", (*argv)[1]);
			(*argv) += 2;
			(*argc) -= 2;
		}
		if (!device)
			device = alcOpenDevice(NULL);
		if (!device)
		{
			fprintf(stderr, "Could not open a device!\n");
			return 1;
		}

		ctx = alcCreateContext(device, NULL);
		if (ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
		{
			if (ctx != NULL)
				alcDestroyContext(ctx);
			alcCloseDevice(device);
			fprintf(stderr, "Could not set a context!\n");
			return 1;
		}

		name = NULL;
		if (alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
			name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
		if (!name || alcGetError(device) != AL_NO_ERROR)
			name = alcGetString(device, ALC_DEVICE_SPECIFIER);
		printf("Opened \"%s\"\n", name);

		return 0;
	}

	/* CloseAL closes the device belonging to the current context, and destroys the
	* context. */
	void CloseAL(void)
	{
		ALCdevice *device;
		ALCcontext *ctx;

		ctx = alcGetCurrentContext();
		if (ctx == NULL)
			return;

		device = alcGetContextsDevice(ctx);

		alcMakeContextCurrent(NULL);
		alcDestroyContext(ctx);
		alcCloseDevice(device);
	}


	void ApplySin(ALfloat *data, ALdouble g, ALuint srate, ALuint freq)
	{
		ALdouble smps_per_cycle = (ALdouble)srate / freq;
		ALuint i;
		for (i = 0; i < srate; i++)
			data[i] += (ALfloat)(sin(i / smps_per_cycle * 2.0*3.14) * g);
	}
	
	void CreateWave(StkFloat* ptr, int srate2, int duration, ALuint& buffer)
	{
		ALuint seed = 22222;
		ALfloat data_size;

		
		ALenum err;
		ALuint i;
		ALuint srate = ALuint(srate2);
		data_size = srate * sizeof(ALfloat);
		ALfloat *data = (ALfloat *)calloc(1, data_size);
		//std::cout << "total frames2 " << int(srate) << std::endl;

		for (int i = 0; i < srate; i++) {
			data[i] = (ALfloat)ptr[i];
			//std::cout << data[i] << std::endl;
		}

		/* Buffer the audio data into a new buffer object. */
		//buffer = 0;
		alGenBuffers(1, &buffer);
		alBufferData(buffer, AL_FORMAT_MONO_FLOAT32, data, data_size, ALfloat(srate/ duration));
		// for (int i = 0; i < srate; i++) {

		//   std::cout << data[i] << std::endl;
		// }
		free(data);

		/* Check if an error occured, and clean up if so. */
		err = alGetError();
		if (err != AL_NO_ERROR)
		{
			std::cout << "error occured" << std::endl;
			fprintf(stderr, "OpenAL Error: %s\n", alGetString(err));
			if (alIsBuffer(buffer))
				alDeleteBuffers(1, &buffer);
			buffer=0;
		}

	//	return buffer;
	}

	void InitEnv()
	{
		// Initialize the environment
		alutInit(0, NULL);
		std::cout << "start 1" << std::endl;
		// Capture errors
		alGetError();
		std::cout << "start 2" << std::endl;
	}
	void ExitEnv()
	{
		alutExit();
	}

	//generate noise for defined time with defined seed
	StkFrames  GenerateNoise(double time, double srate2, float seed)
	{

		Stk::setSampleRate(srate2);

		Noise noise;
		SineWave input;
		Modulate mod;
		mod.setVibratoRate(40);
		mod.setRandomGain(120);
	//	noise.setSeed(seed);
		input.setFrequency(5);
		long nFrames = (long)(int(5+time) * Stk::sampleRate());
		StkFrames frames(nFrames, 1);
		StkFrames frames2(nFrames, 1);
		//StkFrames frames3(nFrames, 1);
		noise.tick(frames);
		input.tick(frames2);

	//	mod.tick(frames3);
	//	frames2.Modulate(frames3);
		
		
		//frames.Modulate(frames2);
		//frames.ModulateSin(10, 60, 44000);
		return frames;
	}

	
	void WaitforSound(float scale)
	{

		alutSleep(scale);
	}
	void PlaySelSound(ALuint source)
	{
		alSourcePlay(source);
		
	}

	void MoveSounds(std::vector<stkSound::soundDesc>& descSs, std::vector<float>& xpos, int scale)
	{
		
		std::this_thread::sleep_for(std::chrono::microseconds(scale * 1));
		for (int i = 1; i < 80; i++)
		{
			for (int j = 0; j < descSs.size(); j++)
			{
				alSource3f(descSs[j].source, AL_POSITION, xpos[j]+i/100, 0.0f, -1 + abs(xpos[j]) / i);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(scale * 1));
		}
	}
	//test function with sleep - screen update doesnt' work
	void RotateTest(int scale)
	{

		//rotate and others
		//Play the sound until it finishes. 
		float angle = 0.0;

		//alutSleep(time);
		float step = scale / 10;
		float curTime = 0;
		//alutSleep(step);
		//Sleep(step * 1000);
		std::this_thread::sleep_for(std::chrono::microseconds(scale * 100));
		for (int i = 0; i < 20; i++)
		{
			angle += 0.3 * M_PI * 0.5;
			if (angle > M_PI)
				angle -= M_PI*2.0;
			//stkSound::RotateAngle(angle, sDesc.source);
			//context["TimeSound"]->setFloat(float(scale/10));

			//Render::camera_slow_rotate = !Render::camera_slow_rotate;

			//glfwSwapBuffers(window);
			//ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

			std::this_thread::sleep_for(std::chrono::milliseconds(scale * 100));
		}
	}
	soundDesc PlayBuffer(StkFloat* ptr, int time, int size, int j, optix::float2 xpos) {


		ALuint buffer = ALuint(j);
		ALuint source=ALuint(j);
		ALuint state;
		//std::cout << "total frames " << int(frames.size()) << std::endl;

		// Load pcm data into buffer
		CreateWave(ptr, size, time,buffer); //alutCreateBufferFromFile("banjo.wav");

		//set listener position
		alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
		alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		ALfloat flValues[6];
		//https://github.com/blackberry/OpenAL/blob/master/OpenAL32/alListener.c
		//https://stackoverflow.com/questions/7861306/clarification-on-openal-listener-orientation
		flValues[0] = 0;
		flValues[1] = 0;
		flValues[2] = -1;
		flValues[3] = 0;
		flValues[4] = 1;
		flValues[5] = 0;
		//alListenerfv(AL_ORIENTATION, flValues);


		// Create sound source (use buffer to fill source)
		alGenSources(1, &source);
		//alSourcei(source, AL_BUFFER, buffer);

		//sf::Listener::SetPosition(ListenerLocation.x, -3.0f, ListenerLocation.y);
		//sf::Listener::SetDirection(cos(Angle)*100.0f, 100.0f, sin(Angle)*100.0f);

		alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
		alSource3f(source, AL_POSITION, xpos.x, xpos.y, -1+abs(xpos.x));
		alSourcei(source, AL_BUFFER, buffer);
		assert(alGetError() == AL_NO_ERROR && "Failed to setup sound source");

		
		return soundDesc(source,buffer);
	
		
	}

	void RotateAngle(double angle, ALuint& source)
	{
		alSource3f(source, AL_POSITION, (ALfloat)sin(angle), 0.0f, -(ALfloat)cos(angle));

		//if (has_angle_ext) 
		{

			ALfloat angles[2] = { (ALfloat)(M_PI / 6.0 - angle), (ALfloat)(-M_PI / 6.0 - angle) };
			alSourcefv(source, AL_STEREO_ANGLES, angles);
		}
		ALenum state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		//std::cout << angle << std::endl;

		
	}
	void ApplyADSR(stk::StkFrames& frames, int releaseCount)
	{
		ADSR adsr;
		Stk::setSampleRate(48000);
		//long releaseCount = (long)(Stk::sampleRate() *frames.size()* 0.5);
		adsr.setAttackTime(1.0);
		//adsr.setReleaseRate(0.2);
		//adsr.setSustainLevel(0.5);
		adsr.setAllTimes(0.1, 0.1, 0.4, 0.1);
		adsr.keyOn();
		bool once = false;
		//std::cout << adsr.lastOut() << std::endl;
		for (int i = 0; i < frames.size(); i++)
		{
			frames[i] *= adsr.lastOut()*0.2;
			//std::cout << adsr.lastOut() << std::endl;
			adsr.tick();
			if (i >= releaseCount) {
				if (!once)
				{
					adsr.keyOff();
					//std::cout << adsr.lastOut() << std::endl;
					once = true;
				}
			}
		}

		
		

	}
	//generate plucked string
	StkFrames GeneratePlucked(int time, double srate2, double freq, double amplitude)
	{


		//SineWave **oscs = (SineWave **)malloc(channels * sizeof(SineWave *));

		Stk::setSampleRate(srate2);
		Plucked pl;
		pl.noteOn(freq, amplitude);

		long nFrames = (long)(time * Stk::sampleRate());
		StkFrames frames(nFrames, 1);

		pl.tick(frames, 1);
		return frames;
	}
		
}