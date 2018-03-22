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
        m_mesh.Free();
    }

    void ChunkManager::Init()
    {
        std::string result;
        m_shader = VTerrain::Shaders::CompileShader(nullptr, nullptr, nullptr, nullptr, result);
        assert(m_shader.m_program != 0);

        m_mesh.Generate();
    }

    void ChunkManager::Update(int posX, int posY)
    {
        const int W = static_cast<int>(config.chunkSize);
        Vec2<int> off(
			(int)floor((posX - floor(W / 2.f) + (W % 2 != 0)) / W) + 1,
            (int)floor((posY - floor(W / 2.f) + (W % 2 != 0)) / W) + 1
        );

        while (m_factory.HasGeneratedChunks())
        {
            GetFurthestChunk().Regenerate(m_factory.PopGeneratedChunk());
        }

        if (off != m_lastOffPos)
        {
			m_factory.ClearRequests();
            m_lastOffPos = off;
            AddChunksToRegen(off);
        }
    }

    void ChunkManager::Render(const float * viewMatrix, const float * projectionMatrix) const
    {
        glPatchParameteri(GL_PATCH_VERTICES, 4);

        glUseProgram(m_shader.m_program);

        glBindBuffer(GL_ARRAY_BUFFER, m_mesh.GetMeshBuf());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh.GetIndicesBuf());

        //Vertices
        glVertexAttribPointer(m_shader.attrib_vertex, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
        glEnableVertexAttribArray(m_shader.attrib_vertex);

        //UVs
        glVertexAttribPointer(m_shader.attrib_UV, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
        glEnableVertexAttribArray(m_shader.attrib_UV);

        // ------ Setting uniforms -------------------------
        glUniformMatrix4fv(m_shader.loc_view_matrix, 1, GL_FALSE, viewMatrix);

        //Projection Matrix
        glUniformMatrix4fv(m_shader.loc_projection_matrix, 1, GL_FALSE, projectionMatrix);

        glUniform1f(m_shader.loc_ambient_color, config.ambientLight);


        //Global light direction
        Vec3<float> dir(config.globalLight[0], config.globalLight[1], config.globalLight[2]);
        dir.Normalize();
        glUniform3fv(m_shader.loc_global_light_direction, 1, dir.Data());

        glUniform1f(m_shader.loc_max_height, config.maxHeight);

        glUniform1f(m_shader.loc_fog_distance, config.fogDistance);

        glUniform3fv(m_shader.loc_fog_color, 1, config.fogColor);

        glUniform3fv(m_shader.loc_water_color, 1, config.waterColor);

        glUniform1f(m_shader.loc_water_height, config.waterHeight);

        glUniform1ui(m_shader.loc_maxLOD, config.nLODs);
        glUniform1f(m_shader.loc_LODdistance, config.LODdistance);

        glUniform1i(m_shader.loc_render_chunk_borders, config.debug.renderChunkBorders);
        glUniform1i(m_shader.loc_render_heightmap, config.debug.renderHeightmap);
        glUniform1i(m_shader.loc_render_light, config.debug.renderLight);

        glUniform1i(m_shader.loc_heightmap, 0);
        for (int n = 0; n < 10; n++)
        {
            glUniform1i(m_shader.textures[n].loc_diffuse, (n * 2 + 1));

            glUniform1i(m_shader.textures[n].loc_heightmap, (n * 2 + 2));

			glUniform1fv(m_shader.textures[n].data, 9, m_textures[n].Get());

            glActiveTexture(GL_TEXTURE1 + n * 2);
            glBindTexture(GL_TEXTURE_2D, m_textures[n].buf_diffuse);

            glActiveTexture(GL_TEXTURE1 + n * 2 + 1);
            glBindTexture(GL_TEXTURE_2D, m_textures[n].buf_heightmap);
        }

        if (config.debug.wiredRender == false)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        if (config.singleSidedFaces)
        {
            glEnable(GL_CULL_FACE);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }

        for (auto it = m_chunks.begin(); it != m_chunks.end(); it++)
        {
            it->Draw(m_shader);
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUseProgram(0);
    }

    void ChunkManager::CleanChunks()
    {
        for (auto it = m_chunks.begin(); it != m_chunks.end(); it++)
        {
            it->Free();
        }
        m_factory.ClearRequests();
        AddChunksToRegen(m_lastOffPos);
    }

    void ChunkManager::SetHeightCurve(std::function<float(float)> func)
    {
        m_factory.SetHeightCurve(func);
        CleanChunks();
    }

    void ChunkManager::SetSeed(uint seed)
    {
        m_factory.SetSeed(seed);
        CleanChunks();
    }

    void ChunkManager::AddChunksToRegen(Vec2<int> pos)
    {
		//TODO draw a circle instead of a square
		int maxDist = 0;
		uint nChunks = 1;
		while (nChunks < config.maxChunks)
		{
			nChunks += ++maxDist * 4;
		}
		maxDist --;
		for (int dist = 0; dist <= maxDist; dist++)
		{
			for (int y = -dist; y <= dist; y++)
			{
				for (int x = -dist; x <= dist; x++)
				{
					if (abs(x) + abs(y) == dist)
					{
						AddChunkToRegen(Vec2<int>(pos.x() + x, pos.y() + y));
					}
				}
			}
		}
    }

	bool ChunkManager::AddChunkToRegen(Vec2<int> pos)
    {
        if (IsLoaded(pos) == false && m_factory.IsRequested(pos) == false)
        {
            m_factory.PushChunkRequest(pos);
			return true;
        }
		return false;
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
        return *m_chunks.begin();
    }

	const Chunk & ChunkManager::GetChunk(Vec2<int> pos) const
	{
		for (std::vector<Chunk>::const_iterator it = m_chunks.cbegin(); it != m_chunks.cend(); it++)
		{
			if (it->GetPos() == pos)
			{
				return *it;
			}
		}
		assert(false);
		return *m_chunks.cbegin();
	}

    bool ChunkManager::IsLoaded(Vec2<int> pos) const
	{
        for (std::vector<Chunk>::const_iterator it = m_chunks.cbegin(); it != m_chunks.cend(); it++)
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