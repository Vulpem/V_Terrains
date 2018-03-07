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
//  You should have received a copy of the GNU General Public License along with V Terrains.  If not, see <http://www.gnu.org/licenses/>.
#pragma once

#include "../Globals.h"

namespace VTerrain
{
	class Mesh
        {
        public:
            Mesh();
            ~Mesh();

            void Generate();
            void Free();

            uint GetMeshBuf() { return m_dataBuff; }
            uint GetIndicesBuf(uint LOD) { return m_indicesBuff[LOD]; }
            uint GetNumIndices(uint LOD) { return m_nIndices[LOD]; }
            bool IsGenerated() { return m_generated; }
        private:

            void GenerateMesh();
            void GenerateIndices(uint LOD);

            void FreeMesh();
            void FreeLOD(uint lod);

            bool m_bufferGenerated = false;
            std::map<uint, uint> m_indicesBuff;
            std::map<uint, uint> m_nIndices;
            uint m_dataBuff;
            bool m_generated;
        };
}