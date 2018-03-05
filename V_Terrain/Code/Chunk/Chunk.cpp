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
#include "../Utils/GenImage.h"
#include <math.h>

namespace VTerrain
{
    ChunkManager ChunkManager::m_instance = ChunkManager();

    ChunkManager::Chunk::Chunk() :
        m_minLOD(UINT_MAX)
        , m_buf_heightmap(0u)
    {
    }

    void ChunkManager::Chunk::Regenerate(ChunkFactory::GeneratedChunk base)
    {
        m_buf_heightmap = GenImage::FromRGBA(base.m_data, base.m_size.x(), base.m_size.y());
        m_minLOD = base.m_LOD;
        m_pos = base.m_pos;
    }

    void ChunkManager::Chunk::Free()
    {
        GenImage::FreeImage(m_buf_heightmap);
        m_minLOD = UINT_MAX;
    }

    void ChunkManager::Chunk::Draw(const float* viewMatrix, const float* projectionMatrix, uint LOD)
    {
        if (m_minLOD != UINT_MAX)
        {
            uint drawLOD = LOD;
            if (IsLODReady(LOD) == false)
            {
                drawLOD = m_minLOD;
            }
            m_mesh.Render(viewMatrix, projectionMatrix, Vec3<int>(pos.x() * (Config::chunkWidth - 1), 0u, pos.y() * (Config::chunkHeight - 1)), LOD);
        }
    }

    bool ChunkManager::Chunk::IsLODReady(uint LOD)
    {
        return (LOD >= m_minLOD);
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
            //TODO set LOD
            //(m_instance.m_lastOffPos - it->first).Length();
            it->Draw(viewMatrix, projectionMatrix, Config::TMP::LOD);
        }
    }

    void ChunkManager::CleanChunks()
    {
        for (auto it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            it->Free();
        }
        AddChunksToRegen(m_instance.m_lastOffPos);
    }

    void ChunkManager::RegenAll()
    {
        for (auto it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            AddChunkToForceRegen(it->GetPos());
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
        if (IsLoaded(pos) == false)
        {
            m_instance.m_factory.PushChunkRequest(pos);
        }
    }

    void ChunkManager::AddChunkToForceRegen(Vec2<int> pos)
    {
        m_instance.m_factory.PushChunkRequest(pos);
    }

    ChunkManager::Chunk & ChunkManager::GetChunk(Vec2<int> pos)
    {
        for (std::vector<Chunk>::iterator it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            if (it->GetPos() == pos)
            {
                return *it;
            }
        }
        assert(false);
        Chunk a;
        return a;
    }

    bool ChunkManager::IsLoaded(Vec2<int> pos)
	{
        for (std::vector<Chunk>::iterator it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            if (it->GetPos() == pos)
            {
                return true;
            }
        }
        return false;
	}

    Vec2<int> ChunkManager::GetFurthestChunk()
    {
        //TODO optimize
        float maxDist = 0.f;
        Vec2<int> ret;
        for (auto it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            const float dist = (it->GetPos() - m_instance.m_lastOffPos).LengthSqr();
            if (dist > maxDist)
            {
                maxDist = dist;
                ret = it->GetPos();
            }
        }
        return ret;
    }
}