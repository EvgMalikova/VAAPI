#pragma once
/*
#include "itkImage.h"
#include <vector>
constexpr unsigned int Dimension = 3;
namespace itkUtil {

	
	typedef itk::Image<float, 2>        Float2DImageType;
	typedef itk::Image<float, Dimension>        FloatImageType;
     
	typedef itk::Image<int, Dimension>          IntImageType;
	typedef itk::Image<int, Dimension>          BinaryImageType;
	typedef itk::Point<float, Dimension>		FloatPoint;
	typedef std::vector<FloatPoint>				FloatPoints;

	template<typename TImage>
	void RegionCopy(typename TImage::Pointer input, typename TImage::Pointer output);
	//template <class T> void ReadImage(itk::Image< T, Dimension>*fl_image, const char* filename, typename itk::Image<  T, Dimension>::SizeType size, int reduce, double*sp);
	
    float ComputeSDF(FloatImageType*fl_image, float lowerthreshold);
	void ComputeCenters(FloatImageType*fl_image, float lowerthreshold, FloatPoints& points);
	void ComputeMIDIfromSDF(FloatImageType* midi_image, const FloatImageType*fl_image, float min);
}
*/
