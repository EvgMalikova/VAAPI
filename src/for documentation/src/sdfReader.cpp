#include "sdfReader.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

#include <iomanip>
#include <fstream>

int sdfReader::GetAtomNumber(std::string type)
{
    int vel_magnitude = 0;

    if (type == "H")       //and check its value
    {
        vel_magnitude = 1; ///yellow
    }
    if (type == "C")       //and check its value
    {
        vel_magnitude = 2;
    }
    if (type == "N")       //and check its value
    {
        vel_magnitude = 3;// / 8.0;
    }
    if (type == "S")       //and check its value
    {
        vel_magnitude = 4; //not visible
    }
    if (type == "O")       //and check its value
    {
        vel_magnitude = 5;
    }
    if (type == "P")       //and check its value
    {
        vel_magnitude = 6;// / 8.0;
    }
    return vel_magnitude;
}

float sdfReader::GetAtomRadii(std::string type)
{
    //https://en.wikipedia.org/wiki/Atomic_radius
    //Devided by 100
    float vel_magnitude = 0;

    if (type == "H")       //and check its value
    {
        vel_magnitude = 0.25; ///yellow
    }
    if (type == "C")       //and check its value
    {
        vel_magnitude = 0.70;
    }
    if (type == "N")       //and check its value
    {
        vel_magnitude = 0.65;// / 8.0;
    }
    if (type == "S")       //and check its value
    {
        vel_magnitude = 1.0; //not visible
    }
    if (type == "O")       //and check its value
    {
        vel_magnitude = 0.60;
    }
    if (type == "P")       //and check its value
    {
        vel_magnitude = 1.0;// / 8.0;
    }
    return vel_magnitude;
}

void sdfReader::ReadFile()
{
    std::shared_ptr<MOL> d = optixReader<MOL>::GetOutput();

    std::string filename = optixReader<MOL>::Getfile();
    //manage particle frame and others

    std::ifstream ifs(filename.c_str());

    d->bbox_min.x = d->bbox_min.y = d->bbox_min.z = 1e16f;
    d->bbox_max.x = d->bbox_max.y = d->bbox_max.z = -1e16f;
    std::cout << "Reading txt file " << optixReader<MOL>::Getfile() << std::endl;
    int maxchars = 8192;
    std::vector<char> buf(static_cast<size_t>(maxchars)); // Alloc enough size

    float wmin = 1e16f;
    float wmax = -1e16f;

    //read number of atoms
   // ifs.getline(&buf[0], maxchars);

    //std::string line(&buf[0]);
   // std::istringstream in(line);

  //  int numAt;
  //  in >> numAt;
    while (ifs.peek() != -1)
        // for ( int i=0;i<numAt;i++)
    {
        ifs.getline(&buf[0], maxchars);

        std::string line(&buf[0]);

        std::istringstream in(line);      //make a stream for the line itself

        int numAt;
        int numBt;
        in >> numAt;
        in >> numBt;
        //skip next line
        //ifs.getline(&buf[0], maxchars);

        for (int i = 0; i < numAt; i++)
        {
            ifs.getline(&buf[0], maxchars);
            std::string line(&buf[0]);

            std::istringstream in(line);

            float x, y, z;
            in >> x >> y >> z;       //now read the whitespace-separated floats
                                     /*
                                     Hydrogen = White
                                     Oxygen = Red
                                     Chlorine = Green
                                     Nitrogen = Blue
                                     Carbon = Grey
                                     Sulphur = Yellow
                                     Phosphorus = Orange
                                     Other = Varies - mostly Dark Red/Pink/Maroon*/
                                     //int nn= getAtomNumber(type);

            std::string type;
            in >> type;                  //and read the first whitespace-separated token

            int vel_magnitude = GetAtomNumber(type);//getAtomVelocity(nn);
            float radii = GetAtomRadii(type);
            wmin = fminf(wmin, vel_magnitude);
            wmax = fmaxf(wmax, vel_magnitude);

            d->centers.push_back(optix::make_float3(x, y, z));
            d->type.push_back(vel_magnitude);
            d->rad.push_back(radii);
            //  radii.push_back( 2.0 );
        }
        //scanning for bonds
        for (int i = 0; i < numBt; i++)
        {
            ifs.getline(&buf[0], maxchars);
            std::string line(&buf[0]);

            std::istringstream in(line);

            int id1, id2, t;
            in >> id1 >> id2 >> t;
            d->bonds.push_back(optix::make_int2(id1, id2));
        }
    }

    const size_t numParticles = d->centers.size();
    std::cout << "# particles = " << numParticles << std::endl;

    optix::float3 pmin, pmax;
    pmin.x = pmin.y = pmin.z = 1e16f;
    pmax.x = pmax.y = pmax.z = -1e16f;

    for (size_t i = 0; i < numParticles; i++)
    {
        const optix::float3& p = d->centers[i];

        pmin.x = fminf(pmin.x, p.x);
        pmin.y = fminf(pmin.y, p.y);
        pmin.z = fminf(pmin.z, p.z);

        pmax.x = fmaxf(pmax.x, p.x);
        pmax.y = fmaxf(pmax.y, p.y);
        pmax.z = fmaxf(pmax.z, p.z);
    }

    //    std::cout << "Particle pmin = " << pmin << std::endl;
    //    std::cout << "Particle pmax = " << pmax << std::endl;

    d->bbox_min = pmin;
    d->bbox_max = pmax;
    float fixed_radius = 0.6; //todo: compute
   // if (fixed_radius == 0.1f)
   //     fixed_radius = optix::length(bbox_max - bbox_min) / powf(float(numParticles), 0.333333f);

    std::cout << "Using fixed_radius = " << fixed_radius << std::endl;

    d->bbox_min -= optix::make_float3(fixed_radius);
    d->bbox_max += optix::make_float3(fixed_radius);
}