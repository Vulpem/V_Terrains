//  V Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hern�ndez L�zaro
//  
//  "V Terrains" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or any later version.
//  
//  "V Terrains" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//  
//  For more details, read "COPYING.txt" and "COPYING.LESSER.txt" included in this project.
//  You should have received a copy of the GNU General Public License along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
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
            
            void GenerateData(const PerlinNoise::NoiseMap& map);
        private:
            void AddVertex(const Vec3<float>& v);
            void AddUV(const Vec2<float>& uv);
            void AddNormal(const Vec3<float>& n);

            uint m_nVertices = 0;
            uint m_nUVs = 0;
            uint m_nNormals = 0;

        public:
            std::vector<float> m_data;
        };

        class Mesh
        {
        public:
            void Generate(const MeshData& meshData);
            static void GenerateIndices(uint width, uint height, uint LOD);

            void FreeMesh();

            void Render(const float* viewMatrix, const float* projectionMatrix, const Vec3<int>& offset, uint LOD);
       
            uint m_dataBuff;
            static uint m_shaderProgram;
        private:
            bool m_bufferGenerated = false;
            static std::map<uint, uint> m_indicesBuff;
            static std::map<uint, uint> m_nIndices;
        };
#pragma endregion
    };
}