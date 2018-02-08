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

            void Regenerate(uint LOD);
            void Regenerate(uint LOD, Vec2<int> offset);
            void Free();
            void Draw(const float* viewMatrix, const float* projectionMatrix);
			Vec2<int> GetPos() { return m_offset; }
			uint GetLOD() { return m_LOD; }
            bool IsUsed() { return m_mesh.m_used; }
            float DistanceToSqr(Vec2<int> chunkIndex);

            std::mutex m_mutex;
        private:

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
        static void AddChunkToRegen(uint LOD, Vec2<int> pos);		

        static bool IsVisible(Vec2<int> pos);
		static bool IsLoaded(Vec2<int> pos);
        static Vec2<int> GetFurthestChunk();

        std::map<Vec2<int>, Chunk> m_chunks;
        std::map<uint, std::list<Vec2<int>>> m_chunkstoRegen;
        Vec2<int> m_lastOffPos;
        Vec2<int> m_currentChunk;

        static ChunkManager m_instance;
    };
}