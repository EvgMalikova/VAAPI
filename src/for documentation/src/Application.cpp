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
    }
    return optix::make_float3(1, 1, 1);
}
// DAR Only for sutil::samplesPTXDir() and sutil::writeBufferToFile()
#include <sutil.h>
SceneManager::SceneManager()

{
    int width = 512;
    int height = 512;
    m_pinholeCamera = std::shared_ptr<PinholeCamera>(new PinholeCamera());

    m.Update();//creates context

    ren.SetValid(m.GetValid());
    ren.SetContext(m.GetOutput());

    m_widget = std::shared_ptr<vaBaseWidget>(new vaRayCastBaseWidget());
    m_widget->SetContext(m.GetOutput());

    ren.SetOpticalDims(width, height);
    ren.SetAuditoryDims(5, 5);
    ren.SetCamera(m_pinholeCamera.get());

    //TODO: add separately

    //

    //set not dynamic
    ren.SetDynamic(true);
    ren.SetAuditory(true);

    //overwrites auditory ray-generation
    //This should be done before interactor SetUp() procedure
    // that does the final setups of all renderer stuff,
    // like setting RayGenerationProgam for visual and auditory context ray tracing
    if (m_widget->isRayCast()) {
        ren.SetAuditoryRayGenerationFromWidget("widget_ray_cast", dynamic_cast<vaRayCastBaseWidget*>(m_widget.get())->GetRayCastProg());
        //auditoryMapper* m = dynamic_cast<auditoryMapper*>(new auditoryMapperPlucked());

        ren.SetAuditoryMapModel(new auditoryMapperPlucked());
    }
}

SceneManager::SceneManager(const int width,
    const int height,
    const unsigned int devices,
    const unsigned int stackSize,
    const bool interop)

{
    m_pinholeCamera = std::shared_ptr<PinholeCamera>(new PinholeCamera());

    m.Update();//creates context

    ren.SetValid(m.GetValid());
    ren.SetContext(m.GetOutput());

    ren.SetOpticalDims(width, height);
    ren.SetCamera(m_pinholeCamera.get());

    //set not dynamic
    ren.SetDynamic(true);
    ren.SetAuditory(true);
}
void SceneManager::Init()
{
    //add geometry handle
    m_widget->CreateGeometryHandle();
    ren.SetWidget(m_widget.get());
    try
    {
        m_timer.restart();
        const double timeInit = m_timer.getTime();

        std::cout << "createScene()" << std::endl;
        createScene();
        const double timeScene = m_timer.getTime();

        std::cout << "m_context->validate()" << std::endl;
        //ren.Update();
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
    if (ren.IsValid())
    {
        m.GetOutput()->destroy();
    }

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
}

bool SceneManager::isValid()
{
    return ren.IsValid();
}
void SceneManager::createTriangulatedScene()
{
    optixPlane pl;
    pl.SetContext(m.GetOutput());
    pl.SettessU(1);
    pl.SettessV(1);
    pl.SetupAxis(1);
    pl.Update();

    Lambertian l;
    l.SetContext(m.GetOutput());
    l.Update();

    vaMapper* map1 = new vaMapper();
    map1->SetContext(m.GetOutput());
    map1->SetInput(pl.GetOutput());
    map1->AddMaterial(l.GetOutput(), l.GetType());
    map1->Update();

    optixTRIActor* ac = new optixTRIActor();
    ac->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    ac->AddMapper(map1);
    ac->Update();

    ren.AddActor(ac);

    //store
    actorTri.push_back(ac);
    mappers.push_back(map1);
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
    SDFRoundingOp round[nums];
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
    }

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

    vaMapper* map21 = new vaMapper();
    map21->SetContext(m.GetOutput());
    map21->SetInput(sdf.GetOutput());
    map21->AddMaterial(mSdf.GetOutput(), mSdf.GetType());
    map21->Update();

    vaActor* acSdf1 = new vaActor();
    acSdf1->SetContext(m.GetOutput()); //sets context and initialize acceleration properties

                                      //TODO: overwrite with mapper function that returns it's instance
                                      //ac.SetGeometry(mat, pl.GetOutput());
    acSdf1->AddMapper(map21);// .GetOutput());
    acSdf1->Update();

    ren.AddActor(acSdf1);

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

    SDFSubtractionOp opBlend;
    opBlend.SetContext(m.GetOutput());
    opBlend.AddOpperand1(&tex);
    opBlend.AddOpperand2(m_widget->GetHandle());
    // opBlend.SetKoeff(0.3);
    opBlend.Update();

    /* Mapper, simillar to vtkMapper*/
    vaMapper* map2 = new vaMapper();
    map2->SetContext(m.GetOutput());
    map2->SetDescInput(tex.GetOutputDesc());
    map2->AddMaterial(texMaterial.GetOutput(), texMaterial.GetType());
    map2->Update();

    /*Actor, simillar to vtkActor*/
    vaActor* acSdf = new vaActor();
    acSdf->SetContext(m.GetOutput());
    acSdf->AddMapper(map2);
    acSdf->Update();

    /*Add actor to the rendere*/
    ren.AddActor(acSdf);

    //store
    actorSdf.push_back(acSdf);
    mappers.push_back(map2);
}

void SceneManager::createMolSolventScene()
{
    //ren.SetAudioBuffer()
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
    tex.SetTexture(readSDFTex2.GetTexture(), readSDFTex2.GetParam());

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
    //  vaMapper* map3 = new vaMapper();
    //  map3->SetContext(m.GetOutput());
    //  map3->SetDescInput(mol.GetOutputDesc());
    //  map3->AddMaterial(mSdf.GetOutput(), mSdf.GetType()); /*<Sets optical object properties*/
    //  map3->SetScalarModeOn();
      // map2->Update();

      /* Mapper, simillar to vtkMapper*/
    vaMapper* map2 = new vaMapper();
    map2->SetContext(m.GetOutput());
    map2->SetDescInput(tex.GetOutputDesc());
    map2->AddMaterial(mSdf.GetOutput(), mSdf.GetType());
    map2->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/

    map2->Update();

    /*Actor, simillar to vtkActor*/
    vaActor* acSdf = new vaActor();
    acSdf->SetContext(m.GetOutput());
    acSdf->AddMapper(map2);
    acSdf->Update();

    /*Add actor to the rendere*/
    ren.AddActor(acSdf);

    //store
    actorSdf.push_back(acSdf);
    mappers.push_back(map2);
}

#define IGN_VISIBILITY_SELECTABLE      0x00000002
void SceneManager::createAuditoryMoleculeSceneDynamic()
{
    /*Read molecule data from XYZ file.
    For info on XYZ file format:
    //http://wiki.jmol.org/index.php/File_formats/Formats/XYZ
    */
    sdfReader read;
    read.Setfile("c2.sdf");
    read.Update();

    sdfReader read2;
    read2.Setfile("c1.sdf");
    read2.Update();
    /*Molecule CPK representation as sdf primitive
    For info on representation:
    https://en.wikipedia.org/wiki/Space-filling_model
    */
    sdfCPKDynMol mol;
    mol.SetContext(m.GetOutput());
    mol.SetCenter(read.GetOutput1()); /*<Sets atoms centers*/
    mol.SetCenter2(read2.GetOutput1()); /*<Sets atoms centers*/

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
    vaMapper* map3 = new vaMapper();
    map3->SetContext(m.GetOutput());
    //map3->SetInput(mol.GetOutput());
    map3->SetDescInput(mol.GetOutputDesc());
    map3->AddMaterial(mSdf.GetOutput(), mSdf.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    /*
    Creates actor
    */
    vaActor* acSdfMol = new vaActor();
    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    //RTvisibilitymask mask;
    //rtGeometryGroupGetVisibilityMask(acSdfMol->GetOutput(), &mask)
    //RTvisibilitymask mask = acSdfMol->GetOutput()->getVisibilityMask();
    //std::cout << " VIS Mask = " << mask << std::endl;
    //acSdfMol->GetOutput()->setVisibilityMask(2);

    ren.AddActor(acSdfMol);
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);
}
void SceneManager::createAuditoryMoleculeScene2()
{
    /*Read molecule data from XYZ file.
    For info on XYZ file format:
    //http://wiki.jmol.org/index.php/File_formats/Formats/XYZ
    */
    sdfReader read;
    read.Setfile("water2.sdf");
    read.Update();

    /*Molecule CPK representation as sdf primitive
    For info on representation:
    https://en.wikipedia.org/wiki/Space-filling_model
    */
    sdfBallSticksMol mol;
    mol.SetContext(m.GetOutput());
    mol.SetCenter(read.GetOutput1()); /*<Sets atoms centers*/
    //mol.SimulateDynamic(3, read.GetOutput1());

    mol.SetBonds(read.GetOutput4()); /*<Sets atoms centers*/

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
    vaMapper* map3 = new vaMapper();
    map3->SetContext(m.GetOutput());
    //map3->SetInput(mol.GetOutput());
    map3->SetDescInput(mol.GetOutputDesc());
    map3->AddMaterial(mSdf.GetOutput(), mSdf.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    /*
    Creates actor
    */
    vaActor* acSdfMol = new vaActor();
    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    //RTvisibilitymask mask;
    //rtGeometryGroupGetVisibilityMask(acSdfMol->GetOutput(), &mask)
    //RTvisibilitymask mask = acSdfMol->GetOutput()->getVisibilityMask();
    //std::cout << " VIS Mask = " << mask << std::endl;
    //acSdfMol->GetOutput()->setVisibilityMask(2);

    ren.AddActor(acSdfMol);
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

    /*Molecule CPK representation as sdf primitive
    For info on representation:
    https://en.wikipedia.org/wiki/Space-filling_model
    */
    sdfCPKMol mol;
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
    vaMapper* map3 = new vaMapper();
    map3->SetContext(m.GetOutput());
    map3->SetDescInput(mol.GetOutputDesc());//SetInput(mol.GetOutput());
    map3->AddMaterial(mSdf.GetOutput(), mSdf.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    /*
    Creates actor
    */
    vaActor* acSdfMol = new vaActor();
    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    //RTvisibilitymask mask;
    //rtGeometryGroupGetVisibilityMask(acSdfMol->GetOutput(), &mask)
    //RTvisibilitymask mask = acSdfMol->GetOutput()->getVisibilityMask();
    //std::cout << " VIS Mask = " << mask << std::endl;
    //acSdfMol->GetOutput()->setVisibilityMask(2);

    ren.AddActor(acSdfMol);
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);
}

void SceneManager::createAuditoryMoleculeScene()
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
    sdfCPKMol mol;
    mol.SetContext(m.GetOutput());
    mol.SetCenter(read.GetOutput1()); /*<Sets atoms centers*/
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
    vaMapper* map3 = new vaMapper();
    map3->SetContext(m.GetOutput());
    map3->SetInput(mol.GetOutput());
    map3->AddMaterial(mSdf.GetOutput(), mSdf.GetType()); /*<Sets optical object properties*/
    map3->SetScalarModeOn();
    map3->AddMaterial(mVSdf2.GetOutput(), mVSdf2.GetType()); /*<Sets auditory object properties*/
    map3->Update();

    /*
    Creates actor
    */
    vaActor* acSdfMol = new vaActor();
    acSdfMol->SetContext(m.GetOutput()); //sets context and initialize acceleration properties
    acSdfMol->AddMapper(map3);
    acSdfMol->Update();

    //RTvisibilitymask mask;
    //rtGeometryGroupGetVisibilityMask(acSdfMol->GetOutput(), &mask)
    //RTvisibilitymask mask = acSdfMol->GetOutput()->getVisibilityMask();
    //std::cout << " VIS Mask = " << mask << std::endl;
    //acSdfMol->GetOutput()->setVisibilityMask(2);

    ren.AddActor(acSdfMol);
    //store
    actorSdf.push_back(acSdfMol);
    mappers.push_back(map3);
}
// Scene testing all materials on a single geometry instanced via transforms and sharing one acceleration structure.
void SceneManager::createScene()
{
    //initMaterials();
    std::cout << "Start" << std::endl;
    try
    {
        ///Look to
        //OpenNurbs references on API
        //http://www.visigrapp.org/ - for 4 october

        m.GetOutput()->setMaxCallableProgramDepth(80);

        //createTriangulatedScene();

        //experiments with heterogeneous objects
        //and interactive visual-auditory ray-casting
        //createDynamicHeterogeneousObjectScene();

        createMolSolventScene();
        //createAuditoryMoleculeScene(); - broken
        //createFrepScene();
        //------------------
        //TODO:createAuditoryMoleculeSceneDynamic
        //start writing optical and auditory models through materials for
        //ray-casting

        //--------------------------------------------
        //experiment with molecules blending and dynamic molecules rendering

       // createAuditoryMoleculeScene2(); //for latest sweeping and etc
       // createAuditoryMoleculeSceneMolDynam();
    }
    catch (optix::Exception& e)
    {
        std::cerr << e.getErrorString() << std::endl;
    }
}