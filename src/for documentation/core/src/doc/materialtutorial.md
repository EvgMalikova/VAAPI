Custom material creation
========================


Example of custom optical material creation on base of Emission-absorption Volume.

C++ files
---------

The Material class will be defined as :
~~~c

class SdfEmAbsorbVolume : public visAuditoryMaterial {

public:
    SdfEmAbsorbVolume() { };
    ~SdfEmAbsorbVolume() {};


protected:
    virtual void SetMaterialParameters();
    virtual void Initialize();
};
~~~

In this tutorial the following 2 functions inherited from visAuditoryMaterial  class: would be reimplemented:
Initialize(); - defines Optix closest_hit, any_hit and potentially callable programs to render the optix material. For more information reference Optix documentation

SetMaterialParameters(); - sets custom variables, called from cuda programs



Any cuda program can be inited and after called by name through procedure:
    void InitProg(std::string prog/*<name of program in cuda file*/, std::string file/*<name of cuda file*/, std::string name /*<assigned name of program for further reference*/);

After program can be referenced by assigned to it name.

There are only three main programs for material, defined with reserved names
~~~c

    std::string anyhit_prog; /*<Any hit optix program name */
    std::string closesthit_prog; /*<Closest hit optix program name */
    std::string dynamic_prog; /*<Name for callable procedure defining some dynamic changes to material*/
~~~
The names of those programs for further reference are assigned through procedures:
~~~c

   void SetClosestHitProgName(std::string name); /*<Sets closest hit program name for further reference with GetClosestHitProgName()*/

    void SetAnyHitProgName(std::string name); /*<Sets any hit program name for further reference*/
    void SetDynamicProgName(std::string name);/*<Sets dynamic highlight program name*/
~~~

For example the initialisation procedure for closest hit program will look like

~~~c
            InitProg("closesthit_sdf", "closesthit.cu", "closesthit_sdf");
            SetClosestHitProgName("closesthit_sdf"); //sets closesthit to true;
~~~


In this tutorial the any hit program will be initialised as 


void SdfEmAbsorbVolume::Initialize() {

    if (optixObject::GetContext()->get() != nullptr)
    {
        try
        {
            InitProg("closesthit_sdf", "closesthit.cu", "closesthit_sdf");
            SetClosestHitProgName("closesthit_sdf"); //sets closesthit to true;

        }
        catch (optix::Exception& e)
        {
            std::cerr << e.getErrorString() << std::endl;
        }
    }
    else std::cerr << "no context defined" << std::endl;

}


An example of setting custom parameters:
~~~c
void SdfEmAbsorbVolume::SetMaterialParameters()
{

    //optix::Material m = GetMaterial(i);
    GetOutput()["ambient_light_color"]->setFloat(1.0, 1.0, 0.0);
    //m["albedo"]->setFloat(1.0f, 0.5f, 0.0f);

    //TODO: setupLights



}
~~~


Cuda files
----------

