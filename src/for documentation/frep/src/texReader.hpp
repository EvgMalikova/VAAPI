#include "texReader.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
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

template <typename T> void texReader<T>::ReadFile()
{
    //TODO: make that in cycle for dynamic data
    //-----------------------------
    T* d = optixReader<T>::GetOutput().get();

    std::string filename = optixReader<T>::Getfile();
    //manage particle frame and others

    OutputImageType::Pointer im_internal = OutputImageType::New();
    ReadImage(im_internal, filename.c_str());

    PreprocessData(im_internal);

    //GetBounding box
    //im_internal->UpdateOutputData();
    //im_internal->Update();
    //void* dat = ;
    d = (T*)(im_internal->GetBufferPointer());
    //-----------

    //Sets texture
    int nx = reduce;
    int ny = reduce;
    int nz = reduce;
    //unsigned char *tex_data = stbi_load((char*)fileName.c_str(), &nx, &ny, &nn, 0);

    std::cout << "computed size " << reduce << std::endl;
    sampler = GetContext()->createTextureSampler();
    sampler->setWrapMode(0, RT_WRAP_CLAMP_TO_EDGE);
    sampler->setWrapMode(1, RT_WRAP_CLAMP_TO_EDGE);
    sampler->setWrapMode(2, RT_WRAP_CLAMP_TO_EDGE);
    //sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_LINEAR);
    sampler->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
    sampler->setReadMode(m_ReadMode);// RT_TEXTURE_READ_ELEMENT_TYPE);
    sampler->setMaxAnisotropy(1.0f);
    sampler->setMipLevelCount(1u);
    sampler->setArraySize(1u);

    std::cout << "sampler is set " << reduce << std::endl;
    buffer = GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT, nx, ny, nz);
    //RT_CHECK_ERROR(rtBufferSetSize3D(buffer, nx, ny, nz));
    buffer->setSize(nx, ny, nz);
    float* data = static_cast<float *>(buffer->map());

    int index = 0;
    for (int i = 0; i < nx; ++i) {
        for (int j = 0; j < ny; ++j) {
            for (int k = 0; k < nz; ++k) {
                //int iindex = ((ny - j - 1) * nx + i) * nn;

                data[index] = float(d[index]);

                // std::cout << tex_data[index] << std::endl;
                index++;
            }
        }
    }

    buffer->unmap();

    sampler->setBuffer(buffer);
    sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
    //return sampler;

    itk::ImageFileWriter<OutputImageType>::Pointer wr
        = itk::ImageFileWriter<OutputImageType>::New();
    wr->SetFileName("output1.nrrd");
    wr->SetInput(im_internal);
    wr->Update();
}

template <typename T> void texReader<T>::PrepareImages(OutputImageType * sdf_image, OutputImageType *fl_image)
{
    OutputImageType::RegionType region;
    OutputImageType::IndexType start;
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;

    region.SetSize(size);
    region.SetIndex(start);

    BinaryImageType::IndexType desiredStart;
    desiredStart.Fill(0);

    BinaryImageType::SizeType desiredSize;
    desiredSize.Fill(reduce);

    BinaryImageType::RegionType desiredRegion(desiredStart, desiredSize);

    fl_image->SetRegions(desiredRegion);
    //std::cout << fl_image << std::endl;

    //sp.Fill(T(0.5));
    fl_image->SetSpacing(sp);
    fl_image->SetOrigin(inputOrigin);

    fl_image->Allocate();
    //std::cout << fl_image << std::endl;

    sdf_image->SetRegions(region);
    sdf_image->SetSpacing(sp);
    sdf_image->SetOrigin(inputOrigin);

    sdf_image->Allocate();
    sdf_image->Update();
}

template <typename T> void texReader<T>::ReadImage(OutputImageType *fl_image, const char* filename)
{
    OutputImageType::Pointer sdf_image = OutputImageType::New();

    PrepareImages(sdf_image, fl_image);

    OutputImageType::RegionType region = sdf_image->GetLargestPossibleRegion();
    itk::ImageRegionIterator<OutputImageType> imageIterator(sdf_image, region);
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
    in.close();
    sdf_image->Update();

    using RescaleFilterType = itk::RescaleIntensityImageFilter<
        OutputImageType, OutputImageType >;
    RescaleFilterType::Pointer    rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(sdf_image);
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(255);
    rescaleFilter->Update();
    sdf_image = rescaleFilter->GetOutput();

    sdf_image->Update();
    RegionCopy<OutputImageType>(sdf_image, fl_image);
    fl_image->Update();

    //return fl_image;
}