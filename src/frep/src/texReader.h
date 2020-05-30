#include "optixReader.h"
#include "itkImage.h"
#include <itkPoint.h>

#include "renderTypes.h"
#ifndef texReader_h
#define texReader_h
/**
* \class  sdfTextureReader
* \brief Read texture from file and applies signed distance transfrom to generate SDF texture
*
* Uses itk signed distance transform
*
*/

constexpr unsigned int Dim = 3;
template <typename T> class texReader : public optixReader<T>
{
public:

    typedef itk::Image<T, Dim>        OutputImageType;
    typedef typename OutputImageType::SizeType SizeType;
    typedef typename OutputImageType::SpacingType SpacingType;
    typedef typename OutputImageType::PointType PointType;
    typedef itk::Image<int, Dim>          IntImageType;
    typedef itk::Image<int, Dim>          BinaryImageType;
    typedef itk::Point<T, Dim>		    FloatPoint;
    typedef std::vector<FloatPoint>				FloatPoints;

    void SetContext(optix::Context c) { m_context = c; };
    optix::Context GetContext() { return m_context; };
    texReader() {
        sp[0] = T(0.5);
        sp[1] = T(0.5);
        sp[2] = T(0.5);

        //TODO: read from file
        size[0] = 139;
        size[1] = 150;
        size[2] = 160;

        //autoCrop
        m_crop = true;
        reduce = 138;

        T point[3] = { T(0),T(0),T(0) };
        //OutputImageType::SpacingType spacing = OutputImageType::SpacingType(sp);
        // T point[3] = { T(0.0),T(0.0),T(0.0) };

        inputOrigin = OutputImageType::PointType(point);

        m_ReadMode = RT_TEXTURE_READ_NORMALIZED_FLOAT;
    };
    ~texReader() {};

    int GetReduce() { return reduce; };
    void SetReduce(int r) { reduce = r; };
    void SetSpacing(T s) {
        sp[0] = s;
        sp[1] = s;
        sp[2] = s;
    }
    void SetOrigin(T x, T y, T z)
    {
        T point[3] = { x,y,z };
        //OutputImageType::SpacingType spacing = OutputImageType::SpacingType(sp);
        // T point[3] = { T(0.0),T(0.0),T(0.0) };

        inputOrigin = PointType(point);
    }
    optix::TextureSampler GetTexture() { return sampler; };
    void SetSize(int x, int y, int z)
    {
        size[0] = x;
        size[1] = y;
        size[2] = z;
    };
protected:
    RTtexturereadmode m_ReadMode;

    void PrepareImages(OutputImageType * sdf_image, OutputImageType *fl_image);

    virtual void ReadFile();
    virtual void ComputeSampler() {};
    virtual void ReadImage(OutputImageType *fl_image, const char* filename);

    virtual void PreprocessData(typename texReader::OutputImageType *fl_image) {};
private:

    SpacingType sp;
    SizeType size;
    PointType inputOrigin;
    int reduce;
    bool m_crop;

    optix::TextureSampler sampler;
    optix::Context m_context;
    optix::Buffer buffer;
};
#include "texReader.hpp"
#endif