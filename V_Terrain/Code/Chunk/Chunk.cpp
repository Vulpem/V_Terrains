#include "Chunk.h"
#include <math.h>

namespace VTerrain
{
    ChunkManager ChunkManager::m_instance = ChunkManager();

    ChunkManager::Chunk::Chunk() :
        m_LOD(0u)
        , m_offset(0,0)
    {}

    void ChunkManager::Chunk::Regenerate(uint gen, uint LOD)
    {
        Regenerate(gen, LOD, m_offset);
    }

    void ChunkManager::Chunk::Regenerate(uint gen, uint LOD, Vec2<int> offset)
    {
        m_generation = utils::Max(m_generation, gen);
		if (LOD != m_LOD)
		{
            m_offset = offset;
			m_LOD = LOD;

			VTerrain::PerlinNoise::NoiseMap m_noiseMap = VTerrain::PerlinNoise::GenNoiseMap(offset);
			VTerrain::MeshGenerator::MeshData meshData;
			meshData.Generate(m_noiseMap);
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

    ChunkManager::ChunkManager() :
        m_lastOffPos(-1000, -1000)
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
        if (off != m_instance.m_lastOffPos)
        {
            m_instance.m_lastOffPos = off;
            for (int y = -1; y <= 1; y++)
            {
                for (int x = -1; x <= 1; x++)
                {
                    AddChunkToRegen(m_instance.m_chunkGeneration, 1, Vec2<int>(off.x() + x, off.y() + y));
                }
            }
            m_instance.m_chunkGeneration++;
        }

        RegenChunk();
    }

    void ChunkManager::Render(const float * viewMatrix, const float * projectionMatrix)
    {
        for (auto it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            it->second.Draw(viewMatrix, projectionMatrix);
        }
    }

    void ChunkManager::FreeChunk()
    {
    }

    void ChunkManager::RegenChunk()
    {
        for (auto it = m_instance.m_chunkstoRegen.begin(); it != m_instance.m_chunkstoRegen.end(); it++)
        {
            if (it->second.empty() == false)
            {
                const uint gen = it->second.front().second;
                const Vec2<int> offset = it->second.front().first;
                if (IsLoaded(offset) == false || gen > m_instance.m_chunks[offset].GetGen())
                {
                    const uint LOD = it->first;
                    m_instance.m_chunks[offset].Regenerate(gen, LOD, offset);
                    return;
                }
                it->second.pop_front();
            }
        }
    }

    void ChunkManager::AddChunkToRegen(uint gen, uint LOD, Vec2<int> pos)
    {
        m_instance.m_chunkstoRegen[LOD].push_back({ pos, gen });
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
}