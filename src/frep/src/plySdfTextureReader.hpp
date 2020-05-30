#include "plySdfTextureReader.h"
#include "itkCastImageFilter.h"

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
#include "itkMeshFileReader.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkTriangleMeshToBinaryImageFilter.h"

template <typename T> void plySdfTextureReader<T>::ReadImage(OutputImageType *fl_image, const char* filename)
{
    OutputImageType::Pointer sdf_image = OutputImageType::New();

    PrepareImages(sdf_image, fl_image);

    OutputImageType::RegionType region = sdf_image->GetLargestPossibleRegion();
    using MeshType = itk::Mesh<T, 3>;
    // std::cout << "PLY------" << std::endl;

    using ReaderType = itk::MeshFileReader<MeshType>;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(filename);
    reader->Update();

    MeshType::Pointer mesh = reader->GetOutput();

    // std::cout << "DONE file reading" << std::endl;
    using FilterType = itk::TriangleMeshToBinaryImageFilter<MeshType, OutputImageType>;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(reader->GetOutput());

    filter->SetSize(sdf_image->GetLargestPossibleRegion().GetSize());

    filter->SetOrigin(sdf_image->GetOrigin());
    filter->SetSpacing(sdf_image->GetSpacing());
    filter->SetTolerance(sdf_image->GetSpacing()[0] / 1620.0);
    //filter->SetInfoImage(sdf_image);
    //filter->SetInsideValue(itk::NumericTraits<T>::max());
    filter->SetInsideValue(255);
    filter->SetOutsideValue(0);
    //try
    //{
    filter->Update();

    // }
    // catch (itk::ExceptionObject & error)
   //  {
   //      std::cerr << "Error: " << error << std::endl;
         // return EXIT_FAILURE;
  //   }
     /* itk::ImageRegionIterator<OutputImageType> imageIterator(sdf_image, region);
     imageIterator.GoToBegin();

     std::ifstream in;
     in.open(filename);
     float ii = 0.1;
     float ii2 = 0.1;

     while (!imageIterator.IsAtEnd())
     {
         // Get the value of the current pixel
         //unsigned char val = imageIterator.Get();
         //std::cout << (int)val << std::endl;

         // Set the current pixel to white
         in >> ii;
         imageIterator.Set(ii);
         //ii2++;

         ++imageIterator;
     }
     in.close();*/
     //sdf_image->Update();

   /* itk::ImageFileWriter<OutputImageType>::Pointer wr
        = itk::ImageFileWriter<OutputImageType>::New();
    wr->SetFileName("output3.nrrd");
    wr->SetInput(filter->GetOutput());
    wr->Update();

    using RescaleFilterType = itk::RescaleIntensityImageFilter<
        OutputImageType, OutputImageType >;
    RescaleFilterType::Pointer    rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(filter->GetOutput());
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(255);
    rescaleFilter->Update();
    sdf_image = rescaleFilter->GetOutput();

    sdf_image->Update();*/
    RegionCopy<OutputImageType>(filter->GetOutput(), fl_image);
    fl_image->Update();

    //return fl_image;
}
//#include "itkSignedDanielssonDistanceMapImageFilter.h"
template <typename T> void plySdfTextureReader<T>::PreprocessData(OutputImageType *fl_image)
{
    typedef typename texReader::BinaryImageType BinaryImageType;
    itk::ImageFileWriter<OutputImageType>::Pointer wr
        = itk::ImageFileWriter<OutputImageType>::New();
    wr->SetFileName("output3.nrrd");
    wr->SetInput(fl_image);
    wr->Update();

    using FilterType = itk::CastImageFilter<OutputImageType, BinaryImageType>;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(fl_image);
    filter->Update();
    std::cout << "Casted" << std::endl;

    //compute signed distance field
    typedef  itk::ApproximateSignedDistanceMapImageFilter< BinaryImageType, OutputImageType  > ApproximateSignedDistanceMapImageFilterType;
    ApproximateSignedDistanceMapImageFilterType::Pointer approximateSignedDistanceMapImageFilter =
        ApproximateSignedDistanceMapImageFilterType::New();
    approximateSignedDistanceMapImageFilter->SetInput(filter->GetOutput());
    approximateSignedDistanceMapImageFilter->SetInsideValue(255.0);
    approximateSignedDistanceMapImageFilter->SetOutsideValue(0.0);
    approximateSignedDistanceMapImageFilter->Update();

    std::cout << "Distance computed" << std::endl;
    typedef itk::SmoothingRecursiveGaussianImageFilter<
        OutputImageType, OutputImageType >  GaussFilterType;

    GaussFilterType::Pointer smoothingRecursiveGaussianImageFilter = GaussFilterType::New();
    smoothingRecursiveGaussianImageFilter->SetInput(approximateSignedDistanceMapImageFilter->GetOutput());
    smoothingRecursiveGaussianImageFilter->SetSigma(0.4);
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

    // itk::ImageFileWriter<OutputImageType>::Pointer wr
    //     = itk::ImageFileWriter<OutputImageType>::New();
    wr->SetFileName("output31.nrrd");
    // Read in the nrrd data. The file contains the reference image and the gradient
    // images.
    wr->SetInput(fl_image);
    wr->Update();/**/

    //std::cout << fl_image->GetLargestPossibleRegion() << std::endl;
    StatisticsImageFilterType::Pointer statisticsImageFilter2
        = StatisticsImageFilterType::New();
    statisticsImageFilter2->SetInput(fl_image);
    statisticsImageFilter2->Update();
    float max2 = statisticsImageFilter2->GetMaximum();

    param.lvShift = m_min;
    param.texSize = GetReduce();
}