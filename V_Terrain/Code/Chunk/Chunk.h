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

#include "../Mesh/MeshGenerator.h"

#include <mutex>

namespace VTerrain
{
    class ChunkManager
    {
    public:
        class Chunk
        {
        public:
            Chunk();

            void Regenerate(Vec2<int> offset);
            void Free();
            void Draw(const float* viewMatrix, const float* projectionMatrix, Vec2<int> pos, uint LOD = 0);
            bool IsLODReady(uint LOD);

        private:
            std::thread m_RegenThread;
            uint m_maxLOD;

            MeshGenerator::Mesh m_mesh;
        };

        ChunkManager();

        static ChunkManager& Instance() { return m_instance; }
        static void Update(int posX, int posY);
        static void Render(const float* viewMatrix, const float* projectionMatrix);
        static void CleanChunks();
        static void RegenAll();

    private:
        static void FreeChunk();
        static void RegenChunk();

        static void AddChunksToRegen(Vec2<int> pos);
        static void AddChunkToRegen(Vec2<int> pos);	
        static void AddChunkToForceRegen(Vec2<int> pos);

        static bool IsVisible(Vec2<int> pos);
		static bool IsLoaded(Vec2<int> pos);
        static Vec2<int> GetFurthestChunk();

        std::map<Vec2<int>, Chunk> m_chunks;
        std::queue<Vec2<int>> m_chunkstoRegen;
        std::queue<Vec2<int>> m_chunkstoForceRegen;
        Vec2<int> m_lastOffPos;
        Vec2<int> m_currentChunk;

        bool m_firstFrame;

        static ChunkManager m_instance;
    };
}