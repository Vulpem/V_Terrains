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
            void AddVertex(float x, float y, float z);
            void AddUV(float x, float y);
            void AddNormal(float x, float y, float z);

            void AddTri(uint a, uint b, uint c);

        public:
            std::vector<float> m_vertices;
            std::vector<float> m_UVs;
            std::vector<float> m_normals;
            std::vector<float> m_data;

            std::vector<uint> m_indices;
        };

        class Mesh
        {
        public:
            uint m_dataBuff;
            uint m_indicesBuff;
            uint m_nIndices = 0u;
            bool used = false;

            static uint m_shaderProgram;
           
            void Generate(const MeshData& meshData);
            void FreeMesh();

            void Render(const float* viewMatrix, const float* projectionMatrix);
        };
#pragma endregion
    };
}