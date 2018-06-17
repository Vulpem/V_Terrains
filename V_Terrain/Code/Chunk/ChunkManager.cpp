//  RPG Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hernàndez Làzaro
//  
//  "RPG Terrains" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or any later version.
//  
//  "RPG Terrains" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//  
//  For more details, read "COPYING.txt" and "COPYING.LESSER.txt" included in this project.
//  You should have received a copy of the GNU General Public License along with RPG Terrains.  If not, see <http://www.gnu.org/licenses/>.
#include "ChunkManager.h"

#include "ChunkMesh.h"
#include "../Utils/GenImage.h"
#include "../ExternalLibs/Glew/include/glew.h"


namespace RPGT
{
    ChunkManager::ChunkManager()
		: m_chunks()
		, m_lastOffPos(0, 0)
        , m_currentChunk(INT_MAX, INT_MIN)
		, m_factory()
        , m_isInit(false)
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
        m_shader = RPGT::Shaders::CompileShader(nullptr, nullptr, nullptr, nullptr, result);
		ASSERT(m_shader.m_program != 0, "Error compiling shaders:\n%s", result.data());

        GenerateMesh();
        m_isInit = true;
    }

    void ChunkManager::Update(int posX, int posY)
    {
        ASSERT(m_isInit, "Called Update before Init");

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
		ASSERT(m_isInit, "Called Render before Init");

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

        glUniform1f(m_shader.loc_fog_distance, config.fogDistance);

        glUniform3fv(m_shader.loc_fog_color, 1, config.fogColor);

        glUniform1f(m_shader.loc_water_height, config.waterHeight);

        glUniform1i(m_shader.loc_render_chunk_borders, config.debug.renderChunkBorders);
        glUniform1i(m_shader.loc_render_heightmap, config.debug.renderHeightmap);
        glUniform1i(m_shader.loc_render_light, config.debug.renderLight);

        for (int n = 0; n < 10; n++)
        {
            glUniform1i(m_shader.textures[n].loc_diffuse, (n * 2 + 1));

            glUniform1i(m_shader.textures[n].loc_heightmap, (n * 2 + 2));

			glUniform1fv(m_shader.textures[n].data, 11, m_textures[n].Get());

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

        const float r[9] = {    viewMatrix[0], viewMatrix[1], viewMatrix[2],
                                viewMatrix[4], viewMatrix[5], viewMatrix[6],
                                viewMatrix[8], viewMatrix[9], viewMatrix[10]};
        const float p[3] = {    viewMatrix[12], viewMatrix[13], viewMatrix[14] };


        Vec3<float> cameraPos = Vec3<float>(
            p[0] * r[0] + p[1] * r[1] + p[2] * r[2],
            p[0] * r[3] + p[1] * r[4] + p[2] * r[5],
            p[0] * r[6] + p[1] * r[7] + p[2] * r[8]
            );

        glUniform1i(m_shader.loc_heightmap, 0);
        const uint nIndices = m_mesh.GetNIndices();
        std::for_each(m_chunks.begin(), m_chunks.end(),
            [=](const Chunk& chunk)
        {
            if (chunk.IsLoaded())
            {
                chunk.BindHeightmap(0);
                chunk.BindModelMatrix(m_shader.loc_model_matrix);
                glDrawElements(GL_PATCHES, nIndices, GL_UNSIGNED_INT, (void*)0);
            }
        });

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUseProgram(0);
    }

    void ChunkManager::CleanChunks()
    {
        std::for_each(m_chunks.begin(), m_chunks.end(), [](Chunk& chunk) { chunk.Free(); });
        m_factory.ClearRequests();
        AddChunksToRegen(m_lastOffPos);
    }

    void ChunkManager::SetSeed(uint seed)
    {
        m_factory.SetSeed(seed);
        CleanChunks();
    }

    void ChunkManager::GenerateMesh()
    {
        m_mesh.Free();
        m_mesh.Generate();
    }

    void ChunkManager::GetPoint(float x, float y, float & height, float * normal) const
    {
        const int HMresolution = static_cast<int>(RPGT::config.chunkSize);
        Vec2<int> currentChunk(
            floor((x - floor(HMresolution / 2.f) + (HMresolution % 2 != 0)) / HMresolution + 1),
            floor((y - floor(HMresolution / 2.f) + (HMresolution % 2 != 0)) / HMresolution + 1)
        );
        auto chunk = std::find_if(m_chunks.begin(), m_chunks.end(),
            [currentChunk](const Chunk& c)
		{ const Vec2<int> p = c.GetPos(); return (p.x() == currentChunk.x() && p.y() == currentChunk.y()); });

        if (chunk != m_chunks.end())
        {
            chunk->GetPoint(x - currentChunk.x() * config.chunkSize, y - currentChunk.y() * config.chunkSize, height, normal);
        }
        else
        {
            height = -1.f;
            normal[0] = 0.f;
            normal[1] = 0.f;
            normal[2] = 0.f;
        }
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

        std::vector<Vec2<int>> toAdd;
        toAdd.reserve(config.maxChunks);

		AddChunkToRegen(Vec2<int>(pos.x(), pos.y()));
		for (int dist = 1; dist < maxDist; dist++)
		{
			for (int x = 0; x < dist; x++)
			{
				const int y = dist - x;
				AddChunkToRegen(Vec2<int>(pos.x() + x, pos.y() + y));
				AddChunkToRegen(Vec2<int>(pos.x() - x - 1, pos.y() + y - 1));
				AddChunkToRegen(Vec2<int>(pos.x() + x + 1, pos.y() - y + 1));
				AddChunkToRegen(Vec2<int>(pos.x() - x, pos.y() - y));
			}
		}
    }

	bool ChunkManager::AddChunkToRegen(Vec2<int> pos)
    {
        if (IsLoaded(pos) == false)
        {
            m_factory.PushChunkRequest(pos);
			return true;
        }
		return false;
    }


    Chunk & ChunkManager::GetChunk(Vec2<int> pos)
    {
        auto it = std::find_if(m_chunks.begin(), m_chunks.end(), [pos](const Chunk& chunk) { return chunk.GetPos() == pos; });
        if (it != m_chunks.end())
        {
                return *it;
        }
        ASSERT(false, "Tried to access a non-existing chunk at pos %i, %i", pos.x(), pos.y());
        return *m_chunks.begin();
    }

	const Chunk & ChunkManager::GetChunk(Vec2<int> pos) const
	{
        auto it = std::find_if(m_chunks.begin(), m_chunks.end(), [pos](const Chunk& chunk) { return chunk.GetPos() == pos; });
        if (it != m_chunks.end())
        {
            return *it;
        }
		ASSERT(false, "Tried to access a non-existing chunk at pos %i, %i", pos.x(), pos.y());
		return *m_chunks.cbegin();
	}

    bool ChunkManager::IsLoaded(Vec2<int> pos) const
	{
        auto it = std::find_if(m_chunks.begin(), m_chunks.end(), [pos](const Chunk& chunk) { return chunk.GetPos() == pos; });
		return (it != m_chunks.end() && it->IsLoaded());
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
        ASSERT(ret != nullptr, "This function just straight up broke.");
        return *ret;
    }
}