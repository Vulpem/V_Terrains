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
#include "Chunk.h"
#include <math.h>

namespace VTerrain
{
    ChunkManager ChunkManager::m_instance = ChunkManager();

    ChunkManager::Chunk::Chunk() :
        m_maxLOD(UINT_MAX)
    {}

    void ChunkManager::Chunk::Regenerate(Vec2<int> offset)
    {
        VTerrain::PerlinNoise::NoiseMap m_noiseMap = VTerrain::PerlinNoise::GenNoiseMap(offset); //46/60ms
        VTerrain::MeshGenerator::MeshData meshData;
        meshData.GenerateData(m_noiseMap); //251/150ms
        m_mesh.Generate(meshData);
        m_maxLOD = 0;
    }

    void ChunkManager::Chunk::Free()
    {
        m_mesh.FreeMesh();
    }

    void ChunkManager::Chunk::Draw(const float* viewMatrix, const float* projectionMatrix, Vec2<int> pos, uint LOD)
    {
        if (IsLODReady(LOD))
        {
            m_mesh.Render(viewMatrix, projectionMatrix, Vec3<int>(pos.x() * (Config::chunkWidth - 1),0u, pos.y() * (Config::chunkHeight - 1)), LOD);
        }
    }

    bool ChunkManager::Chunk::IsLODReady(uint LOD)
    {
        return (LOD >= m_maxLOD);
    }

    ChunkManager::ChunkManager() :
        m_lastOffPos(0, 0)
        , m_currentChunk(0, 0)
        , m_firstFrame(true)
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
        
        if (off != m_instance.m_lastOffPos || m_instance.m_firstFrame)
        {
            m_instance.m_lastOffPos = off;
            AddChunksToRegen(off);
        }

        RegenChunk();
        if (m_instance.m_chunks.size() > Config::maxChunks)
        {
            FreeChunk();
        }

        if (m_instance.m_firstFrame)
        {
            for (int n = 0; n < Config::nLODs; n++)
            {
                MeshGenerator::Mesh::GenerateIndices(Config::chunkWidth, Config::chunkHeight, n);
            }
            m_instance.m_firstFrame = false;
        }
    }

    void ChunkManager::Render(const float * viewMatrix, const float * projectionMatrix)
    {
        for (auto it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            it->second.Draw(viewMatrix, projectionMatrix, it->first, (it->first != m_instance.m_lastOffPos && false));
        }
    }

    void ChunkManager::CleanChunks()
    {
        for (auto it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            it->second.Free();
        }
        m_instance.m_chunks.clear();
        while (m_instance.m_chunkstoRegen.empty() == false) { m_instance.m_chunkstoRegen.pop(); }
        while (m_instance.m_chunkstoForceRegen.empty() == false) { m_instance.m_chunkstoForceRegen.pop(); }
        AddChunksToRegen(m_instance.m_lastOffPos);
    }

    void ChunkManager::RegenAll()
    {
        for (auto it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            AddChunkToForceRegen(it->first);
        }
    }

    void ChunkManager::FreeChunk()
    {
        m_instance.m_chunks.erase(GetFurthestChunk());
    }

    void ChunkManager::RegenChunk()
    {
        Vec2<int> pos;
        bool found = false;
        if (m_instance.m_chunkstoForceRegen.empty() == false)
        {
            pos = m_instance.m_chunkstoForceRegen.front();
            m_instance.m_chunkstoForceRegen.pop();
            found = true;
        }

        if (found == false && m_instance.m_chunkstoRegen.empty() == false)
        {
            pos = m_instance.m_chunkstoRegen.front();
            m_instance.m_chunkstoRegen.pop();
            found = (m_instance.m_chunks.find(pos) == m_instance.m_chunks.end());
        }

        if (found)
        {
            m_instance.m_chunks[pos].Regenerate(pos);
        }
    }

    void ChunkManager::AddChunksToRegen(Vec2<int> pos)
    {
        //TODO configurable
        for (int y = -2; y <= 2; y++)
        {
            for (int x = -2; x <= 2; x++)
            {
                AddChunkToRegen(Vec2<int>(pos.x() + x, pos.y() + y));
            }
        }
    }

    void ChunkManager::AddChunkToRegen(Vec2<int> pos)
    {
        m_instance.m_chunkstoRegen.push(pos);
    }

    void ChunkManager::AddChunkToForceRegen(Vec2<int> pos)
    {
        m_instance.m_chunkstoForceRegen.push(pos);
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
            const float dist = (it->first - m_instance.m_lastOffPos).LengthSqr();
            if (dist > maxDist)
            {
                maxDist = dist;
                ret = it->first;
            }
        }
        return ret;
    }
}