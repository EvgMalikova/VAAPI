#include "Renderer.h"


#include "itkUtil.h"
#include "itkCastImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include <itkImageFileWriter.h>
#include <itkNrrdImageIO.h>
#include "vtkSoundRendering.h"

//--------------------
// stk
#include "stkSound.h"
#include "NvFlex.h"
#include <NvFlexExt.h>
#include <NvFlexDevice.h>

#include "../flex/core/types.h"
#include "../flex/core/maths.h"
typedef XVector4<float> Vec4;

void FlexProc(int n,float dt)
{
	NvFlexLibrary* library = NvFlexInit();

	// create new solver
	NvFlexSolverDesc solverDesc;
	NvFlexSetSolverDescDefaults(&solverDesc);
	solverDesc.maxParticles = n;
	solverDesc.maxDiffuseParticles = 0;

	NvFlexSolver* solver = NvFlexCreateSolver(library, &solverDesc);

	NvFlexBuffer* particleBuffer = NvFlexAllocBuffer(library, n, sizeof(float4), eNvFlexBufferHost);
	NvFlexBuffer* velocityBuffer = NvFlexAllocBuffer(library, n, sizeof(float4), eNvFlexBufferHost);
	NvFlexBuffer* phaseBuffer = NvFlexAllocBuffer(library, n, sizeof(int), eNvFlexBufferHost);

	int numParticles = 0;
	bool done = false;
	while (!done)
	{
		// map buffers for reading / writing
		float4* particles = (float4*)NvFlexMap(particleBuffer, eNvFlexMapWait);
		float3* velocities = (float3*)NvFlexMap(velocityBuffer, eNvFlexMapWait);
		int* phases = (int*)NvFlexMap(phaseBuffer, eNvFlexMapWait);

		// spawn (user method)
		//numParticles = SpawnParticles(particles, velocities, phases, numParticles);

		// render (user method)
		//RenderParticles(particles, velocities, phases, numParticles);

		// unmap buffers
		NvFlexUnmap(particleBuffer);
		NvFlexUnmap(velocityBuffer);
		NvFlexUnmap(phaseBuffer);

		// write to device (async)
		NvFlexSetParticles(solver, particleBuffer, NULL);
		NvFlexSetVelocities(solver, velocityBuffer, NULL);
		NvFlexSetPhases(solver, phaseBuffer, NULL);

		// set active count
		NvFlexSetActiveCount(solver, numParticles);

		// tick
		NvFlexUpdateSolver(solver, dt, 1, false);

		// read back (async)
		NvFlexGetParticles(solver, particleBuffer, NULL);
		NvFlexGetVelocities(solver, velocityBuffer, NULL);
		NvFlexGetPhases(solver, phaseBuffer, NULL);
	}

	NvFlexFreeBuffer(particleBuffer);
	NvFlexFreeBuffer(velocityBuffer);
	NvFlexFreeBuffer(phaseBuffer);

	NvFlexDestroySolver(solver);
	NvFlexShutdown(library);
}
////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
//const char *volumeFilename2 = "output2.raw";

extern "C" void initCuda(void *h_volume, void *sdf_volume, void *h_volume2, void *sdf_volume2, cudaExtent volumeSize);


Renderer ren;// = new Renderer();

std::string filename = "ed1.txt";
std::string filename2 = "p1.txt";

std::string filename21 = "ed2.txt";
std::string filename22 = "p2.txt";
//using namespace itkUtil;


int
main(int argc, char **argv)
{
	double sp[3] = { 0.05,0.05,0.05 };
	//GenerateWave(); - works
	itkUtil::FloatImageType::SizeType size;
	size[0] = 139;
	size[1] = 150;
	size[2] = 160;

	int reduce = 138;
   //Read first frame
	itkUtil::FloatImageType::Pointer im_sdf = itkUtil::FloatImageType::New();
	itkUtil::ReadImage(im_sdf,filename.c_str(),size, reduce,sp);
	
	float min= itkUtil::ComputeSDF(im_sdf, 0.05);

	//read second field
	itkUtil::FloatImageType::SizeType size2;
	size2[0] = 138;
	size2[1] = 150;
	size2[2] = 160;

	itkUtil::FloatImageType::Pointer im_sdf2 = itkUtil::FloatImageType::New();
	itkUtil::FloatImageType::Pointer im_internal2 = itkUtil::FloatImageType::New();

	itkUtil::ReadImage(im_sdf2, filename21.c_str(), size2, reduce, sp);
	itkUtil::ReadImage(im_internal2, filename22.c_str(), size2, reduce,sp);
	float min2 = itkUtil::ComputeSDF(im_sdf2, 0.05);

	//compute midi field
	itkUtil::FloatImageType::Pointer midi_image = itkUtil::FloatImageType::New();
	//itkUtil::ComputeMIDIfromSDF(midi_image, im_sdf, min);
	

	//ren.sRen.SetImageData(midi_image);//just to test should be other preprocessed data
	//ren.sRen.SetScale(0.0, double(-min));
	//ren.sRen.PlayPianoRangeSlice(1, 80, 6);

	itk::ImageFileWriter<itkUtil::FloatImageType>::Pointer wr
		= itk::ImageFileWriter<itkUtil::FloatImageType>::New();
	wr->SetFileName("output1.nrrd");
	wr->SetInput(im_internal);
	wr->Update();

	//compute centers
	itkUtil::FloatPoints points1;
	itkUtil::ComputeCenters(im_internal, 25, points1);

	itkUtil::FloatPoints points2;
	itkUtil::ComputeCenters(im_internal2, 35, points2);

	int sizet = points1.size();
	
	if (sizet > int(points2.size())) {
		std::cout << "Not equal size of points is detected " << std::endl;
		return 0;
	}
	float x, y, z;
	//load to GPU
	for (int i = 0; i < 3; i++) {
		x = points1[i][0]/( sp[0]*reduce);
		y = points1[i][1] / (sp[0] * reduce);
		z = points1[i][2] / (sp[0] * reduce);
		ren.m_centers1.push_back(make_float3(x, y, z));

		x = points2[i][0] / (sp[0] * reduce);
		y = points2[i][1] / (sp[0] * reduce);
		z = points2[i][2] / (sp[0] * reduce);
		ren.m_centers2.push_back(make_float3(x, y, z));
	}

	 


	
	//update all data
	im_sdf->UpdateOutputData();
	im_sdf->Update();

	im_sdf2->UpdateOutputData();
	im_sdf2->Update();
	//set params
	
	sdfParams param1;
	param1.lvShift = min;
	param1.texSize = reduce;

	sdfParams param2;
	param2.lvShift = min2;
	param2.texSize = reduce;




	size_t dim = size_t(param1.texSize);
	cudaExtent volumeSize = make_cudaExtent(dim, dim, dim);
	ren.setSDFParam(param1);
	ren.setSDFParam(param2);

	
        // First initialize OpenGL context, so we can properly set the GL for CUDA.
        // This is necessary in order to achieve optimal performance with OpenGL/CUDA interop.
        ren.initGL(&argc, argv);

        // use command-line specified CUDA device, otherwise use device with highest Gflops/s
        ren.chooseCudaDevice(argc, (const char **)argv, true);
  
	std::cout << "Start!\n" << std::endl;
	//std::cout << img2 << std::endl;
	void *h_volume = im_internal->GetBufferPointer(); //img2->GetBufferPointer();
	void *sdf_volume=im_sdf->GetBufferPointer();
	

	void *h_volume2 = im_internal2->GetBufferPointer(); //img2->GetBufferPointer();
	void *sdf_volume2 = im_sdf2->GetBufferPointer();

	
	initCuda(h_volume, sdf_volume, h_volume2, sdf_volume2, volumeSize);
	printf("test 1passed");
	ren.InCuda(volumeSize);

    free(h_volume);
	free(sdf_volume);
	free(h_volume2);
	free(sdf_volume2);
	
	//perform all OpenGL functions
	ren.Init();
}
