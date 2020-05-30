#include "optixTextureReader.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

#include <iomanip>
#include <fstream>
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

//#include "itkSignedDanielssonDistanceMapImageFilter.h"
template <typename T> void sdfTextureReader<T>::PreprocessData(OutputImageType *fl_image)
{
    typedef typename texReader::BinaryImageType BinaryImageType;
    //compute binary mask
    using FilterType = itk::BinaryThresholdImageFilter< OutputImageType, BinaryImageType >;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(fl_image);
    filter->SetLowerThreshold(lowerthreshold);
    filter->SetUpperThreshold(255.0);
    filter->SetOutsideValue(0);
    filter->SetInsideValue(255);
    filter->Update();

    //compute signed distance field
    typedef  itk::ApproximateSignedDistanceMapImageFilter< BinaryImageType, OutputImageType  > ApproximateSignedDistanceMapImageFilterType;
    ApproximateSignedDistanceMapImageFilterType::Pointer approximateSignedDistanceMapImageFilter =
        ApproximateSignedDistanceMapImageFilterType::New();
    approximateSignedDistanceMapImageFilter->SetInput(filter->GetOutput());
    approximateSignedDistanceMapImageFilter->SetInsideValue(255);
    approximateSignedDistanceMapImageFilter->SetOutsideValue(0);
    approximateSignedDistanceMapImageFilter->Update();

    typedef itk::SmoothingRecursiveGaussianImageFilter<
        OutputImageType, OutputImageType >  GaussFilterType;

    GaussFilterType::Pointer smoothingRecursiveGaussianImageFilter = GaussFilterType::New();
    smoothingRecursiveGaussianImageFilter->SetInput(approximateSignedDistanceMapImageFilter->GetOutput());
    smoothingRecursiveGaussianImageFilter->SetSigma(0.5);
    smoothingRecursiveGaussianImageFilter->Update();

    //fl_image = approximateSignedDistanceMapImageFilter->GetOutput();
    RegionCopy<OutputImageType>(smoothingRecursiveGaussianImageFilter->GetOutput(), fl_image);
    fl_image->Update();

    //https://github.com/GEM3D/PDFS
    //https://devtalk.nvidia.com/default/topic/411405/cuda-based-level-set-pde-reinitialization-please-point-out-wheres-wrong/
    //http://flafla2.github.io/2016/10/01/raymarching.html

    //clamp signed distance field

    //https://computergraphics.stackexchange.com/questions/306/sharp-corners-with-signed-distance-fields-fonts
    //http://cuda-programming.blogspot.com/2013/04/texture-references-object-in-cuda.html

    //compute statistics

    typedef itk::StatisticsImageFilter<OutputImageType> StatisticsImageFilterType;
    StatisticsImageFilterType::Pointer statisticsImageFilter
        = StatisticsImageFilterType::New();
    statisticsImageFilter->SetInput(fl_image);
    statisticsImageFilter->Update();
    m_max = statisticsImageFilter->GetMaximum();
    m_min = statisticsImageFilter->GetMinimum();
    std::cout << "Mean: " << statisticsImageFilter->GetMean() << std::endl;
    std::cout << "Std.: " << statisticsImageFilter->GetSigma() << std::endl;
    std::cout << "Min: " << m_min << std::endl;
    std::cout << "Max: " << m_max << std::endl;
    itk::ImageRegionIterator<OutputImageType> imageIterator2(fl_image, fl_image->GetLargestPossibleRegion());
    imageIterator2.GoToBegin();

    //normalise field
    while (!imageIterator2.IsAtEnd())
    {
        // Get the value of the current pixel
        //unsigned char val = imageIterator.Get();
        //std::cout << (int)val << std::endl;

        // Set current pixel as positive
        float ii = imageIterator2.Get();
        ii -= m_min;
        imageIterator2.Set(ii);
        //ii2++;

        ++imageIterator2;
    }
    fl_image->Update();
    fl_image->UpdateOutputInformation();

    /*itk::ImageFileWriter<OutputImageType>::Pointer wr
        = itk::ImageFileWriter<OutputImageType>::New();
    wr->SetFileName("output3.nrrd");
    // Read in the nrrd data. The file contains the reference image and the gradient
    // images.
    wr->SetInput(fl_image);
    wr->Update();*/

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

    param.lvShift = m_min;
    param.texSize = GetReduce();
}