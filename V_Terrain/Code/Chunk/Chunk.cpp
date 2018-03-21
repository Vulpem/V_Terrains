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
#include "Chunk.h"

#include "ChunkMesh.h"
#include "../Utils/GenImage.h"
#include "../ExternalLibs/Glew/include/glew.h"

namespace VTerrain
{
    Shader Chunk::m_shader = Shader();
	Mesh Chunk::m_mesh = Mesh();
	ConditionalTexture Chunk::m_textures[10];

     Chunk::Chunk() :
        m_minLOD(UINT_MAX)
        , m_buf_heightmap(0u)
    {

		 m_textures[0].color = { 1.f,0.f,1.f };
		 m_textures[0].minHeight = 0.f;
		 m_textures[0].maxHeight = 100.f;
		 m_textures[0].minSlope = 0.f;
		 m_textures[0].maxSlope = 1.f;

		 m_textures[1].color = { 0.f,0.f,1.f };
		 m_textures[1].minHeight = 0.f;
		 m_textures[1].maxHeight = 10000.f;
		 m_textures[1].minSlope = 0.f;
		 m_textures[1].maxSlope = 0.5f;

		 m_textures[2].color = { 0.f,1.f,0.f };
		 m_textures[2].minHeight = 0.f;
		 m_textures[2].maxHeight = 10000.f;
		 m_textures[2].minSlope = 0.f;
		 m_textures[2].maxSlope = 1.f;

    }

    void  Chunk::Regenerate(ChunkFactory::GeneratedChunk base)
    {
		if (m_buf_heightmap != 0)
		{
			Free();
		}

        m_buf_heightmap = GenImage::FromRGBA(base.m_data, base.m_size.x(), base.m_size.y());
        m_minLOD = base.m_LOD;
        m_pos = base.m_pos;
    }

    void  Chunk::Free()
    {
		if (m_buf_heightmap != 0)
		{
			GenImage::FreeImage(m_buf_heightmap);
		}
        m_minLOD = UINT_MAX;
    }

    void  Chunk::Draw(const float* viewMatrix, const float* projectionMatrix, uint LOD) const
    {
		//TODO investigate tesselation
        if (IsLoaded())
        {
            uint drawLOD = 0;
            if (IsLODReady(LOD) == false)
            {
                drawLOD = m_minLOD;
            }
            if (drawLOD >= config.nLODs)
            {
                drawLOD = config.nLODs - 1;
            }

            //

            glPatchParameteri(GL_PATCH_VERTICES, (2) * (2));
            glPatchParameteri(GL_PATCH_DEFAULT_OUTER_LEVEL, LOD);
            glPatchParameteri(GL_PATCH_DEFAULT_INNER_LEVEL, LOD);
            //

            glUseProgram(m_shader.m_program);

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnable(GL_TEXTURE_2D);

            // ------ Setting uniforms -------------------------

            glUniformMatrix4fv(m_shader.loc_view_matrix, 1, GL_FALSE, viewMatrix);

            //Projection Matrix
            glUniformMatrix4fv(m_shader.loc_projection_matrix, 1, GL_FALSE, projectionMatrix);

            const float tmp[3] = { m_pos.x() * config.chunkSize, 0.f, m_pos.y() * config.chunkSize };
            glUniform3fv(m_shader.loc_position_offset, 1, tmp);

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
			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D, m_buf_heightmap);

			for (int n = 0; n < 10; n++)
			{
				glUniform1i(m_shader.textures[n].loc_diffuse, n * 2 + 1);
				glActiveTexture(GL_TEXTURE0 + n*2 + 1);
				glBindTexture(GL_TEXTURE_2D, m_textures[n].buf_diffuse);

				glUniform1i(m_shader.textures[n].loc_heightmap, n * 2 + 2);
				glActiveTexture(GL_TEXTURE0 + n * 2 + 2);
				glBindTexture(GL_TEXTURE_2D, m_textures[n].buf_heightmap);

				glUniform3fv(m_shader.textures[n].loc_color, 1, m_textures[n].color.Data());
				glUniform1f(m_shader.textures[n].loc_minSlope, m_textures[n].minSlope);
				glUniform1f(m_shader.textures[n].loc_maxSlope, m_textures[n].maxSlope);
				glUniform1f(m_shader.textures[n].loc_minHeight, m_textures[n].minHeight);
				glUniform1f(m_shader.textures[n].loc_maxHeight, m_textures[n].maxHeight);
			}

            glBindBuffer(GL_ARRAY_BUFFER, m_mesh.GetMeshBuf());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh.GetIndicesBuf());

            //Vertices
            glVertexAttribPointer(m_shader.attrib_vertex, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
            glEnableVertexAttribArray(m_shader.attrib_vertex);

            //UVs
            glVertexAttribPointer(m_shader.attrib_UV, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
            glEnableVertexAttribArray(m_shader.attrib_UV);

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

            glDrawElements(GL_PATCHES, m_mesh.GetNumIndices(), GL_UNSIGNED_INT, (void*)0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glPopMatrix();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            glUseProgram(0);
        }
    }

    bool  Chunk::IsLODReady(uint LOD)  const
    {
        return (LOD >= m_minLOD);
    }

    bool  Chunk::IsLoaded() const
    {
        return (m_minLOD != UINT_MAX);
    }

	Vec2<int> Chunk::GetPos() const
	{
		return m_pos;
	}
}