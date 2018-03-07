//  V Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hern�ndez L�zaro
//  
//  "V Terrains" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or any later version.
//  
//  "V Terrains" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//  
//  For more details, read "COPYING.txt" and "COPYING.LESSER.txt" included in this project.
//  You should have received a copy of the GNU General Public License along with V Terrains.  If not, see <http://www.gnu.org/licenses/>.
#include "ChunkManager.h"

#include "ChunkMesh.h"
#include "../Utils/GenImage.h"
#include "../ExternalLibs/Glew/include/glew.h"


namespace VTerrain
{
    ChunkManager::ChunkManager()
		: m_chunks()
		, m_lastOffPos(0, 0)
        , m_currentChunk(INT_MAX, INT_MIN)
		, m_factory()
    {
        m_chunks.resize(config.maxChunks);
    }

    ChunkManager::~ChunkManager()
    {
        Chunk::m_mesh.Free();
    }

    void ChunkManager::Update(int posX, int posY)
    {
        const int W = static_cast<int>(config.chunkWidth * config.quadSize);
        const int H = static_cast<int>(config.chunkHeight * config.quadSize);
        Vec2<int> off(
			(int)floor((posX - floor(W / 2.f) + (W % 2 != 0)) / W) + 1,
            (int)floor((posY - floor(H / 2.f) + (H % 2 != 0)) / H) + 1
        );
        

        while (m_factory.HasGeneratedChunks())
        {
            GetFurthestChunk().Regenerate(m_factory.PopGeneratedChunk());
        }

        if (m_factory.HasRequestedChunks() == false)
        {
            int dist = 0;
            //Generate chunks when stopped
            //TODO
        }

        if (off != m_lastOffPos)
        {
            m_lastOffPos = off;
            AddChunksToRegen(off);
        }
    }

    void ChunkManager::Render(const float * viewMatrix, const float * projectionMatrix)
    {
        for (auto it = m_chunks.begin(); it != m_chunks.end(); it++)
        {
            //TODO improve set LOD
            it->Draw(viewMatrix, projectionMatrix, (m_lastOffPos - it->GetPos()).Length() / 2);
        }
    }

    void ChunkManager::CleanChunks()
    {
        for (auto it = m_chunks.begin(); it != m_chunks.end(); it++)
        {
            it->Free();
        }
        AddChunksToRegen(m_lastOffPos);
    }

    void ChunkManager::AddChunksToRegen(Vec2<int> pos)
    {
        //TODO configurable
        for (int y = -6; y <= 6; y++)
        {
            for (int x = -6; x <= 6; x++)
            {
                AddChunkToRegen(Vec2<int>(pos.x() + x, pos.y() + y));
            }
        }
    }

    void ChunkManager::AddChunkToRegen(Vec2<int> pos)
    {
        if (IsLoaded(pos) == false && m_factory.IsRequested(pos) == false)
        {
            m_factory.PushChunkRequest(pos);
        }
    }


    Chunk & ChunkManager::GetChunk(Vec2<int> pos)
    {
        for (std::vector<Chunk>::iterator it = m_chunks.begin(); it != m_chunks.end(); it++)
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
        for (std::vector<Chunk>::iterator it = m_chunks.begin(); it != m_chunks.end(); it++)
        {
            if (it->IsLoaded() && it->GetPos() == pos)
            {
                return true;
            }
        }
        return false;
	}

    Chunk& ChunkManager::GetFurthestChunk()
    {
        //TODO optimize
        Chunk* ret = nullptr;
        float maxDist = 0.f;
        for (auto it = m_chunks.begin(); it != m_chunks.end(); it++)
        {
            if (it->IsLoaded() == false)
            {
                return *it;
            }

            const float dist = (it->GetPos() - m_lastOffPos).LengthSqr();
            if (dist > maxDist)
            {
                maxDist = dist;
                ret = &(*it);
            }
        }
        assert(ret != nullptr);
        return *ret;
    }
}