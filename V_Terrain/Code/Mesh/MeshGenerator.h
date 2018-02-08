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

        private:
            void AddVertex(const Vec3<float>& v);
            void AddUV(const Vec2<float>& uv);
            void AddNormal(const Vec3<float>& n);

            void AddTri(uint a, uint b, uint c);
            uint m_nVertices = 0;
            uint m_nUVs = 0;
            uint m_nNormals = 0;

        public:
            std::vector<float> m_data;

            std::vector<uint> m_indices;
        };

        class Mesh
        {
        public:
            uint m_dataBuff;
            uint m_indicesBuff;
            uint m_nIndices = 0u;
            bool m_used = false;

            static uint m_shaderProgram;
           
            void Generate(const MeshData& meshData);
            void FreeMesh();

            void Render(const float* viewMatrix, const float* projectionMatrix, const Vec3<int>& offset);
        };
#pragma endregion
    };
}