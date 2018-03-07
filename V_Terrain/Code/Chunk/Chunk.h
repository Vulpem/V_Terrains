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
#include "ChunkFactory.h"
#include "../Mesh/MeshGenerator.h"

namespace VTerrain
{
    class ChunkManager
    {
    public:
        class Chunk
        {
        public:
            Chunk();

            void Regenerate(ChunkFactory::GeneratedChunk base);
            void Free();
            void Draw(const float* viewMatrix, const float* projectionMatrix, uint LOD = 0);
            bool IsLODReady(uint LOD);
            bool IsLoaded();
            Vec2<int> GetPos() { return m_pos; }

            static uint m_shaderProgram;
			static Mesh m_mesh;
        private:
            Vec2<int> m_pos;
            uint m_minLOD;
            uint m_buf_heightmap;
        };

        ChunkManager();

        void Update(int posX, int posY);
        void Render(const float* viewMatrix, const float* projectionMatrix);
        void CleanChunks();

    private:
        void AddChunksToRegen(Vec2<int> pos);
        void AddChunkToRegen(Vec2<int> pos);	

        Chunk& GetChunk(Vec2<int> pos);
		bool IsLoaded(Vec2<int> pos);
        Chunk& GetFurthestChunk();

        std::vector<Chunk> m_chunks;

        Vec2<int> m_lastOffPos;
        Vec2<int> m_currentChunk;

        ChunkFactory m_factory;

        bool m_firstFrame;
    };
}