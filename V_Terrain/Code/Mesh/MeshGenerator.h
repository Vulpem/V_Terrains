//  V Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hernàndez Làzaro
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
        class Mesh
        {
        public:
            static void Generate();
            static void Free();

            static uint GetMeshBuf() { return m_dataBuff; }
            static uint GetIndicesBuf(uint LOD) { return m_indicesBuff[LOD]; }
            static uint GetNumIndices(uint LOD) { return m_nIndices[LOD]; }

        private:
            static void GenerateMesh();
            static void GenerateIndices(uint LOD);

            static void FreeMesh();
            static void FreeLOD(uint lod);

            bool m_bufferGenerated = false;
            static std::map<uint, uint> m_indicesBuff;
            static std::map<uint, uint> m_nIndices;
            static uint m_dataBuff;
        };
#pragma endregion
    };
}