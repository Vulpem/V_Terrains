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
        static void AddChunkToRegen(uint LOD, int offX, int offY);
        static bool ChunksToRegen();

        std::vector<Chunk> m_chunks;
        uint m_lastChunkChecked;
        std::map<uint, std::queue<Vec2<int>>> m_chunkstoRegen;
        Vec2<int> m_lastOffPos;

        static ChunkManager m_instance;
    };
}