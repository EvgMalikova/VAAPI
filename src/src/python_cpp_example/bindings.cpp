#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "math.hpp"

namespace py = pybind11;
class Example
{
public:
    Example(double a)
        : _a(a)
    {
    }

    Example& operator+=(const Example& other)
    {
        _a += other._a;
        return *this;
    }

private:
    double _a;
};

PYBIND11_MAKE_OPAQUE(std::vector<optix::float3>);
PYBIND11_MAKE_OPAQUE(std::vector<float>);

PYBIND11_MODULE(vaapi, m)
{
    m.doc() = "Python bindings for an example library";

    namespace py = pybind11;

    py::class_<Example>(m, "Example")
        .def(py::init([](double a)
    {
        return new Example(a);
    }
        )
        )
        .def("__iadd__", &Example::operator+=);

    m.def("add", [](int a, int b) {return a - b; });
    m.def("examples", &examples);
    /*
    Start VAAPI classes
    */

    py::class_<optix::Context>(m, "Context")
        .def(py::init<>());

    py::class_<optix::Material>(m, "Material")
        .def(py::init<>());

    py::class_<MaterialDesc>(m, "MaterialDesc")
        .def(py::init<bool, bool>());

    py::class_<optix::float3>(m, "float3")
        .def(py::init<>());

    py::class_<std::vector<optix::float3>>(m, "Points")
        .def(py::init<>());
    py::class_<std::vector<float>>(m, "FloatArray")
        .def(py::init<>());

    py::class_<optix::float4>(m, "float4")
        .def(py::init<>());

    py::class_<optix::Program>(m, "Program")
        .def(py::init<>());

    py::class_<PinholeCamera, std::shared_ptr<PinholeCamera>>(m, "PinholeCamera")
        .def(py::init<>());

    py::class_<optixSDFGeometry>(m, "optixSDFGeometry")
        .def(py::init<>())
        .def("SetMaterialType", &optixSDFGeometry::SetMaterialType)
        .def("SetContext", &optixSDFGeometry::SetContext)
        .def("Update", &optixSDFGeometry::Update);

    py::class_<sdfHeterogeneous, optixSDFGeometry> hetero(m, "sdfHeterogeneous");

    hetero.def(py::init<>())
        //.def("test", (void (sdfHeterogeneous::*)( )) &sdfHeterogeneous::test, "Set centers for one frame");
        .def("SetCenter", &sdfHeterogeneous::SetC)
        .def("SetCenters", &sdfHeterogeneous::SetCs)
        //.def("SetCenter", (void (sdfHeterogeneous::*)(std::vector<optix::float3 >> )) &sdfHeterogeneous::SetCenter, "Set centers for one frame");
        .def("GetPrimType", &sdfHeterogeneous::GetPrimType);

    py::enum_<sdfHeterogeneous::ObjectType>(hetero, "ObjectType")
        .value("DIM_0D", sdfHeterogeneous::ObjectType::DIM_0D)
        .value("DIM_1D", sdfHeterogeneous::ObjectType::DIM_1D)
        .value("DIM_4D", sdfHeterogeneous::ObjectType::DIM_4D)
        .value("MULTISCALE", sdfHeterogeneous::ObjectType::MULTISCALE)
        .value("CELL", sdfHeterogeneous::ObjectType::CELL)
        .value("GENERAL", sdfHeterogeneous::ObjectType::GENERAL);

    py::class_<sdfHeterogeneous0D, sdfHeterogeneous>(m, "sdfHeterogeneous0D")
        .def(py::init<>())
        .def("SetRadius", &sdfHeterogeneous0D::SetRadius)
        .def("SetType", &sdfHeterogeneous0D::SetType)
        .def("SetCenterRad", &sdfHeterogeneous0D::SetCenterRad)
        .def("SetTypes", &sdfHeterogeneous0D::SetTypes);

    py::class_<contextManager>(m, "ContextManager")
        .def(py::init<>())
        .def("Update", &contextManager::Update)
        .def("GetValid", &contextManager::GetValid)
        .def("GetOutput", &contextManager::GetOutput);

    py::class_<vaBasicRenderer, std::shared_ptr<vaBasicRenderer>>(m, "vaBasicRenderer")
        .def(py::init<bool>())
        .def("SetContext", &vaBasicRenderer::SetContext)
        .def("AddActor", &vaBasicRenderer::AddActor)
        .def("SetPostProcessMaterial", &vaBasicRenderer::SetPostProcessMaterial);

    py::class_<vaRenderer, vaBasicRenderer, std::shared_ptr<vaRenderer>>(m, "vaRenderer")
        .def(py::init<>())
        .def("SetValid", &vaRenderer::SetValid)
        .def("SetOpticalDims", &vaRenderer::SetOpticalDims)
        .def("SetCamera", &vaRenderer::SetCamera)
        .def("SetDynamic", &vaRenderer::SetDynamic)
        .def("SetWidget", &vaRenderer::SetWidget)
        .def("SetAuditory", &vaRenderer::SetAuditory);

    py::class_<GLFW_Window, std::shared_ptr<GLFW_Window>>(m, "Window")
        .def(py::init<>())
        .def("SetDim", &GLFW_Window::SetDim)
        .def("SetRenderer", &GLFW_Window::SetRenderer)
        .def("SetContext", &GLFW_Window::SetContext)
        .def("SetCamera", &GLFW_Window::SetCamera);

    py::class_<RenderWindowInteractor>(m, "RenderWindowInteractor")
        .def(py::init<>())
        .def("SetWindow", &RenderWindowInteractor::SetWindow)
        .def("SetTimeFrames", &RenderWindowInteractor::SetTimeFrames)
        .def("SetTSpeed", &RenderWindowInteractor::SetTSpeed)
        .def("SetUp", &RenderWindowInteractor::SetUp)
        .def("SetWidget", &RenderWindowInteractor::SetWidget)
        .def("Start", &RenderWindowInteractor::Start);

    py::class_<xyzReader>(m, "xyzReader")
        .def(py::init<>())
        .def("Setfile", &xyzReader::Setfile)
        .def("GetOutput1", &xyzReader::GetOutput1)
        .def("GetOutput2", &xyzReader::GetOutput2)
        .def("Update", &xyzReader::Update);

    py::class_<sdfHMicro, sdfHeterogeneous0D>(m, "sdfHMicro")
        .def(py::init<>())

        .def("GetCallableProg", &sdfHMicro::GetCallableProg);

    py::class_<vaAuditoryMaterial>(m, "vaAuditoryMaterial")
        .def(py::init<>())
        .def("GetOutput", &vaAuditoryMaterial::GetOutput)
        .def("GetType", &vaAuditoryMaterial::GetType)
        .def("Update", &vaAuditoryMaterial::Update);

    py::class_<vaEAVolume, vaAuditoryMaterial>(m, "vaEAVolume")
        .def(py::init<>());

    py::class_<vaVolumeSDFHetero, vaEAVolume>(m, "vaVolumeSDFHetero")
        .def(py::init<>())
        .def("SetContext", &vaVolumeSDFHetero::SetContext)
        .def("SetHeteroObjType", &vaVolumeSDFHetero::SetHeteroObjType)
        .def("SetPostprocess", &vaVolumeSDFHetero::SetPostprocess)
        .def("SetSDFProg", &vaVolumeSDFHetero::SetSDFProg)
        .def("GetEvalProg", &vaVolumeSDFHetero::GetEvalProg)
        .def("GetColorProg", &vaVolumeSDFHetero::GetColorProg);

    py::class_<vaMapper, std::shared_ptr<vaMapper>>(m, "vaMapper")
        .def(py::init<>())
        .def("SetContext", &vaMapper::SetContext)
        .def("SetInput", &vaMapper::SetInput)
        .def("AddMaterial", &vaMapper::AddMaterial)
        .def("SetScalarModeOn", &vaMapper::SetScalarModeOn)
        .def("Update", &vaMapper::Update);

    py::class_<vaBaseWidget, std::shared_ptr<vaBaseWidget>>(m, "vaBaseWidget")
        .def(py::init<>())
        .def("SetContext", &vaBaseWidget::SetContext)
        .def("CreateGeometryHandle", &vaBaseWidget::CreateGeometryHandle);

    py::class_<vaRayCastBaseWidget, vaBaseWidget, std::shared_ptr<vaRayCastBaseWidget>>(m, "vaRayCastBaseWidget")
        .def(py::init<>());

    py::class_<vaBasicActor, std::shared_ptr<vaBasicActor> >(m, "vaBasicActor")
        .def(py::init<>());

    py::class_<vaActor, vaBasicActor, std::shared_ptr<vaActor>>(m, "vaActor")
        .def(py::init<>())
        .def("SetContext", &vaActor::SetContext)
        .def("AddMapper", &vaActor::AddMapper)
        .def("Update", &vaActor::Update);

    py::class_<SceneManager>(m, "SceneManager")
        .def(py::init<>())
        .def("SetExample", &SceneManager::SetExample)
        .def("GetRenderer", &SceneManager::GetRenderer)
        .def("Init", &SceneManager::Init)
        .def("GetCamera", &SceneManager::GetCamera)
        .def("SetXYZData", &SceneManager::SetXYZData)
        .def("GetContext", &SceneManager::GetContext);

    py::class_<MPoints>(m, "MPoints", py::buffer_protocol())
        /*     .def_buffer([](MPoints &m) -> py::buffer_info {
             return py::buffer_info(
                 m.data(),
                 sizeof(float),
                 py::format_descriptor<float>::format(),
                 2,
                 { m.rows(), m.cols() },
                 { sizeof(float) * m.cols(),
                 sizeof(float) }
             );
         })*/
        .def(py::init([](py::array_t<float> buffer) {
        py::buffer_info info = buffer.request();

        auto v = new MPoints(info.shape[0], info.shape[1]);
        memcpy(v->data(), info.ptr, sizeof(float) * (size_t)(v->rows() * v->cols()));
        return v;
    }));

    py::class_<MTypes>(m, "MTypes", py::buffer_protocol())
        .def(py::init([](py::array_t<int> buffer) {
        py::buffer_info info = buffer.request();

        auto v = new MTypes(info.shape[0], info.ndim);
        memcpy(v->data(), info.ptr, sizeof(int) * (size_t)(v->rows() * v->cols()));
        return v;
    }));

    m.def("glfwTerminate", &glfwTerminate);

    py::class_<sdfMolReader>(m, "sdfMolReader")
        .def(py::init([]()
    {
        return new sdfMolReader();
    }
        )
        )
        ;

    /**/
}

/*
PYBIND11_PLUGIN(vaapi) {
    py::module m("vaapi", R"doc(
        Python module
        -----------------------
        .. currentmodule:: python_cpp_example
        .. autosummary::
           :toctree: _generate

           add
           subtract
            main_call
    )doc");

    m.def("add", &add, R"doc(
        Add two numbers

        Some other information about the add function.
    )doc");

    m.def("subtract", &subtract, R"doc(
        Subtract two numbers

        Some other information about the subtract function.
    )doc");

    m.def("main_call", &main_call, R"doc(
        Runs the main application with default parameters

    )doc");

    return m.ptr();
}
*/