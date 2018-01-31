#pragma once

#include "../Utils/Globals.h"

#include "../Noise/PerlinNoise.h"

namespace VTerrain
{
    class MeshGenerator
    {
    public:
#pragma region Mesh
        class Mesh
        {
            std::vector<float> m_vertices;
            std::vector<uint> indices;

        public:
            Mesh();
            ~Mesh();

            void AddTri(std::vector<float> Tri);
        };
#pragma endregion

    public:
        MeshGenerator();

        static Mesh GenerateMesh(const PerlinNoise::NoiseMap& map);

        MeshGenerator& Instance() { return m_instance; }
    private:
        static MeshGenerator m_instance;
    };
}