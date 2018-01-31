#include "MeshGenerator.h"

namespace VTerrain
{
    MeshGenerator MeshGenerator::m_instance = MeshGenerator();

    MeshGenerator::Mesh::Mesh()
    {
    }

    MeshGenerator::Mesh::~Mesh()
    {
    }

    void MeshGenerator::Mesh::AddTri(std::vector<float> Tri)
    {
    }

    MeshGenerator::MeshGenerator()
    {
    }

    MeshGenerator::Mesh MeshGenerator::GenerateMesh(const PerlinNoise::NoiseMap & map)
    {
        Mesh ret;

        return ret;
    }
}