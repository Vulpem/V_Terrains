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
#include "Chunk.h"

#include "ChunkMesh.h"
#include "../Utils/GenImage.h"
#include "../ExternalLibs/Glew/include/glew.h"


namespace VTerrain
{
    Shader Chunk::m_shader = Shader();
	Mesh Chunk::m_mesh = Mesh();

     Chunk::Chunk() :
        m_minLOD(UINT_MAX)
        , m_buf_heightmap(0u)
    {
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
        if (IsLoaded())
        {
            uint drawLOD = LOD;
            if (IsLODReady(LOD) == false)
            {
                drawLOD = m_minLOD;
            }
            if (drawLOD >= config.nLODs)
            {
                drawLOD = config.nLODs - 1;
            }

            glUseProgram(m_shader.m_program);

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_buf_heightmap);

            // ------ Setting uniforms -------------------------

            glUniformMatrix4fv(m_shader.loc_view_matrix, 1, GL_FALSE, viewMatrix);

            //Projection Matrix
            glUniformMatrix4fv(m_shader.loc_projection_matrix, 1, GL_FALSE, projectionMatrix);

            const float tmp[3] = { m_pos.x() * ((float)config.chunkWidth * config.quadSize), 0.f, m_pos.y() * ((float)config.chunkHeight * config.quadSize) };
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

            glUniform1i(m_shader.loc_render_chunk_borders, config.tmp.renderChunkBorders);
            glUniform1i(m_shader.loc_render_heightmap, config.tmp.renderHeightmap);



            glBindBuffer(GL_ARRAY_BUFFER, m_mesh.GetMeshBuf());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mesh.GetIndicesBuf(drawLOD));

            //Vertices
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
            glEnableVertexAttribArray(0);

            //UVs
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawElements(GL_TRIANGLES, m_mesh.GetNumIndices(drawLOD), GL_UNSIGNED_INT, (void*)0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glPopMatrix();

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