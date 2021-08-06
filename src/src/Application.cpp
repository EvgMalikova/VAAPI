/*

 */

#include "shaders/app_config.h"

#include "optixWindow.h"
 //#include "glew_include.h"
#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include "optixXYZReader.h"
#include "sdfReader.h"
#include "optixTextureReader.h"
#include "plySdfTextureReader.h"

#include "optixSDFOperations.h"
#include "vaColorScheme.h"
#include "vaSoundMapScheme.h"
#include "vaRayCastBaseWidget.h"

static optix::float3 CPK_GetColorById(int t)
{
    // return TFBuffer[t];
    switch (t)
    {
    case 1: //H
        return optix::make_float3(1, 1, 1);
        break;
    case 2: //C
        return optix::make_float3(0.5);
        break;
    case 3: //N
        return optix::make_float3(0, 0, 0.5);
        break;
    case 4: //S
        return optix::make_float3(1, 1, 0);
        break;
    case 5: //O
        return optix::make_float3(1, 0, 0);
        break;
    case 6: //P
        return optix::make_float3(1, 0.5, 0);
        break;
    case 7: //Si
        return optix::make_float3(226 / 255, 203 / 255, 182 / 255);
        break;
    }
    return optix::make_float3(1, 1, 1);
}
// DAR Only for sutil::samplesPTXDir() and sutil::writeBufferToFile()
#include <sutil.h>
SceneManager::SceneManager()

{
    int width = 1512;
    int height = 1512;
    m_pinholeCamera = std::shared_ptr<PinholeCamera>(new PinholeCamera());
    m_example = 0;
    m.Update();//creates context

    ren = std::shared_ptr<vaRenderer>(new vaRenderer()); //vaAdancedRenderer
    ren->SetValid(m.GetValid());
    ren->SetContext(m.GetOutput());

    m_widget = std::shared_ptr<vaBaseWidget>(new vaRayCastBaseWidget());
    m_widget->SetContext(m.GetOutput());

    ren->SetOpticalDims(width, height);
    ren->SetAuditoryDims(5, 5);
    ren->SetCamera(m_pinholeCamera);

    //TODO: add separately

    //

    //set not dynamic
    ren->SetDynamic(true);
    ren->SetAuditory(true);

    //overwrites auditory ray-generation
    //This should be done before interactor SetUp() procedure
    // that does the final setups of all renderer stuff,
    // like setting RayGenerationProgam for visual and auditory context ray tracing
    if (m_widget->isRayCast()) {
        ren->SetAuditoryRayGenerationFromWidget("widget_ray_cast", dynamic_cast<vaRayCastBaseWidget*>(m_widget.get())->GetRayCastProg());
        //auditoryMapper* m = dynamic_cast<auditoryMapper*>(new auditoryMapperPlucked());

        ren->SetAuditoryMapModel(new auditoryMapperPlucked());
    }
}

SceneManager::SceneManager(const int width,
    const int height,
    const unsigned int devices,
    const unsigned int stackSize,
    const bool interop)

{
    m_pinholeCamera = std::shared_ptr<PinholeCamera>(new PinholeCamera());
    m_example = 0;
    m.Update();//creates context
    ren = std::shared_ptr<vaAdvancedRenderer>(new vaAdvancedRenderer());
    std::cout << "INITED" << std::endl;
    ren->SetValid(m.GetValid());
    ren->SetContext(m.GetOutput());

    ren->SetOpticalDims(width, height);
    ren->SetCamera(m_pinholeCamera);

    //set not dynamic
    ren->SetDynamic(true);
    ren->SetAuditory(true);
}
void SceneManager::Init()
{
    //add geometry handle
    m_widget->CreateGeometryHandle();
    ren->SetWidget(m_widget);
    try
    {
        m_timer.restart();
        const double timeInit = m_timer.getTime();

        std::cout << "createScene()" << std::endl;
        createScene();
        const double timeScene = m_timer.getTime();

        std::cout << "m_context->validate()" << std::endl;
        //ren->Update();
        //m_context->validate();
        const double timeValidate = m_timer.getTime();

        std::cout << "m_context->launch()" << std::endl;
        // m_context->launch(0, 0, 0); // Dummy launch to build everything (entrypoint, width, height)
        const double timeLaunch = m_timer.getTime();

        std::cout << "initScene(): " << timeLaunch - timeInit << " seconds overall" << std::endl;
        std::cout << "{" << std::endl;
        std::cout << "  createScene() = " << timeScene - timeInit << " seconds" << std::endl;
        std::cout << "  validate()    = " << timeValidate - timeScene << " seconds" << std::endl;
        std::cout << "  launch()      = " << timeLaunch - timeValidate << " seconds" << std::endl;
        std::cout << "}" << std::endl;
    }
    catch (optix::Exception& e)
    {
        std::cerr << e.getErrorString() << std::endl;
    }
}
SceneManager::~SceneManager()
{
    // DAR FIXME Do any other destruction here.
    if (ren->IsValid())
    {
        m.GetOutput()->destroy();
    }

    mappers.clear();
    actorSdf.clear();
    /*

    for (int i = 0; i < mappers.size(); i++)
    {
        if (mappers[i] != nullptr)
            delete mappers[i];
    }

    for (int i = 0; i < actorSdf.size(); i++)
    {
        if (actorSdf[i] != nullptr)
            delete actorSdf[i];
    }

    for (int i = 0; i < actorTri.size(); i++)
    {
        if (actorTri[i] != nullptr)
            delete actorTri[i];
    }
    */
    //destroy renderer
//    delete ren;
}

bool SceneManager::isValid()
{
    return ren->IsValid();
}

void SceneManager::createMicrostructureScene()
{
    //--------------------------------------------
    //Creation of SDF geometry - sphere
    sdfMicrosturcture sdf;
    sdf.SetContext(m.GetOutput());
    sdf.SetCenter1(optix::make_float3(0, 0, 0));
    sdf.SetDims(optix::make_float3(5.0));
    sdf.Update();

    vaBasicMaterial mSdf;
    mSdf.SetContext(m.GetOutput());
    mSdf.Update();
    /*SDFVolumeMaterial mVSdf;
    mVSdf.SetContext(m.GetOutput());
    mVSdf.Update();*/

    BasicLight l1;
    l1.color = optix::make_float3(1.0);
    l1.pos = optix::make_float3(10.0);

    BasicLight l2;
    l2.color = optix::make_float3(1.0);
    l2.pos = optix::make_float3(0, 0, 10.0);

    BasicLight l3;
    l3.color = optix::make_float3(1.0);
    l3.pos = optix::make_float3(-10.0, 0, -1.0);

    /*A general material, that implements rendering of two scalar fields in following modes:
    * -Emission-absorption optical model for Volume Rendering with transfer function that
    *  highlights the internal structure of both electron density and electrostaric potential fields
    *
    * -Colors geometry surface with mapped to color values of  electrostaric potential field
    */

    //
    vaEAVolume texMaterial;
    texMaterial.SetContext(m.GetOutput());
    texMaterial.AddLight(&l1);
    texMaterial.AddLight(&l2);
    //texMaterial.AddLight(&l3);
    texMaterial.SetSDFProg(sdf.GetCallableProg()); /*<Gets sdf primitive optix callable program reference*/
    texMaterial.SetType(vaEAVolume::MaterialType::TRANSP); /*<Volume rendering mode*/
    //texMaterial.SetTexture(readR1.GetTexture());
    //texMaterial.SetTexture(readR2.GetTexture());
    texMaterial.Update();

    std::shared_ptr<vaMapper> map21 = std::shared_ptr<vaMapper>(new vaMapper());

    map21->SetContext(m.GetOutput());
    map21->SetInput(sdf.GetOutput());
    map21->AddMaterial(texMaterial.GetOutput(), texMaterial.GetType());
    map21->Update();

    std::shared_ptr< vaActor> acSdf1 = std::shared_ptr< vaActor>(new  vaActor());

    acSdf1->SetContext(m.GetOutput()); //sets context and initialize acceleration properties

                                       //TODO: overwrite with mapper function that returns it's instance
                                       //ac.SetGeometry(mat, pl.GetOutput());
    acSdf1->AddMapper(map21);// .GetOutput());
    acSdf1->Update();

    ren->AddActor(acSdf1);

    //store
    actorSdf.push_back(acSdf1);
    mappers.push_back(map21);

    //asign prog
    ren->SetMissProgSDFProg(sdf.GetCallableProg());
    //asign prog
    //ren->SetMissProgSDFProg(sdf.GetCallableProg());
}

void SceneManager::createFrepScene()
{
    //--------------------------------------------
    //Creation of SDF geometry - sphere
    optixSDFBox sdf;
    sdf.SetContext(m.GetOutput());
    sdf.SetCenter1(optix::make_float3(1.5));
    sdf.SetDims(optix::make_float3(0.3));
    sdf.Update();
    const int nums = 60;
    /*  SDFRoundingOp round[nums];
      round[0].SetContext(m.GetOutput());
      round[0].AddOpperand(&sdf);
      round[0].SetKoeff(0.01);
      round[0].Update();

      for (int i = 1; i < nums - 1; i++)
      {
          round[i].SetContext(m.GetOutput());

          round[i].AddOpperand(round[i - 1].GetOutputSdfObject());// &sdf);
          round[i].SetKoeff(0.01);
          round[i].Update();
      }*/

    optixSDFTorus sdfT;
    sdfT.SetContext(m.GetOutput());
    sdfT.SetCenter1(optix::make_float3(0.0));
    sdfT.SetRadius1(optix::make_float3(0.4, 0.1, 0.0));
    sdfT.Update();

    SDFElongateOp el;

    el.SetContext(m.GetOutput());
    el.AddOpperand(&sdfT);
    el.SetHKoeff(optix::make_float3(0.0, 1.0, 2.1));
    el.Update();

    SDFBlendUnionOp opBlend;
    opBlend.SetContext(m.GetOutput());
    opBlend.AddOpperand1(&sdf);
    opBlend.AddOpperand2(el.GetOutputSdfObject());
    opBlend.SetKoeff(0.3);
    opBlend.Update();

    SDFBlendUnionOp opBlend2;
    opBlend2.SetContext(m.GetOutput());
    opBlend2.AddOpperand1(&sdf);
    opBlend2.AddOpperand2(m_widget->GetHandle());
    opBlend2.SetKoeff(0.5);
    opBlend2.Update();

    vaBasicMaterial mSdf;
    mSdf.SetContext(m.GetOutput());
    mSdf.Update();

    std::shared_ptr<vaMapper> map21 = std::shared_ptr<vaMapper>(new vaMapper());

    map21->SetContext(m.GetOutput());
    map21->SetInput(sdf.GetOutput());
    map21->AddMaterial(mSdf.GetOutput(), mSdf.GetType());
    map21->Update();

    std::shared_ptr< vaActor> acSdf1 = std::shared_ptr< vaActor>(new  vaActor());

    acSdf1->SetContext(m.GetOutput()); //sets context and initialize acceleration properties

                                      //TODO: overwrite with mapper function that returns it's instance
                                      //ac.SetGeometry(mat, pl.GetOutput());
    acSdf1->AddMapper(map21);// .GetOutput());
    acSdf1->Update();

    ren->AddActor(acSdf1);

    //store
    actorSdf.push_back(acSdf1);
    mappers.push_back(map21);
}

void SceneManager::createDynamicHeterogeneousObjectScene()
{
    /*
    *Reading dynamic electron density and electrostatic potential fields data: initial and final state
    */
    /*<Basic file reader of scalar field data on structural grid, those output is an SDF Texture Sampler*/
    /* Reading electron density field, in the initial state, before geometry optimisation*/
    sdfTextureReader<float> readSDFTex1;
    readSDFTex1.SetContext(m.GetOutput());
    readSDFTex1.SetSize(139, 150, 160);
    readSDFTex1.SetThreshold(0.26);/*<defines field isosurface that will define the molecule boundary*/
    readSDFTex1.Setfile("ed1.txt");
    readSDFTex1.Update();
    /* Reading electron density field, in the final state, after geometry optimisation*/
    sdfTextureReader<float> readSDFTex2;
    readSDFTex2.SetContext(m.GetOutput());
    readSDFTex2.SetSize(138, 150, 160);
    readSDFTex2.SetThreshold(0.26);
    readSDFTex2.Setfile("ed2.txt");
    readSDFTex2.Update();

    /*<Basic file reader of scalar field data on structural grid, those output is an optix Texture Sampler*/
    /* Reading electrostatic potential field, in the initial state, before geometry optimisation*/
    texReader<float> readR1;
    readR1.SetContext(m.GetOutput());
    readR1.SetSize(139, 150, 160);
    readR1.Setfile("p1.txt");
    readR1.Update();

    /* Reading electrostatic potential field, in the final state, after geometry optimisation*/
    texReader<float> readR2;
    readR2.SetContext(m.GetOutput());
    readR2.SetSize(138, 150, 160);
    readR2.Setfile("p2.txt");
    readR2.Update();

    /* Creating a dynamic sdf primitive on GPU, representing molecule boundary, described with electron density field*/
    sdfTexture tex;
    tex.SetContext(m.GetOutput());
    tex.SetTexture(readSDFTex1.GetTexture(), readSDFTex1.GetParam());
    tex.SetTexture(readSDFTex2.GetTexture(), readSDFTex1.GetParam());
    tex.Update();

    BasicLight l1;
    l1.color = optix::make_float3(1.0);
    l1.pos = optix::make_float3(1.0);

    BasicLight l2;
    l2.color = optix::make_float3(1.0);
    l2.pos = optix::make_float3(0, 0, 1.0);

    /*A general material, that implements rendering of two scalar fields in following modes:
    * -Emission-absorption optical model for Volume Rendering with transfer function that
    *  highlights the internal structure of both electron density and electrostaric potential fields
    *
    * -Colors geometry surface with mapped to color values of  electrostaric potential field
    */

    //Error in material
    vaEAVolume texMaterial;
    texMaterial.SetContext(m.GetOutput());
    texMaterial.AddLight(&l1);
    texMaterial.AddLight(&l2);
    texMaterial.SetSDFProg(tex.GetCallableProg()); /*<Gets sdf primitive optix callable program reference*/
    texMaterial.SetType(vaEAVolume::MaterialType::VOLUME); /*<Volume rendering mode*/
    texMaterial.SetTexture(readR1.GetTexture());
    texMaterial.SetTexture(readR2.GetTexture());
    texMaterial.Update();

    //SDFSubtractionOp opBlend;
    SDFBlendUnionOp   opBlend;
    opBlend.SetContext(m.GetOutput());
    opBlend.AddOpperand1(&tex);
    opBlend.AddOpperand2(m_widget->GetHandle());
    opBlend.SetKoeff(0.3);
    opBlend.Update();

    /* Mapper, simillar to vtkMapper*/
    std::shared_ptr<vaMapper> map2 = std::shared_ptr<vaMapper>(new vaMapper());

    map2->SetContext(m.GetOutput());
    map2->SetDescInput(tex.GetOutputDesc());
    map2->AddMaterial(texMaterial.GetOutput(), texMaterial.GetType());
    map2->Update();

    /*Actor, simillar to vtkActor*/
    std::shared_ptr< vaActor> acSdf = std::shared_ptr< vaActor>(new  vaActor());

    acSdf->SetContext(m.GetOutput());
    acSdf->AddMapper(map2);
    acSdf->Update();

    /*Add actor to the rendere*/
    ren->AddActor(acSdf);

    //store
    actorSdf.push_back(acSdf);
    mappers.push_back(map2);
}

void SceneManager::createMolSolventScene()
{
    //ren->SetAudioBuffer()
    /*
    *Reading dynamic electron density and electrostatic potential fields data: initial and final state
    */
    /*<Basic file reader of scalar field data on structural grid, those output is an SDF Texture Sampler*/
    /* Reading electron density field, in the initial state, before geometry optimisation*/
    plySdfTextureReader<float> readSDFTex1;
    readSDFTex1.SetContext(m.GetOutput());
    readSDFTex1.SetSize(250, 250, 250);
    readSDFTex1.SetReduce(250);
    readSDFTex1.SetSpacing(0.4); //0.2 for original beta1
    //  readSDFTex1.SetOrigin(-50.0, -50.0, -30.0); //for  surfBig331
     //readSDFTex1.SetOrigin(-60.0, -80.0, -50.0); //for rec.obj
   // readSDFTex1.SetOrigin(-30.0, -30.0, -30.0); //for solvent and surf - cavities
   //readSDFTex1.SetOrigin(7.5, -45.0, -42.5); //for alpha.obj
   // readSDFTex1.SetOrigin(-50.0, -55.0, -35.0);//beta
    readSDFTex1.SetOrigin(-35.0, 50.0, 0.0); //gamma
    readSDFTex1.SetThreshold(0.26);/*<defines field isosurface that will define the molecule boundary*/
    readSDFTex1.Setfile("beta.obj");// solvent.obj");//surfBig331.obj");
    readSDFTex1.Update();
    //https://en.wikipedia.org/wiki/Abalone_(molecular_mechanics) - pictures

    m_widget->SetRadius(1.4 / 100);// (250 * 0.4)); //normalization to water size for solvent

    plySdfTextureReader<float> readSDFTex2;
    readSDFTex2.SetContext(m.GetOutput());
    readSDFTex2.SetSize(250, 250, 250);
    readSDFTex2.SetReduce(250);
    readSDFTex2.SetSpacing(0.4); //0.4
                                 //  readSDFTex1.SetOrigin(-50.0, -50.0, -30.0); //for  surfBig331
                                 //readSDFTex1.SetOrigin(-60.0, -80.0, -50.0); //for rec.obj
                                 // readSDFTex1.SetOrigin(-30.0, -30.0, -30.0); //for solvent and surf - cavities
                                 //readSDFTex1.SetOrigin(7.5, -45.0, -42.5); //for alpha.obj
    //readSDFTex2.SetOrigin(-50.0, -55.0, -35.0);//beta
    readSDFTex2.SetOrigin(-35.0, 50.0, 0.0); //gamma
    readSDFTex2.SetThreshold(0.26);/*<defines field isosurface that will define the molecule boundary*/
    readSDFTex2.Setfile("gamma.obj");// solvent.obj");//surfBig331.obj");
    readSDFTex2.Update();

    vaBasicMaterial mSdf;
    mSdf.SetContext(m.GetOutput());
    mSdf.Update();

    /* Creating a dynamic sdf primitive on GPU, representing molecule boundary, described with electron density field*/
    sdfTexture tex;
    tex.SetContext(m.GetOutput());
    tex.SetTexture(readSDFTex1.GetTexture(), readSDFTex1.GetParam());
    //tex.SetTexture(readSDFTex2.GetTexture(), readSDFTex2.GetParam());

    tex.Update();

    BasicLight l1;
    l1.color = optix::make_float3(1.0);
    l1.pos = optix::make_float3(1.0);

    BasicLight l2;
    l2.color = optix::make_float3(1.0);
    l2.pos = optix::make_float3(0, 0, 1.0);

    /*A general material, that implements rendering of two scalar fields in following modes:
    * -Emission-absorption optical model for Volume Rendering with transfer function that
    *  highlights the internal structure of both electron density and electrostaric potential fields
    *
    * -Colors geometry surface with mapped to color values of  electrostaric potential field
    */

    SDFSubtractionOp opBlend;
    opBlend.SetContext(m.GetOutput());
    opBlend.AddOpperand1(&tex);
    opBlend.AddOpperand2(m_widget->GetHandle());
    // opBlend.SetKoeff(0.3);
    opBlend.Update();

    /*
    Creates auditory material
    */
    // SDFAudioVolumeMaterial mVSdf2;
    SDFAudioRayTraceMaterial mVSdf2;
    mVSdf2.SetContext(m.GetOutput());
    mVSdf2.Update();

    /*
    Creates mapper
    */

    /* Mapper, simillar to vtkMapper*/
    std::shared_ptr<vaMapper> map2 = std::shared_ptr<vaMapper>(new vaMapper());
    map2->SetContext(m.GetOutput());
    map2->SetDescInput(tex.GetOutputDesc());
    map2->AddMaterial(mSdf.GetOutput(), mSdf.GetType());
    map2->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/

    map2->Update();

    /*Actor, simillar to vtkActor*/
    std::shared_ptr< vaActor> acSdf = std::shared_ptr< vaActor>(new  vaActor());

    acSdf->SetContext(m.GetOutput());
    acSdf->AddMapper(map2);
    acSdf->Update();

    /*Add actor to the rendere*/
    ren->AddActor(acSdf);

    //store
    actorSdf.push_back(acSdf);
    mappers.push_back(map2);
}

#define IGN_VISIBILITY_SELECTABLE      0x00000002
void SceneManager::ExampleTetra()
{
    /*Read molecule data from XYZ file.
    For info on XYZ file format:
    //http://wiki.jmol.org/index.php/File_formats/Formats/XYZ
    */
    tetReader read;
    read.Setfile("2.vtk");
    read.Update();

    /*Molecule CPK representation as sdf primitive
    For info on representation:
    https://en.wikipedia.org/wiki/Space-filling_model
    */

    optix::float3 cent = read.GetCenter();
    m_pinholeCamera->SetCenter(cent);
    m_pinholeCamera->m_distance = 500;

    ren->SetBoundingBox(read.GetBMin(), read.GetBMax());

    sdfHeterogeneous4D mol; //sdfHBallSticksMol mol;////sdfCPKMol mol;
    mol.SetContext(m.GetOutput());
    mol.SetCenter(read.GetOutput1());

    //mol.SetType(read.GetOutput3());
    if (read.GetOutput4().size() > 0)
        mol.SetRadius(read.GetOutput4());
    else
    {
        //set constant radius
    }
    //tetras
    mol.SetTetras(read.GetOutput2());

    mol.SetMaterialType(0);
    mol.Update();

    BasicLight l1;
    l1.color = optix::make_float3(1.0);
    l1.pos = optix::make_float3(10.0);

    BasicLight l2;
    l2.color = optix::make_float3(1.0);
    l2.pos = optix::make_float3(0, 0, 10.0);

    BasicLight l3;
    l3.color = optix::make_float3(1.0);
    l3.pos = optix::make_float3(-10.0, 0, -1.0);

    vaVolumeSDFHetero texMaterial;
    texMaterial.SetContext(m.GetOutput());
    texMaterial.SetHeteroObjType(mol.GetPrimType());
    texMaterial.SetPostprocess(1);

    texMaterial.AddLight(&l1);
    texMaterial.AddLight(&l2);
    //texMaterial.AddLight(&l3);
    texMaterial.SetSDFProg(mol.GetCallableProg());
    texMaterial.SetType(vaEAVolume::MaterialType::TRANSP);
    texMaterial.Update();

    // Creates mapper

    std::shared_ptr<vaMapper> map3 = std::shared_ptr<vaMapper>(new vaMapper());
    map3->SetContext(m.GetOutput());
    map3->SetInput(mol.GetOutput());
    map3->AddMaterial(texMaterial.GetOutput(), texMaterial.GetType());
    map3->SetScalarModeOn();
    map3->Update();

    //Creates actor

    std::shared_ptr< vaActor> acSdfMol = std::shared_ptr< vaActor>(new  vaActor());

    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    ren->AddActor(acSdfMol);

    ren->SetPostProcessMaterial(texMaterial.GetEvalProg(), texMaterial.GetColorProg());
    ren->SetHeteroObjType(1);
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);
}

void SceneManager::Example1()
{
    int postProcess = 1;
    /*Read molecule data from XYZ file.
    For info on XYZ file format:
    //http://wiki.jmol.org/index.php/File_formats/Formats/XYZ
    */
    sdfReader read;
    read.Setfile("quaz_big.mol");
    read.Update();
    optix::float3 cent = read.GetCenter();
    m_pinholeCamera->SetCenter(cent);
    m_pinholeCamera->m_distance = 10;
    ren->SetBoundingBox(read.GetBMin(true), read.GetBMax(true));

    /*Molecule CPK representation as sdf primitive
    For info on representation:
    https://en.wikipedia.org/wiki/Space-filling_model
    */
    sdfHCrazyBonds mol;//sdfHBondsSticks mol; //
    mol.SetContext(m.GetOutput());
    mol.SetCenter(read.GetOutput1()); /*<Sets atoms centers*/

    mol.SetRadius(read.GetOutput2()); /*<Sets atoms radii specified for element type,
                                      for info: https://en.wikipedia.org/wiki/Atomic_radius*/
    mol.SetType(read.GetOutput3()); /*<Sets atom type*/
    mol.SetBonds(read.GetOutput4());
    mol.SetMaterialType(0);
    mol.Update();

    BasicLight l1;
    l1.color = optix::make_float3(1.0);
    l1.pos = optix::make_float3(10.0);

    BasicLight l2;
    l2.color = optix::make_float3(1.0);
    l2.pos = optix::make_float3(0, 0, 10.0);

    BasicLight l3;
    l3.color = optix::make_float3(1.0);
    l3.pos = optix::make_float3(-10.0, 0, -1.0);

    vaVolumeSDFHetero texMaterial;
    texMaterial.SetContext(m.GetOutput());
    texMaterial.SetHeteroObjType(mol.GetPrimType());
    texMaterial.SetPostprocess(postProcess); //manually set post processing

    texMaterial.SetSDFProg(mol.GetCallableProg()); /*<Gets sdf primitive optix callable program reference*/
    texMaterial.Update();

    /*
    Creates mapper
    */
    std::shared_ptr<vaMapper> map3 = std::shared_ptr<vaMapper>(new vaMapper());
    map3->SetContext(m.GetOutput());
    map3->SetInput(mol.GetOutput());
    map3->AddMaterial(texMaterial.GetOutput(), texMaterial.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    // map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    /*
    Creates actor
    */
    std::shared_ptr< vaActor> acSdfMol = std::shared_ptr< vaActor>(new  vaActor());
    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    ren->SetHeteroObjType(0);
    ren->AddActor(acSdfMol);
    if (postProcess > 0)
        ren->SetPostProcessMaterial(texMaterial.GetEvalProg(), texMaterial.GetColorProg());
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);
}

void SceneManager::Example3()
{
    int postProcess = 1;
    /*Read molecule data from XYZ file.
    For info on XYZ file format:
    //http://wiki.jmol.org/index.php/File_formats/Formats/XYZ
    */
    sdfReader read;
    read.Setfile("es.sdf");
    read.Update();
    sdfReader read2;
    read2.Setfile("cs.sdf");
    read2.Update();
    optix::float3 cent = read.GetCenter();
    m_pinholeCamera->SetCenter(cent);
    m_pinholeCamera->m_distance = 10;
    ren->SetBoundingBox(read.GetBMin(true), read.GetBMax(true));

    /*Molecule CPK representation as sdf primitive
    For info on representation:
    https://en.wikipedia.org/wiki/Space-filling_model
    */
    sdfHBondsSticks mol; //
    mol.SetContext(m.GetOutput());

    mol.SetNumFrames(3);
    mol.SetCenter(read.GetOutput1(), 1); /*<Sets atoms centers*/
                                         //mol.SimulateDynamic(6, read.GetOutput1()); - works, but we will try collapse of cluster
    mol.SetCenter(read2.GetOutput1(), 2);
    //mol.SetCenter(read2.GetOutput1(), 3);
    mol.SetCenter(read.GetOutput1(), 3);
    mol.SetRadius(read.GetOutput2()); /*<Sets atoms radii specified for element type,
                                      for info: https://en.wikipedia.org/wiki/Atomic_radius*/
    mol.SetType(read.GetOutput3()); /*<Sets atom type*/
    mol.SetBonds(read.GetOutput4());
    mol.SetMaterialType(0);
    mol.Update();

    BasicLight l1;
    l1.color = optix::make_float3(1.0);
    l1.pos = optix::make_float3(10.0);

    BasicLight l2;
    l2.color = optix::make_float3(1.0);
    l2.pos = optix::make_float3(0, 0, 10.0);

    BasicLight l3;
    l3.color = optix::make_float3(1.0);
    l3.pos = optix::make_float3(-10.0, 0, -1.0);

    vaVolumeSDFHetero texMaterial;
    texMaterial.SetContext(m.GetOutput());
    texMaterial.SetHeteroObjType(mol.GetPrimType());
    texMaterial.SetPostprocess(postProcess); //manually set post processing

    texMaterial.SetSDFProg(mol.GetCallableProg()); /*<Gets sdf primitive optix callable program reference*/
    texMaterial.Update();

    /*
    Creates mapper
    */
    std::shared_ptr<vaMapper> map3 = std::shared_ptr<vaMapper>(new vaMapper());
    map3->SetContext(m.GetOutput());
    map3->SetDescInput(mol.GetOutputDesc());
    map3->AddMaterial(texMaterial.GetOutput(), texMaterial.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    // map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    /*
    Creates actor
    */
    std::shared_ptr< vaActor> acSdfMol = std::shared_ptr< vaActor>(new  vaActor());
    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    ren->SetHeteroObjType(0);
    ren->AddActor(acSdfMol);
    if (postProcess > 0)
        ren->SetPostProcessMaterial(texMaterial.GetEvalProg(), texMaterial.GetColorProg());
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);
}
void SceneManager::Example6()
{
    int postProcess = 1;
    sdfMolReader read;
    read.Setfile("ice_very_big_sorted.mol");// "water2.sdf");
    read.Update();

    optix::float3 cent = read.GetCenter();
    m_pinholeCamera->SetCenter(cent);
    m_pinholeCamera->m_distance = 10;
    ren->SetBoundingBox(read.GetBMin(false), read.GetBMax(false));
    ren->SetCenter(cent);

    //read2.Grow(2); TODO: implement
    //http://www1.lsbu.ac.uk/water/escs.html

    /*Molecule CPK representation as sdf primitive
    For info on representation:
    https://en.wikipedia.org/wiki/Space-filling_model
    */

    //sdfMolBallSticksMol mol;//

    sdfMoleculeBallSticks mol;//
    mol.SetMolSize(4);
    mol.SetContext(m.GetOutput());
    mol.SetNumFrames(1);

    mol.SetCenter(read.GetOutput1()); /*<Sets atoms centers*/
                                         //mol.SimulateDynamic(6, read.GetOutput1()); - works, but we will try collapse of cluster

    mol.SetRadius(read.GetOutput2()); /*<Sets atoms radii specified for element type,
                                      for info: https://en.wikipedia.org/wiki/Atomic_radius*/
    mol.SetType(read.GetOutput3()); /*<Sets atom type*/
                                    //after all to correctly fix number
    mol.SetBonds(read.GetOutput4()); /*<Sets atoms centers*/
    mol.SetMols(read.GetOutput5(), 4);
    mol.SetMaterialType(0);
    mol.Update();

    vaVolumeSDFHeteroMultiscale texMaterial;
    texMaterial.SetHeteroObjType(sdfHeterogeneous::ObjectType::CELL);

    texMaterial.SetPostprocess(postProcess); //manually set post processing

    texMaterial.SetContext(m.GetOutput());
    texMaterial.SetSDFProg(mol.GetCallableProg()); //<Gets sdf primitive optix callable program reference
    texMaterial.Update();

    /*
    Creates mapper
    */
    std::shared_ptr<vaMapper> map3 = std::shared_ptr<vaMapper>(new vaMapper());

    map3->SetContext(m.GetOutput());
    map3->SetDescInput(mol.GetOutputDesc());
    map3->AddMaterial(texMaterial.GetOutput(), texMaterial.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    // map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    /*
    Creates actor
    */
    std::shared_ptr< vaActor> acSdfMol = std::shared_ptr< vaActor>(new  vaActor());
    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    ren->SetHeteroObjType(0);
    ren->AddActor(acSdfMol);
    if (postProcess > 0)
        ren->SetPostProcessMaterial(texMaterial.GetEvalProg(), texMaterial.GetColorProg());
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);
}

void SceneManager::Example31()
{
    int postProcess = 1;
    sdfMolReader read;
    read.Setfile("es.sdf");// "water2.sdf");
    read.Update();

    sdfMolReader read2;
    read2.Setfile("cs.sdf");// "water2.sdf");
    read2.Update();
    optix::float3 cent = read.GetCenter();
    // m_pinholeCamera->SetCenter(cent);
    // m_pinholeCamera->m_distance = 10;
    // ren->SetBoundingBox(fminf(read.GetBMin(), read2.GetBMin()), fmaxf(read.GetBMax(), read2.GetBMax()));

     //read2.Grow(2); TODO: implement
     //http://www1.lsbu.ac.uk/water/escs.html

     /*Molecule CPK representation as sdf primitive
     For info on representation:
     https://en.wikipedia.org/wiki/Space-filling_model
     */

     //sdfMolBallSticksMol mol;//

    sdfMoleculeBallSticks mol;//
    mol.SetMolSize(2);
    mol.SetContext(m.GetOutput());
    mol.SetNumFrames(3);

    mol.SetCenter(read.GetOutput1(), 1); /*<Sets atoms centers*/
                                         //mol.SimulateDynamic(6, read.GetOutput1()); - works, but we will try collapse of cluster
    mol.SetCenter(read2.GetOutput1(), 2);
    //mol.SetCenter(read2.GetOutput1(), 3);
    mol.SetCenter(read.GetOutput1(), 3);

    mol.SetRadius(read.GetOutput2()); /*<Sets atoms radii specified for element type,
                                      for info: https://en.wikipedia.org/wiki/Atomic_radius*/
    mol.SetType(read.GetOutput3()); /*<Sets atom type*/
                                    //after all to correctly fix number
    mol.SetBonds(read.GetOutput4()); /*<Sets atoms centers*/
    mol.SetMols(read.GetOutput5(), 2);
    mol.SetMaterialType(0);
    mol.Update();

    vaVolumeSDFHeteroMultiscale texMaterial;
    texMaterial.SetPostprocess(postProcess); //manually set post processing

    texMaterial.SetContext(m.GetOutput());

    texMaterial.SetSDFProg(mol.GetCallableProg()); //<Gets sdf primitive optix callable program reference
    texMaterial.Update();

    /*
    Creates mapper
    */
    std::shared_ptr<vaMapper> map3 = std::shared_ptr<vaMapper>(new vaMapper());
    map3->SetContext(m.GetOutput());
    map3->SetDescInput(mol.GetOutputDesc());
    map3->AddMaterial(texMaterial.GetOutput(), texMaterial.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    // map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    /*
    Creates actor
    */
    std::shared_ptr< vaActor> acSdfMol = std::shared_ptr< vaActor>(new  vaActor());
    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    ren->SetHeteroObjType(0);
    ren->AddActor(acSdfMol);
    if (postProcess > 0)
        ren->SetPostProcessMaterial(texMaterial.GetEvalProg(), texMaterial.GetColorProg());
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);
}

void SceneManager::createMicrostuctureMOlScene()
{
    /*Read molecule data from XYZ file.
    For info on XYZ file format:
    //http://wiki.jmol.org/index.php/File_formats/Formats/XYZ
    */
    sdfReader read;
    read.Setfile("c1_sym.sdf"); //c1.sdf" quaz_big.mol");// es.sdf");// "water2.sdf");
    read.Update();

    //  sdfReader read2;
    //  read2.Setfile("c2.sdf");// cs.sdf");// "water2.sdf");
    //  read2.Update();
      //http://www1.lsbu.ac.uk/water/escs.html

      /*Molecule CPK representation as sdf primitive
      For info on representation:
      https://en.wikipedia.org/wiki/Space-filling_model
      */
      /* xyzReader read;
       read.Setfile("slice.xyz");

       read.Update();
       read.Grow(2); //increase speed and type

                     /*Molecule CPK representation as sdf primitive
                     For info on representation:
                     https://en.wikipedia.org/wiki/Space-filling_model
                     */
                     /* sdfCPKMol mol;
                      //sdfCPKDynMol mol;
                      mol.SetContext(m.GetOutput());
                      mol.SimulateDynamic(2, read.GetOutput1());
                      mol.SetRadius(read.GetOutput2());
                      mol.SetType(read.GetOutput3());
                      mol.SetMaterialType(0);
                      mol.Update();
                      */

    sdfBallSticksMol mol;
    mol.SetContext(m.GetOutput());
    // mol.SetScale(0.3);
     //mol.SetShift(optix::make_float3(0, 0, 0));
    //x 0,2
    //y 2,2.5
    //z -1,-2
    //2,3,-2
    //0, 2.0, -1
    mol.SetShift(optix::make_float3(2, 3, -2));
    mol.SetCenter(read.GetOutput1());
    //mol.SetNumFrames(2);
    //mol.SetShift(optix::make_float3(1, -1, -1));
    //mol.SetShift(optix::make_float3(-30, -30, -30));
    //2, -2, -2));
   //  mol.SetCenter(read.GetOutput1(), 1); /*<Sets atoms centers*/
                                          //mol.SimulateDynamic(6, read.GetOutput1()); - works, but we will try collapse of cluster
   //  mol.SetCenter(read2.GetOutput1(), 2);

    mol.SetRadius(read.GetOutput2()); /*<Sets atoms radii specified for element type,
                                      for info: https://en.wikipedia.org/wiki/Atomic_radius*/
    mol.SetType(read.GetOutput3()); /*<Sets atom type*/
                                    //after all to correctly fix number
    mol.SetBonds(read.GetOutput4()); /*<Sets atoms centers*/

    mol.SetMaterialType(0);
    mol.Update();
    //Creation of SDF geometry - sphere
    sdfMicrosturcture sdf;
    sdf.SetContext(m.GetOutput());
    sdf.SetCenter(optix::make_float3(0, 0, 0));//  and 3
    sdf.SetCenter1(optix::make_float3(0, 0, 0));//  and 3
    sdf.SetRadius(optix::make_float3(5.1));
    sdf.SetRadius1(optix::make_float3(5.1));
    sdf.SetDims(optix::make_float3(5.1));
    sdf.Update();

    BasicLight l1;
    l1.color = optix::make_float3(1.0);
    l1.pos = optix::make_float3(20.0);

    BasicLight l2;
    l2.color = optix::make_float3(1.0);
    l2.pos = optix::make_float3(0, 0, 15.0);

    BasicLight l3;
    l3.color = optix::make_float3(1.0);
    l3.pos = optix::make_float3(-15.0, 0, 0);

    /*A general material, that implements rendering of two scalar fields in following modes:
    * -Emission-absorption optical model for Volume Rendering with transfer function that
    *  highlights the internal structure of both electron density and electrostaric potential fields
    *
    * -Colors geometry surface with mapped to color values of  electrostaric potential field
    */

    //Error in material
    vaVolumeSDFComplex texMaterial;
    texMaterial.SetContext(m.GetOutput());
    //texMaterial.SetSDFProg(sdf.GetCallableProg());
    texMaterial.AddLight(&l1);
    texMaterial.AddLight(&l2);
    texMaterial.AddLight(&l3);
    texMaterial.Update();

    std::shared_ptr<vaMapper> maptex = std::shared_ptr<vaMapper>(new vaMapper());

    maptex->SetContext(m.GetOutput());
    maptex->SetInput(sdf.GetOutput());
    maptex->AddMaterial(texMaterial.GetOutput(), texMaterial.GetType());
    maptex->Update();

    std::shared_ptr< vaActor> acSdftex = std::shared_ptr< vaActor>(new  vaActor());

    acSdftex->SetContext(m.GetOutput()); //sets context and initialize acceleration properties

                                       //TODO: overwrite with mapper function that returns it's instance
                                       //ac.SetGeometry(mat, pl.GetOutput());
    acSdftex->AddMapper(maptex);// .GetOutput());
    acSdftex->Update();

    //-----------

    vaColorScheme sc;
    sc.SetContext(m.GetOutput());
    sc.SetIdType(); //map  by atom types
    optix::float2 r = mol.GetTypeRange();
    sc.SetRange(optix::make_float2(0, r.y));

    //fill colors for CPK scheme for selected range of atoms
    for (int i = 0; i < int(r.y); i++) {
        sc.AddColor(CPK_GetColorById(i)); //maps atoms ids to colors according to CPK scheme
    }
    sc.Update(); /*<generates optixBuffer and callable program*/

                 /*
                 *Creates optical matrial
                 */
                 /*vaMolVolume2 mSdf;
                 mSdf.SetContext(m.GetOutput());
                 mSdf.SetSDFProg(mol.GetCallableProg());
                 mSdf.SetColorScheme(sc.GetOutput());
                 mSdf.Update();*/

    vaComplexVolumeMaterial mSdf;
    mSdf.SetContext(m.GetOutput());
    //mSdf.SetSDFProg(sdf.GetCallableProg());
    mSdf.SetSDFProg(mol.GetCallableProg());

    mSdf.SetColorScheme(sc.GetOutput());
    mSdf.Update();

    /*
    Creates mapper
    */
    std::shared_ptr<vaMapper> map3 = std::shared_ptr<vaMapper>(new vaMapper());
    map3->SetContext(m.GetOutput());
    //map3->SetInput(mol.GetOutput());
    map3->SetDescInput(mol.GetOutputDesc());
    map3->AddMaterial(mSdf.GetOutput(), mSdf.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    map3->Update();

    /*
    Creates actor
    */
    std::shared_ptr< vaActor> acSdfMol = std::shared_ptr< vaActor>(new  vaActor());

    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    //acSdfMol->AddMapper(map4);
    acSdfMol->Update();

    //RTvisibilitymask mask;
    //rtGeometryGroupGetVisibilityMask(acSdfMol->GetOutput(), &mask)
    //RTvisibilitymask mask = acSdfMol->GetOutput()->getVisibilityMask();
    //std::cout << " VIS Mask = " << mask << std::endl;
    //acSdfMol->GetOutput()->setVisibilityMask(2);

    ren->AddActor(acSdfMol);
    ren->AddActor(acSdftex);
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);

    //
    actorSdf.push_back(acSdftex);
    mappers.push_back(maptex);

    //asign prog
    ren->SetMissProgSDFProg(sdf.GetCallableProg());
    ren->SetMissProgSDFProg2(mol.GetCallableProg());
}

void SceneManager::createAuditoryMoleculeScene2()
{
    /*Read molecule data from XYZ file.
    For info on XYZ file format:
    //http://wiki.jmol.org/index.php/File_formats/Formats/XYZ
    */
    sdfReader read;
    read.Setfile("es.sdf");// "water2.sdf");
    read.Update();

    sdfReader read2;
    read2.Setfile("cs.sdf");// "water2.sdf");
    read2.Update();
    //http://www1.lsbu.ac.uk/water/escs.html

    /*Molecule CPK representation as sdf primitive
    For info on representation:
    https://en.wikipedia.org/wiki/Space-filling_model
    */
    sdfBallSticksMol mol;
    mol.SetContext(m.GetOutput());
    mol.SetNumFrames(3);
    mol.SetCenter(read.GetOutput1(), 1); /*<Sets atoms centers*/
    //mol.SimulateDynamic(6, read.GetOutput1()); - works, but we will try collapse of cluster
    mol.SetCenter(read2.GetOutput1(), 2);
    mol.SetCenter(read.GetOutput1(), 3);
    mol.SetRadius(read.GetOutput2()); /*<Sets atoms radii specified for element type,
                                      for info: https://en.wikipedia.org/wiki/Atomic_radius*/
    mol.SetType(read.GetOutput3()); /*<Sets atom type*/
   //after all to correctly fix number
    mol.SetBonds(read.GetOutput4()); /*<Sets atoms centers*/

    mol.SetMaterialType(0);
    mol.Update();

    optixSDFSphere s1, s2;
    s1.SetContext(m.GetOutput());
    s2.SetContext(m.GetOutput());
    s1.SetCenter1(optix::make_float3(0, -2, 0));
    s2.SetCenter1(optix::make_float3(0, 2, 0));
    s1.SetRadius1(optix::make_float3(0.4));
    s2.SetRadius1(optix::make_float3(0.4));
    s1.Update();
    s2.Update();

    optixSDFBox sdf;
    sdf.SetContext(m.GetOutput());
    sdf.SetCenter1(optix::make_float3(1.5));
    sdf.SetDims(optix::make_float3(0.3));
    sdf.Update();

    SDFBlendUnionOp opB1;
    opB1.SetContext(m.GetOutput());
    opB1.AddOpperand1(&s1);
    opB1.AddOpperand2(&s2);
    opB1.SetKoeff(0.3);
    opB1.Update();

    vaBasicMaterial mSdfS;
    mSdfS.SetContext(m.GetOutput());
    /*<Sets callable program for color mapping
    automatically switch to mapping mode data to color,
    not default color*/

    //mSdfS.SetColorScheme(opB1.GetOutput());
    mSdfS.Update();

    /*SDFVolumeMaterial mVSdf;
    mVSdf.SetContext(m.GetOutput());
    mVSdf.Update();
    */

    vaColorScheme sc;
    sc.SetContext(m.GetOutput());
    sc.SetIdType(); //map  by atom types
    optix::float2 r = mol.GetTypeRange();
    sc.SetRange(optix::make_float2(0, r.y));

    //fill colors for CPK scheme for selected range of atoms
    for (int i = 0; i < int(r.y); i++) {
        sc.AddColor(CPK_GetColorById(i)); //maps atoms ids to colors according to CPK scheme
    }
    sc.Update(); /*<generates optixBuffer and callable program*/

                 /*
                 *Creates optical matrial
                 */
    vaBasicMaterial mSdf;
    mSdf.SetContext(m.GetOutput());
    /*<Sets callable program for color mapping
    automatically switch to mapping mode data to color,
    not default color*/

    mSdf.SetColorScheme(sc.GetOutput());
    mSdf.Update();

    /*
    Creates auditory material
    */
    SDFAudioVolumeMaterial mVSdf2;
    mVSdf2.SetContext(m.GetOutput());
    mVSdf2.Update();

    /*
    Creates mapper
    */
    std::shared_ptr<vaMapper> map3 = std::shared_ptr<vaMapper>(new vaMapper());
    map3->SetContext(m.GetOutput());
    //map3->SetInput(mol.GetOutput());
    map3->SetDescInput(mol.GetOutputDesc());
    map3->AddMaterial(mSdf.GetOutput(), mSdf.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    std::shared_ptr<vaMapper> map4 = std::shared_ptr<vaMapper>(new vaMapper());
    map4->SetContext(m.GetOutput());
    //map3->SetInput(mol.GetOutput());
    map4->SetInput(s1.GetOutput());
    map4->AddMaterial(mSdfS.GetOutput(), mSdfS.GetType()); /*<Sets optical object properties*/
    //map3->SetScalarModeOff();
    map4->Update();
    /*
    Creates actor
    */
    std::shared_ptr< vaActor> acSdfMol = std::shared_ptr< vaActor>(new  vaActor());

    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    //acSdfMol->AddMapper(map4);
    acSdfMol->Update();

    //RTvisibilitymask mask;
    //rtGeometryGroupGetVisibilityMask(acSdfMol->GetOutput(), &mask)
    //RTvisibilitymask mask = acSdfMol->GetOutput()->getVisibilityMask();
    //std::cout << " VIS Mask = " << mask << std::endl;
    //acSdfMol->GetOutput()->setVisibilityMask(2);

    ren->AddActor(acSdfMol);
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);
}
//----------------------------
void SceneManager::createAuditoryMoleculeSceneMolDynam()
{
    /*Read molecule data from XYZ file.
    For info on XYZ file format:
    http://wiki.jmol.org/index.php/File_formats/Formats/XYZ
    */
    xyzReader read;
    read.Setfile("diffusion.xyz");

    read.Update();
    read.Grow(2); //increase speed and type

     /*Molecule CPK representation as sdf primitive
     For info on representation:
     https://en.wikipedia.org/wiki/Space-filling_model
     */
    sdfCPKMol mol;
    //sdfCPKDynMol mol;
    mol.SetContext(m.GetOutput());
    mol.SimulateDynamic(3, read.GetOutput1());//SetCenter(read.GetOutput1()); /*<Sets atoms centers*/
    mol.SetRadius(read.GetOutput2()); /*<Sets atoms radii specified for element type,
                                      for info: https://en.wikipedia.org/wiki/Atomic_radius*/
    mol.SetType(read.GetOutput3()); /*<Sets atom type*/

    mol.SetMaterialType(0);
    mol.Update();

    //SDFSubtractionOp opBlend;
    //opBlend.SetContext(m.GetOutput());
    //opBlend.AddOpperand1(&mol);
    //opBlend.AddOpperand2(m_widget->GetHandle());
    // opBlend.SetKoeff(0.3);
    // opBlend.Update();

    /*SDFVolumeMaterial mVSdf;
    mVSdf.SetContext(m.GetOutput());
    mVSdf.Update();
    */

    vaColorScheme sc;
    sc.SetContext(m.GetOutput());
    sc.SetIdType(); //map  by atom types
    optix::float2 r = mol.GetTypeRange();
    sc.SetRange(optix::make_float2(0, r.y));

    //fill colors for CPK scheme for selected range of atoms
    for (int i = 0; i < int(r.y); i++) {
        sc.AddColor(CPK_GetColorById(i)); //maps atoms ids to colors according to CPK scheme
    }
    sc.Update(); /*<generates optixBuffer and callable program*/

                 /*
                 *Creates optical matrial
                 */
    vaBasicMaterial mSdf;
    mSdf.SetContext(m.GetOutput());
    /*<Sets callable program for color mapping
    automatically switch to mapping mode data to color,
    not default color*/

    mSdf.SetColorScheme(sc.GetOutput());
    mSdf.Update();

    /*
    Creates auditory material
*/
    SDFAudioVolumeMaterial mVSdf2;
    mVSdf2.SetContext(m.GetOutput());
    mVSdf2.Update();

    /*
    Creates mapper
    */
    std::shared_ptr<vaMapper> map3 = std::shared_ptr<vaMapper>(new vaMapper());
    map3->SetContext(m.GetOutput());
    map3->SetDescInput(mol.GetOutputDesc());//SetInput(mol.GetOutput());
    map3->AddMaterial(mSdf.GetOutput(), mSdf.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    /*
    Creates actor
    */
    std::shared_ptr< vaActor> acSdfMol = std::shared_ptr< vaActor>(new  vaActor());

    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    ren->AddActor(acSdfMol);
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);
}
void SceneManager::SetXYZData(MPoints p, MTypes t)
{
    int size = p.cols()*p.rows();
    int j = 0;
    for (int i = 0; i < size; i += 4) {
        optix::float3 l = optix::make_float3(p.data()[i], p.data()[i + 1], p.data()[i + 2]);
        center.push_back(l);
        rad.push_back(p.data()[i + 3]);
        type.push_back(t.data()[j]);
        j++;
    }
    //  SetCenter(center);
    //   SetRadius(rad);
    //   SetType(type);

   // type.reserve(size);
   // memcpy(type.data(), t.data(), sizeof(int) * (size_t)(size));
    SetExample(10);
}
void SceneManager::Example10()
{
    /*Molecule CPK representation as sdf primitive
    For info on representation:
    https://en.wikipedia.org/wiki/Space-filling_model
    */

    sdfHMicro mol;//sdfHMicrostructure mol;// // sdfHeterogeneous mol;//sdfCPKMol mol; //
    mol.SetContext(m.GetOutput());
    mol.SetCenter(center); /*<Sets atoms centers*/
    mol.SetRadius(rad); /*<Sets atoms radii specified for element type,
                                      for info: https://en.wikipedia.org/wiki/Atomic_radius*/
    mol.SetType(type); /*<Sets atom type*/

    mol.SetMaterialType(0);
    mol.Update();

    std::cout << type.size() << " elements" << std::endl;
    std::cout << center[0] << " elements" << std::endl;
    std::cout << type[type.size() - 1] << " types " << type[1] << std::endl;

    BasicLight l1;
    l1.color = optix::make_float3(1.0);
    l1.pos = optix::make_float3(10.0);

    BasicLight l2;
    l2.color = optix::make_float3(1.0);
    l2.pos = optix::make_float3(0, 0, 10.0);

    BasicLight l3;
    l3.color = optix::make_float3(1.0);
    l3.pos = optix::make_float3(-10.0, 0, -1.0);

    vaVolumeSDFHetero texMaterial;

    texMaterial.SetContext(m.GetOutput());
    /*Get heterogeneous object type to compile the right program*/
    texMaterial.SetHeteroObjType(mol.GetPrimType());
    texMaterial.SetPostprocess(1);
    texMaterial.AddLight(&l1);
    texMaterial.AddLight(&l2);
    //texMaterial.AddLight(&l3);
    texMaterial.SetSDFProg(mol.GetCallableProg()); /*<Gets sdf primitive optix callable program reference*/
    texMaterial.SetType(vaEAVolume::MaterialType::TRANSP); /*<Volume rendering mode*/
                                                           //texMaterial.SetTexture(readR1.GetTexture());
                                                           //texMaterial.SetTexture(readR2.GetTexture());
    texMaterial.Update();

    /*
    Creates mapper
    */
    std::shared_ptr<vaMapper> map3 = std::shared_ptr<vaMapper>(new vaMapper());
    map3->SetContext(m.GetOutput());
    map3->SetInput(mol.GetOutput());
    map3->AddMaterial(texMaterial.GetOutput(), texMaterial.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    // map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    /*
    Creates actor
    */
    std::shared_ptr<vaActor> acSdfMol = std::shared_ptr<vaActor>(new vaActor());
    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    ren->AddActor(acSdfMol);
    ren->SetPostProcessMaterial(texMaterial.GetEvalProg(), texMaterial.GetColorProg());
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);
}
void SceneManager::Example0()
{
    /*Read molecule data from XYZ file.
    For info on XYZ file format:
    http://wiki.jmol.org/index.php/File_formats/Formats/XYZ
    */
    xyzReader read;
    read.Setfile("slice.xyz");
    read.Update();

    /*Molecule CPK representation as sdf primitive
    For info on representation:
    https://en.wikipedia.org/wiki/Space-filling_model
    */
    sdfHMicro mol;//sdfHMicrostructure mol;// // sdfHeterogeneous mol;//sdfCPKMol mol; //
    mol.SetContext(m.GetOutput());
    mol.SetCenter(read.GetOutput1()); /*<Sets atoms centers*/
    mol.SetRadius(read.GetOutput2()); /*<Sets atoms radii specified for element type,
                                      for info: https://en.wikipedia.org/wiki/Atomic_radius*/
    mol.SetType(read.GetOutput3()); /*<Sets atom type*/

    mol.SetMaterialType(0);
    mol.Update();

    BasicLight l1;
    l1.color = optix::make_float3(1.0);
    l1.pos = optix::make_float3(10.0);

    BasicLight l2;
    l2.color = optix::make_float3(1.0);
    l2.pos = optix::make_float3(0, 0, 10.0);

    BasicLight l3;
    l3.color = optix::make_float3(1.0);
    l3.pos = optix::make_float3(-10.0, 0, -1.0);

    vaVolumeSDFHetero texMaterial;

    texMaterial.SetContext(m.GetOutput());
    /*Get heterogeneous object type to compile the right program*/
    texMaterial.SetHeteroObjType(mol.GetPrimType());
    texMaterial.SetPostprocess(1);
    texMaterial.AddLight(&l1);
    texMaterial.AddLight(&l2);
    //texMaterial.AddLight(&l3);
    texMaterial.SetSDFProg(mol.GetCallableProg()); /*<Gets sdf primitive optix callable program reference*/
    texMaterial.SetType(vaEAVolume::MaterialType::TRANSP); /*<Volume rendering mode*/
                                                           //texMaterial.SetTexture(readR1.GetTexture());
                                                           //texMaterial.SetTexture(readR2.GetTexture());
    texMaterial.Update();

    /*
    Creates mapper
    */
    std::shared_ptr<vaMapper> map3 = std::shared_ptr<vaMapper>(new vaMapper());
    map3->SetContext(m.GetOutput());
    map3->SetInput(mol.GetOutput());
    map3->AddMaterial(texMaterial.GetOutput(), texMaterial.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    // map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    /*
    Creates actor
    */
    std::shared_ptr<vaActor> acSdfMol = std::shared_ptr<vaActor>(new vaActor());
    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    ren->AddActor(acSdfMol);
    ren->SetPostProcessMaterial(texMaterial.GetEvalProg(), texMaterial.GetColorProg());
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);
}
void SceneManager::Example2()
{
    /*Read molecule data from XYZ file.
    For info on XYZ file format:
    http://wiki.jmol.org/index.php/File_formats/Formats/XYZ
    */
    xyzReader read;
    read.Setfile("slice.xyz");
    read.Update();

    /*Molecule CPK representation as sdf primitive
    For info on representation:
    https://en.wikipedia.org/wiki/Space-filling_model
    */
    sdfHMicrostructure mol;//sdfHMicrostructure mol;// // sdfHeterogeneous mol;//sdfCPKMol mol; //
    mol.SetContext(m.GetOutput());
    mol.SetCenter(read.GetOutput1()); /*<Sets atoms centers*/
    mol.SetRadius(read.GetOutput2()); /*<Sets atoms radii specified for element type,
                                      for info: https://en.wikipedia.org/wiki/Atomic_radius*/
    mol.SetType(read.GetOutput3()); /*<Sets atom type*/

    mol.SetMaterialType(0);
    mol.Update();

    BasicLight l1;
    l1.color = optix::make_float3(1.0);
    l1.pos = optix::make_float3(10.0);

    BasicLight l2;
    l2.color = optix::make_float3(1.0);
    l2.pos = optix::make_float3(0, 0, 10.0);

    BasicLight l3;
    l3.color = optix::make_float3(1.0);
    l3.pos = optix::make_float3(-10.0, 0, -1.0);

    vaVolumeSDFHetero texMaterial;

    texMaterial.SetContext(m.GetOutput());
    /*Get heterogeneous object type to compile the right program*/
    texMaterial.SetHeteroObjType(mol.GetPrimType());
    texMaterial.AddLight(&l1);
    texMaterial.AddLight(&l2);
    //texMaterial.AddLight(&l3);
    texMaterial.SetSDFProg(mol.GetCallableProg()); /*<Gets sdf primitive optix callable program reference*/
    texMaterial.SetType(vaEAVolume::MaterialType::TRANSP); /*<Volume rendering mode*/
                                                           //texMaterial.SetTexture(readR1.GetTexture());
                                                           //texMaterial.SetTexture(readR2.GetTexture());
    texMaterial.Update();

    /*
    Creates mapper
    */
    std::shared_ptr<vaMapper> map3 = std::shared_ptr<vaMapper>(new vaMapper());
    map3->SetContext(m.GetOutput());
    map3->SetInput(mol.GetOutput());
    map3->AddMaterial(texMaterial.GetOutput(), texMaterial.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    // map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    /*
    Creates actor
    */
    std::shared_ptr<vaActor> acSdfMol = std::shared_ptr<vaActor>(new vaActor());
    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    //RTvisibilitymask mask;
    //rtGeometryGroupGetVisibilityMask(acSdfMol->GetOutput(), &mask)
    //RTvisibilitymask mask = acSdfMol->GetOutput()->getVisibilityMask();
    //std::cout << " VIS Mask = " << mask << std::endl;
    //acSdfMol->GetOutput()->setVisibilityMask(2);

    ren->AddActor(acSdfMol);
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);
}
// Scene testing all materials on a single geometry instanced via transforms and sharing one acceleration structure.
void SceneManager::createScene()
{
    ren->SetBackground(optix::make_float3(0, 0, 0));
    //initMaterials();
    std::cout << "Start" << std::endl;
    try
    {
        ///Look to

        m.GetOutput()->setMaxCallableProgramDepth(80);
        switch (m_example)
        {
        case 0:
            Example0();
            break;
        case 1:
            Example1();
            break;
        case 2:
            Example2();
            break;
        case 3:
            Example31(); //createAuditoryMoleculeSceneMol2();//
            break;
        case 4:
            ExampleTetra();
            break;
        case 5:
            createAuditoryMoleculeSceneMolDynam();// a lot of spheres dynamic ray-casting
            //not volume. Just a test for comparison with volume
            break;
        case 6:
            Example6();
            break;
        case 7:
            createDynamicHeterogeneousObjectScene();
            break;
        case 10:
            Example10();
            break;
        }
        //
        //experiments with heterogeneous objects
        //and interactive visual-auditory ray-casting
        //

        //createMolSolventScene();

        /* 1. sdfHetrogeneous object test
        Example mostly is used for FPS comparison to
        more optimised sdfHeterogeneous0D, sdfHeterogeneous1D and etc
        */

        //createFrepScene(); //1)

        //createMicrostructureScene();
        /*Test of example2*/
        //Example1();
        //------------------
        //TODO:createAuditoryMoleculeSceneDynamic
        //start writing optical and auditory models through materials for
        //ray-casting

        //--------------------------------------------
        //experiment with molecules blending and dynamic molecules rendering

        //The last two for microstructures
        //createMicrostuctureMOlScene(); //1) working overlap volumes

        //createCrystalSceneMol2(); //-1) working

        //createAuditoryMoleculeScene2(); //-1) balls and sticks non-transparent
                                        //for latest sweeping and etc
       // createAuditoryMoleculeSceneMol2(); //1) working but slow multi-scale
       // createAuditoryMoleculeSceneMolDynam();
    }
    catch (optix::Exception& e)
    {
        std::cerr << e.getErrorString() << std::endl;
    }
}

/*
SceneManager2

*/

void SceneManager2::Example2()
{
    /*Read molecule data from XYZ file.
    For info on XYZ file format:
    http://wiki.jmol.org/index.php/File_formats/Formats/XYZ
    */
    xyzReader read;
    read.Setfile("slice.xyz");
    read.Update();

    /*Molecule CPK representation as sdf primitive
    For info on representation:
    https://en.wikipedia.org/wiki/Space-filling_model
    */
    sdfHMicrostructure mol;//sdfHMicrostructure mol;// // sdfHeterogeneous mol;//sdfCPKMol mol; //
    mol.SetContext(m.GetOutput());
    mol.SetCenter(read.GetOutput1()); /*<Sets atoms centers*/
    mol.SetRadius(read.GetOutput2()); /*<Sets atoms radii specified for element type,
                                      for info: https://en.wikipedia.org/wiki/Atomic_radius*/
    mol.SetType(read.GetOutput3()); /*<Sets atom type*/

    mol.SetMaterialType(0);
    mol.Update();

    BasicLight l1;
    l1.color = optix::make_float3(1.0);
    l1.pos = optix::make_float3(10.0);

    BasicLight l2;
    l2.color = optix::make_float3(1.0);
    l2.pos = optix::make_float3(0, 0, 10.0);

    BasicLight l3;
    l3.color = optix::make_float3(1.0);
    l3.pos = optix::make_float3(-10.0, 0, -1.0);

    vaVolumeSDFHetero texMaterial;

    texMaterial.SetContext(m.GetOutput());
    /*Get heterogeneous object type to compile the right program*/
    texMaterial.SetHeteroObjType(mol.GetPrimType());
    texMaterial.AddLight(&l1);
    texMaterial.AddLight(&l2);
    //texMaterial.AddLight(&l3);
    texMaterial.SetSDFProg(mol.GetCallableProg()); /*<Gets sdf primitive optix callable program reference*/
    texMaterial.SetType(vaEAVolume::MaterialType::TRANSP); /*<Volume rendering mode*/
                                                           //texMaterial.SetTexture(readR1.GetTexture());
                                                           //texMaterial.SetTexture(readR2.GetTexture());
    texMaterial.Update();

    /*
    Creates mapper
    */
    std::shared_ptr<vaMapper> map3 = std::shared_ptr<vaMapper>(new vaMapper());
    map3->SetContext(m.GetOutput());
    map3->SetInput(mol.GetOutput());
    map3->AddMaterial(texMaterial.GetOutput(), texMaterial.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    // map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    /*
    Creates actor
    */
    std::shared_ptr<vaActor> acSdfMol = std::shared_ptr<vaActor>(new vaActor());
    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    //RTvisibilitymask mask;
    //rtGeometryGroupGetVisibilityMask(acSdfMol->GetOutput(), &mask)
    //RTvisibilitymask mask = acSdfMol->GetOutput()->getVisibilityMask();
    //std::cout << " VIS Mask = " << mask << std::endl;
    //acSdfMol->GetOutput()->setVisibilityMask(2);

    ren->AddActor(acSdfMol);
    //store
//    actorSdf.push_back(acSdfMol);
 //   mappers.push_back(map3);
}
// Scene testing all materials on a single geometry instanced via transforms and sharing one acceleration structure.
void SceneManager2::createScene()
{
    ren->SetBackground(optix::make_float3(0, 0, 0));
    //initMaterials();
    std::cout << "Start" << std::endl;
    try
    {
        ///Look to

        m.GetOutput()->setMaxCallableProgramDepth(80);
        Example2();
    }
    catch (optix::Exception& e)
    {
        std::cerr << e.getErrorString() << std::endl;
    }
}

SceneManager2::SceneManager2()

{
    int width = 1512;
    int height = 1512;
    m_pinholeCamera = std::shared_ptr<PinholeCamera>(new PinholeCamera());
    m_example = 0;
    m.Update();//creates context

    ren = std::shared_ptr<vaRenderer>(new vaRenderer()); //vaAdancedRenderer
    ren->SetValid(m.GetValid());
    ren->SetContext(m.GetOutput());

    m_widget = std::shared_ptr<vaBaseWidget>(new vaRayCastBaseWidget());
    m_widget->SetContext(m.GetOutput());

    ren->SetOpticalDims(width, height);
    ren->SetAuditoryDims(5, 5);
    ren->SetCamera(m_pinholeCamera);

    //TODO: add separately

    //

    //set not dynamic
    ren->SetDynamic(true);
    ren->SetAuditory(true);

    //overwrites auditory ray-generation
    //This should be done before interactor SetUp() procedure
    // that does the final setups of all renderer stuff,
    // like setting RayGenerationProgam for visual and auditory context ray tracing
    if (m_widget->isRayCast()) {
        ren->SetAuditoryRayGenerationFromWidget("widget_ray_cast", dynamic_cast<vaRayCastBaseWidget*>(m_widget.get())->GetRayCastProg());
        //auditoryMapper* m = dynamic_cast<auditoryMapper*>(new auditoryMapperPlucked());

        ren->SetAuditoryMapModel(new auditoryMapperPlucked());
    }
}

void SceneManager2::Init()
{
    //add geometry handle
    m_widget->CreateGeometryHandle();
    ren->SetWidget(m_widget);
    try
    {
        m_timer.restart();
        const double timeInit = m_timer.getTime();

        std::cout << "createScene()" << std::endl;
        createScene();
        const double timeScene = m_timer.getTime();

        std::cout << "m_context->validate()" << std::endl;
        //ren->Update();
        //m_context->validate();
        const double timeValidate = m_timer.getTime();

        std::cout << "m_context->launch()" << std::endl;
        // m_context->launch(0, 0, 0); // Dummy launch to build everything (entrypoint, width, height)
        const double timeLaunch = m_timer.getTime();

        std::cout << "initScene(): " << timeLaunch - timeInit << " seconds overall" << std::endl;
        std::cout << "{" << std::endl;
        std::cout << "  createScene() = " << timeScene - timeInit << " seconds" << std::endl;
        std::cout << "  validate()    = " << timeValidate - timeScene << " seconds" << std::endl;
        std::cout << "  launch()      = " << timeLaunch - timeValidate << " seconds" << std::endl;
        std::cout << "}" << std::endl;
    }
    catch (optix::Exception& e)
    {
        std::cerr << e.getErrorString() << std::endl;
    }
}
SceneManager2::~SceneManager2()
{
    // DAR FIXME Do any other destruction here.
    if (ren->IsValid())
    {
        m.GetOutput()->destroy();
    }

    //    mappers.clear();
    //    actorSdf.clear();
        /*

        for (int i = 0; i < mappers.size(); i++)
        {
        if (mappers[i] != nullptr)
        delete mappers[i];
        }

        for (int i = 0; i < actorSdf.size(); i++)
        {
        if (actorSdf[i] != nullptr)
        delete actorSdf[i];
        }

        for (int i = 0; i < actorTri.size(); i++)
        {
        if (actorTri[i] != nullptr)
        delete actorTri[i];
        }
        */
        //destroy renderer
        //    delete ren;
}

bool SceneManager2::isValid()
{
    return ren->IsValid();
}