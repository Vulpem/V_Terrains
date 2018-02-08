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

            void Regenerate(uint gen, uint LOD);
            void Regenerate(uint gen, uint LOD, Vec2<int> offset);
            void Free();
            void Draw(const float* viewMatrix, const float* projectionMatrix);
			Vec2<int> GetPos() { return m_offset; }
			uint GetLOD() { return m_LOD; }
            uint GetGen() { return m_generation; }
            bool IsUsed() { return m_mesh.m_used; }

            std::mutex m_mutex;
        private:
            uint m_generation;

            MeshGenerator::Mesh m_mesh;
            Vec2<int> m_offset;
            uint m_LOD;
        };

        ChunkManager();

        static ChunkManager& Instance() { return m_instance; }
        static void Update(int posX, int posY);
        static void Render(const float* viewMatrix, const float* projectionMatrix);
        
    private:
        static void FreeChunk();
        static void RegenChunk();
        static void AddChunkToRegen(uint gen, uint LOD, Vec2<int> pos);		

        static bool IsVisible(Vec2<int> pos);
		static bool IsLoaded(Vec2<int> pos);
        static Vec2<int> GetOldestChunk();

        std::map<Vec2<int>, Chunk> m_chunks;
        std::map<uint, std::list<std::pair<Vec2<int>, uint>>> m_chunkstoRegen;
        Vec2<int> m_lastOffPos;
        uint m_chunkGeneration;
        uint m_oldestGeneration;

        static ChunkManager m_instance;
    };
}