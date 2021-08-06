/***************************************************/
/************File with basic unilities************
/***************************************************/

#include "itkUtil.h"
/*
//#include "itkRawImageIO.h"
//#include "itkImageFileReader.h"
#include <itkImageFileWriter.h>
#include <itkNrrdImageIO.h>
#include <itkBinaryThresholdImageFilter.h>

#include <itkApproximateSignedDistanceMapImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkStatisticsImageFilter.h>
#include <itkBinaryImageToShapeLabelMapFilter.h>
#include <itkExtractImageFilter.h>
#include "itkDiscreteGaussianImageFilter.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"

#include <itkRegionOfInterestImageFilter.h>

namespace itkUtil {
	
	/*
	*Copies input image to another image, those smaller region is specified
	*TODO: make region size check
	*consider case of bigger region
	*regions and images should be already allocated in memorry
	*/
	template<typename TImage>
	void RegionCopy(typename TImage::Pointer input, typename TImage::Pointer output)
	{
		//output->SetRegions(output->GetLargestPossibleRegion());
		//output->Allocate();

		itk::ImageRegionConstIterator<TImage> inputIterator(input, output->GetLargestPossibleRegion());
		itk::ImageRegionIterator<TImage> outputIterator(output, output->GetLargestPossibleRegion());

		while (!inputIterator.IsAtEnd())
		{
			outputIterator.Set(inputIterator.Get());
			++inputIterator;
			++outputIterator;
		}
	}


	/*
	float RangeAndRescale(FloatImageType*fl_image, float lowerThreshold)
	
	{
		using FilterType = itk::ThresholdImageFilter< FloatImageType >;
		FilterType::Pointer filter = FilterType::New();
		filter->SetInput(fl_image);
		filter->ThresholdOutside(lowerThreshold, 255);
		filter->SetOutsideValue(0);

	}
	*/
	/*
	*Reads image from standard file 
	*crops it to specified rectangular size
	*and normalises it to 0,255 range
	*
	*TODO: include all this and make a template
	*consider case of bigger region
	*/
	//itk::Image<T, Dimension>        FloatImageType;
    

	/*
	*Reads normalized image (0,255) range, 
	*and performs segmentation of atom regions
	*as a result computes centers of atoms
	*needs a lower thershold for initial value
	*
	*Works for only cube like images with equal size
	*TODO: include all this and make a template
	*consider case of bigger region
	*/
	void ComputeCenters(FloatImageType*fl_image, float lowerthreshold, FloatPoints& points)
	{
		//FloatPoints points;

		FloatImageType::SpacingType spacing = fl_image->GetSpacing();
		FloatImageType::RegionType region = fl_image->GetLargestPossibleRegion();

		FloatImageType::SizeType size = region.GetSize();
		//compute binary mask
		using FilterType = itk::BinaryThresholdImageFilter< FloatImageType, BinaryImageType >;
		FilterType::Pointer filter = FilterType::New();
		filter->SetInput(fl_image);
		filter->SetLowerThreshold(lowerthreshold);
		filter->SetUpperThreshold(255.0);
		filter->SetOutsideValue(0);
		filter->SetInsideValue(255);
		filter->Update();

		itk::ImageFileWriter<BinaryImageType>::Pointer wr
			= itk::ImageFileWriter<BinaryImageType>::New();
		wr->SetFileName("output_points.nrrd");
		wr->SetInput(filter->GetOutput());
		wr->Update();

		typedef itk::BinaryImageToShapeLabelMapFilter<BinaryImageType> BinaryImageToShapeLabelMapFilterType;
		BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
		binaryImageToShapeLabelMapFilter->SetInput(filter->GetOutput());
		binaryImageToShapeLabelMapFilter->SetInputForegroundValue(255);
		binaryImageToShapeLabelMapFilter->SetOutputBackgroundValue(0);

		//binaryImageToShapeLabelMapFilter->ComputeFeretDiameterOn();
		binaryImageToShapeLabelMapFilter->Update();
		// The output of this filter is an itk::ShapeLabelMap, which contains itk::ShapeLabelObject's
		std::cout << "There are " << binaryImageToShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects() << " objects." << std::endl;

		// Loop over all of the blobs
		for (unsigned int i = 0; i < binaryImageToShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++)
		{
			BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = binaryImageToShapeLabelMapFilter->GetOutput()->GetNthLabelObject(i);
			// Output the bounding box (an example of one possible property) of the ith region

		//	std::cout << "Object " << i << " has bounding box " << labelObject->GetBoundingBox() << std::endl;

		//	std::cout << "Object " << i << " has centroid " << labelObject->GetCentroid() << std::endl;

			FloatPoint p = labelObject->GetCentroid();

			//perform normalization to get object index
			for (int i = 0; i < 3; i++)
			{
				p[i] /= spacing[i];
				p[i] /= size[i];
			}
			std::cout << "Object " << p << std::endl;
			points.push_back(p);
		}

		
	}

	float ComputeSDF(FloatImageType*fl_image, float lowerthreshold)
	{
		//compute binary mask
		using FilterType = itk::BinaryThresholdImageFilter< FloatImageType, BinaryImageType >;
		FilterType::Pointer filter = FilterType::New();
		filter->SetInput(fl_image);
		filter->SetLowerThreshold(lowerthreshold);
		filter->SetUpperThreshold(255.0);
		filter->SetOutsideValue(0);
		filter->SetInsideValue(255);
		filter->Update();

				
		//compute signed distance field
		typedef  itk::ApproximateSignedDistanceMapImageFilter< BinaryImageType, FloatImageType  > ApproximateSignedDistanceMapImageFilterType;
		ApproximateSignedDistanceMapImageFilterType::Pointer approximateSignedDistanceMapImageFilter =
			ApproximateSignedDistanceMapImageFilterType::New();
		approximateSignedDistanceMapImageFilter->SetInput(filter->GetOutput());
		approximateSignedDistanceMapImageFilter->SetInsideValue(255);
		approximateSignedDistanceMapImageFilter->SetOutsideValue(0);
		approximateSignedDistanceMapImageFilter->Update();

		typedef itk::SmoothingRecursiveGaussianImageFilter<
			FloatImageType, FloatImageType >  GaussFilterType;

		GaussFilterType::Pointer smoothingRecursiveGaussianImageFilter = GaussFilterType::New();
		smoothingRecursiveGaussianImageFilter->SetInput(approximateSignedDistanceMapImageFilter->GetOutput());
		smoothingRecursiveGaussianImageFilter->SetSigma(0.2);
		smoothingRecursiveGaussianImageFilter->Update();

		//fl_image = approximateSignedDistanceMapImageFilter->GetOutput();
		RegionCopy<FloatImageType>(smoothingRecursiveGaussianImageFilter->GetOutput(), fl_image);
		fl_image->Update();
		

		//fl_image->SetRegions(desiredRegion);
		//fl_image->UpdateOutputData();
		//itk::ImageRegionIterator<FloatImageType> imageIterator(fl_image, region);
		//imageIterator.GoToBegin();


		//https://github.com/GEM3D/PDFS
		//https://devtalk.nvidia.com/default/topic/411405/cuda-based-level-set-pde-reinitialization-please-point-out-wheres-wrong/
		//http://flafla2.github.io/2016/10/01/raymarching.html

		//clamp signed distance field

		//https://computergraphics.stackexchange.com/questions/306/sharp-corners-with-signed-distance-fields-fonts
		//http://cuda-programming.blogspot.com/2013/04/texture-references-object-in-cuda.html

		/*
		RescaleFilterType::Pointer    rescaleFilter2 = RescaleFilterType::New();
		rescaleFilter2->SetInput(fl_image);
		rescaleFilter2->SetOutputMinimum(-1.0);
		rescaleFilter2->SetOutputMaximum(1.0);
		rescaleFilter2->Update();
		fl_image = rescaleFilter2->GetOutput();*/

		//compute statistics
		
		typedef itk::StatisticsImageFilter<FloatImageType> StatisticsImageFilterType;
		StatisticsImageFilterType::Pointer statisticsImageFilter
			= StatisticsImageFilterType::New();
		statisticsImageFilter->SetInput(fl_image);
		statisticsImageFilter->Update();
		float max = statisticsImageFilter->GetMaximum();
		float min = statisticsImageFilter->GetMinimum();
		std::cout << "Mean: " << statisticsImageFilter->GetMean() << std::endl;
		std::cout << "Std.: " << statisticsImageFilter->GetSigma() << std::endl;
		std::cout << "Min: " << min << std::endl;
		std::cout << "Max: " << max << std::endl;
		itk::ImageRegionIterator<FloatImageType> imageIterator2(fl_image, fl_image->GetLargestPossibleRegion());
		imageIterator2.GoToBegin();

		//normalise field
		while (!imageIterator2.IsAtEnd())
		{
			// Get the value of the current pixel
			//unsigned char val = imageIterator.Get();
			//std::cout << (int)val << std::endl;

			// Set current pixel as positive
			float ii = imageIterator2.Get();
			ii -= min;
			imageIterator2.Set(ii);
			//ii2++;


			++imageIterator2;
		}
		fl_image->Update();
		fl_image->UpdateOutputInformation();

		itk::ImageFileWriter<FloatImageType>::Pointer wr
			= itk::ImageFileWriter<FloatImageType>::New();
		wr->SetFileName("output3.nrrd");
		// Read in the nrrd data. The file contains the reference image and the gradient
		// images.
		wr->SetInput(fl_image);
		wr->Update();
		

		//std::cout << fl_image->GetLargestPossibleRegion() << std::endl;
		StatisticsImageFilterType::Pointer statisticsImageFilter2
			= StatisticsImageFilterType::New();
		statisticsImageFilter2->SetInput(fl_image);
		statisticsImageFilter2->Update();
		float max2 = statisticsImageFilter2->GetMaximum();
		std::cout << "Mean: " << statisticsImageFilter2->GetMean() << std::endl;
		std::cout << "Std.: " << statisticsImageFilter2->GetSigma() << std::endl;
		std::cout << "Min: " << statisticsImageFilter2->GetMinimum() << std::endl;
		std::cout << "Max: " << max2 << std::endl;
		
		return min;
	}

	void ComputeMIDIfromSDF(FloatImageType* midi_image, const FloatImageType*fl_image, float min)
	{
		

		FloatImageType::SpacingType spacing=fl_image->GetSpacing(); 
		FloatImageType::PointType inputOrigin = fl_image->GetOrigin();

		midi_image->SetSpacing(spacing);
		midi_image->SetOrigin(inputOrigin);
		midi_image->SetRegions(fl_image->GetLargestPossibleRegion());
		midi_image->Allocate();
		midi_image->Update();

		std::cout << midi_image->GetLargestPossibleRegion().GetSize() << std::endl;
		std::cout << "min " << min << std::endl;
		//RegionCopy<FloatImageType>(midi_image, fl_image);

		std::cout << "done" << std::endl;
		itk::ImageRegionIterator<FloatImageType> imageIterator2(midi_image, midi_image->GetLargestPossibleRegion());
		imageIterator2.GoToBegin();
		itk::ImageRegionConstIterator<FloatImageType> imageIterator1(fl_image, fl_image->GetLargestPossibleRegion());
		imageIterator1.GoToBegin();

		//normalise field
		while (!imageIterator1.IsAtEnd())
		{
			// Get the value of the current pixel
			//unsigned char val = imageIterator.Get();
			//std::cout << (int)val << std::endl;

			// Set current pixel as positive
			float ii = imageIterator1.Get();
			//if (ii > (-min)) ii = 0.0;
			//else
			//{
				ii += min;
				ii = -ii; //invert sign
				if (ii < 0) ii = 0.0;
			//}
			imageIterator2.Set(ii);
			//ii2++;


			++imageIterator2;
			++imageIterator1;
		}
		midi_image->Update();
		midi_image->UpdateOutputInformation();

		
		std::cout << "done2" << std::endl;

		std::cout << "write image" << std::endl;
		itk::ImageFileWriter<itkUtil::FloatImageType>::Pointer wr2
			= itk::ImageFileWriter<itkUtil::FloatImageType>::New();
		wr2->SetFileName("output3.nrrd");
		// Read in the nrrd data. The file contains the reference image and the gradient
		// images.
		wr2->SetInput(midi_image);
		wr2->Update();

		//return midi_image;
	}


}
