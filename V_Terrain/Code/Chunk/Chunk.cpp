#include "Chunk.h"
#include <math.h>

namespace VTerrain
{
    ChunkManager ChunkManager::m_instance = ChunkManager();

    ChunkManager::Chunk::Chunk() :
        m_LOD(0u)
        , m_offsetX(0u)
        , m_offsetY(0u)
    {}

    void ChunkManager::Chunk::Regenerate(uint LOD)
    {
        Regenerate(LOD, m_offsetX, m_offsetY);
    }

    void ChunkManager::Chunk::Regenerate(uint LOD, int offsetX, int offsetY)
    {
        m_offsetX = offsetX;
        m_offsetY = offsetY;
        m_LOD = LOD;

        VTerrain::PerlinNoise::NoiseMap m_noiseMap = VTerrain::PerlinNoise::GenNoiseMap(m_offsetX, m_offsetY);
        VTerrain::MeshGenerator::MeshData meshData;
        meshData.Generate(m_noiseMap);
        m_mesh.Generate(meshData);
    }

    void ChunkManager::Chunk::Free()
    {
        m_mesh.FreeMesh();
    }

    void ChunkManager::Chunk::Draw(const float* viewMatrix, const float* projectionMatrix)
    {
        if (m_mesh.used)
        {
            m_mesh.Render(viewMatrix, projectionMatrix, Vec3<int>(m_offsetX * Config::chunkWidth,0u,m_offsetY * Config::chunkHeight));
        }
    }

    ChunkManager::ChunkManager() :
        m_lastChunkChecked(0u)
        , m_lastOffPos(-1, -1)
    {
        SetMaxChunks(Config::GetMaxChunks());
    }

    void ChunkManager::Update(int posX, int posY)
    {
        Vec2<int> off(
            floor((posX - (Config::chunkWidth / 2) + (Config::chunkWidth % 2 != 0)) / Config::chunkWidth + 1),
            floor((posY - (Config::chunkHeight / 2) + (Config::chunkWidth % 2 != 0)) / Config::chunkHeight) + 1);
        if (off != m_instance.m_lastOffPos)
        {
            m_instance.m_lastOffPos = off;
            for (int y = -1; y <= 1; y++)
            {
                for (int x = -1; x <= 1; x++)
                {
                    if (x == 0 && y == 0)
                    {
                        AddChunkToRegen(1, off.x() + x, off.y() + y);
                    }
                    else
                    {
                        AddChunkToRegen(2, off.x() + x, off.y() + y);
                    }
                }
            }

        }

        RegenChunk();
    }

    void ChunkManager::Render(const float * viewMatrix, const float * projectionMatrix)
    {
        for (auto it : m_instance.m_chunks)
        {
            it.Draw(viewMatrix, projectionMatrix);
        }
    }

    void ChunkManager::SetMaxChunks(uint maxChunks)
    {
        m_instance.m_chunks.resize(maxChunks);
    }

    void ChunkManager::RegenChunk()
    {
        for (auto it = m_instance.m_chunkstoRegen.begin(); it != m_instance.m_chunkstoRegen.end(); it++)
        {
            if (it->second.empty() == false)
            {
                const Vec2<int> offset = it->second.front();
                const uint LOD = it->first;
                it->second.pop();
                if (m_instance.m_lastChunkChecked >= Config::GetMaxChunks()) { m_instance.m_lastChunkChecked = 0; }
                m_instance.m_chunks[m_instance.m_lastChunkChecked++].Regenerate(LOD, offset.x(), offset.y());                
                return;
            }
        }
    }

    void ChunkManager::AddChunkToRegen(uint LOD, int offX, int offY)
    {
        m_instance.m_chunkstoRegen[LOD].push(Vec2<int>(offX, offY));
    }

    bool ChunkManager::ChunksToRegen()
    {
        for (auto it : m_instance.m_chunkstoRegen)
        {
            if (it.second.empty() == false)
            {
                return true;
            }
        }
        return false;
    }
}