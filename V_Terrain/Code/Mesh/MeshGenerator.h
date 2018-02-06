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
            void AddVertex(const Vec3<float>& v);
            void AddUV(const Vec2<float>& uv);
            void AddNormal(const Vec3<float>& n);

            void AddTri(uint a, uint b, uint c);

        public:
            std::vector<Vec3<float>> m_vertices;
            std::vector<Vec2<float>> m_UVs;
            std::vector<Vec3<float>> m_normals;
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