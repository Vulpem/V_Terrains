#include "Chunk.h"
#include <math.h>

namespace VTerrain
{
    ChunkManager ChunkManager::m_instance = ChunkManager();

    ChunkManager::Chunk::Chunk() :
        m_LOD(0u)
        , m_offset(0,0)
    {}

    void ChunkManager::Chunk::Regenerate(uint LOD, bool force)
    {
        Regenerate(LOD, m_offset, force);
    }

    void ChunkManager::Chunk::Regenerate(uint LOD, Vec2<int> offset, bool force)
    {
		if (LOD != m_LOD || force == true)
		{
            m_offset = offset;
			m_LOD = LOD;

			VTerrain::PerlinNoise::NoiseMap m_noiseMap = VTerrain::PerlinNoise::GenNoiseMap(offset); //46/60ms
			VTerrain::MeshGenerator::MeshData meshData;
			meshData.Generate(m_noiseMap); //251/150ms
			m_mesh.Generate(meshData);
		}
    }

    void ChunkManager::Chunk::Free()
    {
        m_mesh.FreeMesh();
    }

    void ChunkManager::Chunk::Draw(const float* viewMatrix, const float* projectionMatrix)
    {
        if (IsUsed())
        {
            m_mesh.Render(viewMatrix, projectionMatrix, Vec3<int>(m_offset.x() * Config::chunkWidth,0u,m_offset.y() * Config::chunkHeight));
        }
    }

    float ChunkManager::Chunk::DistanceToSqr(Vec2<int> chunkIndex)
    {
        return (m_offset - chunkIndex).LengthSqr();
    }

    ChunkManager::ChunkManager() :
        m_lastOffPos(0, 0)
        , m_currentChunk(0, 0)
    {
    }

    void ChunkManager::Update(int posX, int posY)
    {
        const int W = static_cast<int>(Config::chunkWidth);
        const int H = static_cast<int>(Config::chunkHeight);
        Vec2<int> off(
            floor((posX - floor(W / 2.f) + (W % 2 != 0)) / W) + 1,
            floor((posY - floor(H / 2.f) + (H % 2 != 0)) / H) + 1
        );
        
        if (off != m_instance.m_lastOffPos || m_instance.m_chunks.empty() == true)
        {
            m_instance.m_lastOffPos = off;
            AddChunksToRegen(off);
        }

        RegenChunk();
        if (m_instance.m_chunks.size() > Config::GetMaxChunks())
        {
            FreeChunk();
        }
    }

    void ChunkManager::Render(const float * viewMatrix, const float * projectionMatrix)
    {
        for (auto it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            it->second.Draw(viewMatrix, projectionMatrix);
        }
    }

    void ChunkManager::CleanChunks()
    {
        for (auto it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            it->second.Free();
        }
        m_instance.m_chunks.clear();
        m_instance.m_chunkstoRegen.clear();
        AddChunksToRegen(m_instance.m_lastOffPos);
    }

    void ChunkManager::RegenAll()
    {
        for (auto it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            AddChunkToForceRegen(it->second.GetLOD(), it->second.GetPos());
        }
    }

    void ChunkManager::FreeChunk()
    {
        m_instance.m_chunks.erase(GetFurthestChunk());
    }

    void ChunkManager::RegenChunk()
    {
        for (auto it = m_instance.m_chunkstoForceRegen.begin(); it != m_instance.m_chunkstoForceRegen.end(); it++)
        {
            if (it->second.empty() == false)
            {
                const Vec2<int> offset = it->second.front();
                const uint LOD = it->first;
                m_instance.m_chunks[offset].Regenerate(LOD, offset, true);
                it->second.pop_front();
                return;
            }
        }

        for (auto it = m_instance.m_chunkstoRegen.begin(); it != m_instance.m_chunkstoRegen.end(); it++)
        {
            if (it->second.empty() == false)
            {
                const Vec2<int> offset = it->second.front();
                    const uint LOD = it->first;
                    m_instance.m_chunks[offset].Regenerate(LOD, offset);
                    it->second.pop_front();
                    return;
               
            }
        }
    }

    void ChunkManager::AddChunksToRegen(Vec2<int> pos)
    {
        //TODO configurable
        for (int y = -1; y <= 1; y++)
        {
            for (int x = -1; x <= 1; x++)
            {
                AddChunkToRegen(1, Vec2<int>(pos.x() + x, pos.y() + y));
            }
        }
    }

    void ChunkManager::AddChunkToRegen(uint LOD, Vec2<int> pos)
    {
        m_instance.m_chunkstoRegen[LOD].push_back(pos);
    }

    void ChunkManager::AddChunkToForceRegen(uint LOD, Vec2<int> pos)
    {
        m_instance.m_chunkstoForceRegen[LOD].push_back(pos);
    }

    bool ChunkManager::IsVisible(Vec2<int> pos)
    {
        //TODO
        return false;
    }

    bool ChunkManager::IsLoaded(Vec2<int> pos)
	{
        return m_instance.m_chunks.find(pos) != m_instance.m_chunks.end();
	}

    Vec2<int> ChunkManager::GetFurthestChunk()
    {
        //TODO optimize
        float maxDist = 0.f;
        Vec2<int> ret;
        for (auto it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            const float dist = it->second.DistanceToSqr(m_instance.m_lastOffPos);
            if (dist > maxDist)
            {
                maxDist = dist;
                ret = it->second.GetPos();
            }
        }
        return ret;
    }
}