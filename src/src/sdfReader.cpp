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
    if (type == "Si")       //and check its value
    {
        vel_magnitude = 7;// / 8.0;
    }
    return vel_magnitude;
}

float sdfReader::GetAtomRadii(std::string type)
{
    //https://en.wikipedia.org/wiki/Atomic_radius
    //Devided by 100
    float vel_magnitude = 0;
    float less = 1.5;

    if (type == "H")       //and check its value
    {
        vel_magnitude = 0.25 / less; ///yellow
    }
    if (type == "C")       //and check its value
    {
        vel_magnitude = 0.70 / less;
    }
    if (type == "N")       //and check its value
    {
        vel_magnitude = 0.65 / less;// / 8.0;
    }
    if (type == "S")       //and check its value
    {
        vel_magnitude = 1.0 / less; //not visible
    }
    if (type == "Si")       //and check its value
    {
        vel_magnitude = 1.0 / less; //not visible
    }
    if (type == "O")       //and check its value
    {
        vel_magnitude = 0.60 / less;
    }
    if (type == "P")       //and check its value
    {
        vel_magnitude = 1.0 / less;// / 8.0;
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
//*----tetReader
//*
void tetReader::ReadFile()
{
    std::shared_ptr<TETRAHEDRAS> d = optixReader<TETRAHEDRAS>::GetOutput();

    std::string filename = optixReader<TETRAHEDRAS>::Getfile();
    //manage particle frame and others

    std::ifstream ifs(filename.c_str());

    d->bbox_min.x = d->bbox_min.y = d->bbox_min.z = 1e16f;
    d->bbox_max.x = d->bbox_max.y = d->bbox_max.z = -1e16f;
    std::cout << "Reading tet file " << optixReader<TETRAHEDRAS>::Getfile() << std::endl;
    int maxchars = 8192;
    std::vector<char> buf(static_cast<size_t>(maxchars)); // Alloc enough size

    float wmin = 1e16f;
    float wmax = -1e16f;

    int numAt;
    /*
    d->centers.push_back(optix::make_float3(x, y, z));
              d->type.push_back(vel_magnitude);
              d->rad.push_back(radii);
    */
    while (ifs.peek() != -1) {
        ifs.getline(&buf[0], maxchars);

        std::string line(&buf[0]);

        std::istringstream in(line);      //make a stream for the line itself
        std::string header;
        in >> header; //"POINTS"
        int Num;
        if (header == "POINTS")
            // processPoints()
        {
            in >> Num;

            int ids = 0;
            while (ids < Num) {
                ifs.getline(&buf[0], maxchars);
                std::string line(&buf[0]);

                std::stringstream in(line);
                float x, y, z;

                while (!in.eof()) {
                    in >> x >> y >> z;
                    int length = in.tellg();
                    if (length > 0)
                        d->centers.push_back(optix::make_float3(x, y, z));
                }
                ids = d->centers.size();
                //std::cout << ids << "points " << x << "," << y << "," << z << std::endl;
            }
        }

        if (header == "CELLS")
            // processPoints()
        {
            in >> numAt;
            int id = 4;
            while (id == 4) {
                ifs.getline(&buf[0], maxchars);
                std::string line(&buf[0]);

                std::istringstream in(line);

                int  v1, v2, v3, v4;
                in >> id >> v1 >> v2 >> v3 >> v4;
                if (id == 4)
                    d->tetra.push_back(optix::make_int4(v1, v2, v3, v4));
            }
        }

        //skip last line
        ifs.getline(&buf[0], maxchars);
    }

    const size_t numParticles = d->centers.size();
    const size_t numTet = d->tetra.size();

    std::cout << "# points = " << numParticles << std::endl;
    std::cout << "# tetras = " << numTet << std::endl;

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

    wmin = 100;
    wmax = 0;
    for (size_t i = 0; i < numTet; i++)
    {
        const optix::int4& tet = d->tetra[i];
        const optix::float3& p1 = d->centers[tet.x];
        const optix::float3& p2 = d->centers[tet.y];
        const optix::float3& p3 = d->centers[tet.z];
        const optix::float3& p4 = d->centers[tet.w];

        optix::float3 cent = (p1 + p2 + p3 + p4) / 4;
        float rad = optix::length(cent - p1);

        d->type.push_back(int(rad));
        if (wmin > rad) wmin = rad;
        if (wmax < rad) wmax = rad;
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

    std::cout << "Attribute range wmin = " << wmin << ", wmax = " << wmax << std::endl;

    float wRange = float(1.0 / double(wmax - wmin));
}

//*----sdfMolReader
//*

void sdfMolReader::ReadFile()
{
    std::shared_ptr<MOL> d = optixReader<MOL>::GetOutput();

    std::string filename = optixReader<MOL>::Getfile();
    //manage particle frame and others

    std::ifstream ifs(filename.c_str());

    d->bbox_min.x = d->bbox_min.y = d->bbox_min.z = 1e16f;
    d->bbox_max.x = d->bbox_max.y = d->bbox_max.z = -1e16f;
    // std::cout << "Reading txt file " << optixReader<MOL>::Getfile() << std::endl;
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
        int idPrev;
        int idMol;
        //int idBond;
        //Molecule m;

        for (int i = 0; i < numBt; i++)
        {
            ifs.getline(&buf[0], maxchars);
            std::string line(&buf[0]);

            std::istringstream in(line);

            int id1, id2, t;
            in >> id1 >> id2 >> t;
            d->bonds.push_back(optix::make_int2(id1, id2));
            if (i > 0) { //scanning for molecules
                if (id1 == idPrev) //the first element
                {
                    // m.id = 0;
                    d->mols[idMol].bond_id.push_back(i);
                }
                else
                {
                    //next molecule
                    idMol++;

                    d->mols.push_back(Molecule());
                    d->mols[idMol].id = idMol;
                    d->mols[idMol].bond_id.push_back(i);
                    //next molecule
                   // idPrev = id1;
                }
            }
            else {//create first molecule
                idMol = 0;
                d->mols.push_back(Molecule());
                d->mols[idMol].id = idMol;
                d->mols[idMol].bond_id.push_back(i);
            }
            idPrev = id1;
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

    std::cout << "Mols = " << d->mols.size() << std::endl;
    int maxS = d->mols[0].bond_id.size();
    for (size_t i = 0; i < d->mols.size(); i++)
    {
        /*std::cout << "Mols = ";
        for (int j = 0; j < d->mols[i].bond_id.size(); j++)
        {
            std::cout << d->mols[i].bond_id[j] << "  ";
        }
        std::cout << std::endl;
        */
        if (maxS < d->mols[i].bond_id.size())
            maxS = d->mols[i].bond_id.size();
    }

    d->maxMolSize = maxS;
}