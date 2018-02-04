#pragma once

#include "../Utils/Globals.h"

#include "../Noise/PerlinNoise.h"

namespace VTerrain
{
    class MeshGenerator
    {
    public:
#pragma region Mesh
        class MeshData
        {
        public:
            MeshData();
            ~MeshData();
            
            void Generate(const PerlinNoise::NoiseMap& map);
            void CompressData();

        private:
            void AddVertex(double x, double y, double z);
            void AddUV(double x, double y);
            void AddNormal(double x, double y, double z);

            void AddTri(uint a, uint b, uint c);

        public:
            std::vector<double> m_vertices;
            std::vector<double> m_UVs;
            std::vector<double> m_normals;
            std::vector<double> m_data;

            std::vector<uint> m_indices;
        };

        class Mesh
        {
        public:
            uint m_dataBuff;
            uint m_indicesBuff;
            uint m_nIndices = 0u;
            bool used = false;
           
            void Generate(const MeshData& meshData);
            void FreeMesh();

            void Render();
        };
#pragma endregion
    };
}