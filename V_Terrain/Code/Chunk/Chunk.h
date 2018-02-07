#pragma once

#include "../Utils/Globals.h"

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

            void Regenerate(uint LOD);
            void Regenerate(uint LOD, int offsetX, int offsetY);
            void Free();
            void Draw(const float* viewMatrix, const float* projectionMatrix);
			Vec2<int> GetPos() { return Vec2<int>(m_offsetX, m_offsetY); }
			uint GetLOD() { return m_LOD; }

        private:
            MeshGenerator::Mesh m_mesh;
            int m_offsetX;
            int m_offsetY;
            uint m_LOD;
        };

        ChunkManager();

        static ChunkManager& Instance() { return m_instance; }
        static void Update(int posX, int posY);
        static void Render(const float* viewMatrix, const float* projectionMatrix);
        
        static void SetMaxChunks(uint maxChunks);
    private:
        static void RegenChunk();
        static void AddChunkToRegen(uint LOD, Vec2<int> pos);
		static bool ChunksToRegen();
			
		static int FindChunk(Vec2<int> pos);
		static void RemoveRegenDuplicates(Vec2<int> pos);

        std::vector<Chunk> m_chunks;
        uint m_lastChunkChecked;
        std::map<uint, std::list<Vec2<int>>> m_chunkstoRegen;
        Vec2<int> m_lastOffPos;

        static ChunkManager m_instance;
    };
}