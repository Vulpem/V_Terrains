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
#include "../ExternalLibs/Glew/include/glew.h"
#include "../Mesh/MeshGenerator.h"

namespace VTerrain
{
    ChunkManager ChunkManager::m_instance = ChunkManager();

    uint ChunkManager::Chunk::m_shaderProgram = 0u;

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
        if (IsLoaded())
        {
            uint drawLOD = LOD;
            if (IsLODReady(LOD) == false)
            {
                drawLOD = m_minLOD;
            }

            glUseProgram(m_shaderProgram);

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_buf_heightmap);

            // ------ Setting uniforms -------------------------
            //Model Matrix
            float identity[] = { 1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                0,0,0,1 };
            GLint modelLoc = glGetUniformLocation(m_shaderProgram, "model_matrix");
            if (modelLoc != -1) { glUniformMatrix4fv(modelLoc, 1, GL_FALSE, identity); }

            //View matrix
            GLint viewLoc = glGetUniformLocation(m_shaderProgram, "view_matrix");
            if (viewLoc != -1)
            {
                glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix);
            }

            //Projection Matrix
            GLint projectionLoc = glGetUniformLocation(m_shaderProgram, "projection_matrix");
            if (projectionLoc != -1)
            {
                glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projectionMatrix);
            }

            GLint offsetLoc = glGetUniformLocation(m_shaderProgram, "position_offset");
            if (offsetLoc != -1)
            {
                float tmp[3] = { m_pos.x() * (float)Config::chunkWidth, 0.f, m_pos.y() * (float)Config::chunkHeight };
                glUniform3fv(offsetLoc, 1, tmp);
            }

            float col[] = { 0.5f, 0.5f, 0.5f, 1.0f };
            //Ambient color
            GLint ambientColorLoc = glGetUniformLocation(m_shaderProgram, "ambient_color");
            if (ambientColorLoc != -1)
            {
                glUniform4fv(ambientColorLoc, 1, col);
            }

            float materialCol[] = { 1.0f,1.0f,1.0f,1.0f };
            //Material color
            GLint materialColorLoc = glGetUniformLocation(m_shaderProgram, "material_color");
            if (materialColorLoc != -1)
            {
                glUniform4fv(materialColorLoc, 1, materialCol);
            }


            Vec3<float> dir(Config::globalLight[0], Config::globalLight[1], Config::globalLight[2]);
            dir.Normalize();
            //Global light direction
            GLint globalLightDirLoc = glGetUniformLocation(m_shaderProgram, "global_light_direction");
            if (globalLightDirLoc != -1)
            {
                glUniform3fv(globalLightDirLoc, 1, dir.Data());
            }

            GLint maxHeightLoc = glGetUniformLocation(m_shaderProgram, "max_height");
            if (maxHeightLoc != -1)
            {
                glUniform1f(maxHeightLoc, Config::maxHeight);
            }

            glBindBuffer(GL_ARRAY_BUFFER, MeshGenerator::Mesh::GetMeshBuf());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MeshGenerator::Mesh::GetIndicesBuf(LOD));

            //Vertices
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
            glEnableVertexAttribArray(0);

            //UVs
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawElements(GL_TRIANGLES, MeshGenerator::Mesh::GetNumIndices(LOD), GL_UNSIGNED_INT, (void*)0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glPopMatrix();

            glBindTexture(GL_TEXTURE_2D, 0);
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            glUseProgram(0);
        }
    }

    bool ChunkManager::Chunk::IsLODReady(uint LOD)
    {
        return (LOD >= m_minLOD);
    }

    bool ChunkManager::Chunk::IsLoaded()
    {
        return (m_minLOD != UINT_MAX);
    }

    ChunkManager::ChunkManager()
		: m_chunks()
		, m_lastOffPos(0, 0)
        , m_currentChunk(0, 0)
        , m_firstFrame(true)
		, m_factory()
    {
        m_chunks.resize(Config::maxChunks);
    }

    void ChunkManager::Update(int posX, int posY)
    {
        const int W = static_cast<int>(Config::chunkWidth);
        const int H = static_cast<int>(Config::chunkHeight);
        Vec2<int> off(
			(int)floor((posX - floor(W / 2.f) + (W % 2 != 0)) / W) + 1,
            (int)floor((posY - floor(H / 2.f) + (H % 2 != 0)) / H) + 1
        );
        
        while (m_instance.m_factory.HasGeneratedChunks())
        {
            GetFurthestChunk().Regenerate(m_instance.m_factory.PopGeneratedChunk());
        }

        if (off != m_instance.m_lastOffPos || m_instance.m_firstFrame)
        {
            m_instance.m_lastOffPos = off;
            AddChunksToRegen(off);
        }

        if (m_instance.m_firstFrame)
        {
            MeshGenerator::Mesh::Generate();
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
        if (IsLoaded(pos) == false && m_instance.m_factory.IsRequested(pos) == false)
        {
            m_instance.m_factory.PushChunkRequest(pos);
        }
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
            if (it->IsLoaded() && it->GetPos() == pos)
            {
                return true;
            }
        }
        return false;
	}

    ChunkManager::Chunk& ChunkManager::GetFurthestChunk()
    {
        //TODO optimize
        Chunk* ret = nullptr;
        float maxDist = 0.f;
        for (auto it = m_instance.m_chunks.begin(); it != m_instance.m_chunks.end(); it++)
        {
            if (it->IsLoaded() == false)
            {
                return *it;
            }

            const float dist = (it->GetPos() - m_instance.m_lastOffPos).LengthSqr();
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