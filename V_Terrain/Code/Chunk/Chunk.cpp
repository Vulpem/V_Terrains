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
    uint Chunk::m_shaderProgram = 0u;
	Mesh Chunk::m_mesh = Mesh();

     Chunk::Chunk() :
        m_minLOD(UINT_MAX)
        , m_buf_heightmap(0u)
    {
    }

    void  Chunk::Regenerate(ChunkFactory::GeneratedChunk base)
    {
        if (m_mesh.IsGenerated() == false)
        {
            m_mesh.Generate();
        }
        m_buf_heightmap = GenImage::FromRGBA(base.m_data, base.m_size.x(), base.m_size.y());
        m_minLOD = base.m_LOD;
        m_pos = base.m_pos;
    }

    void  Chunk::Free()
    {
        GenImage::FreeImage(m_buf_heightmap);
        m_minLOD = UINT_MAX;
    }

    void  Chunk::Draw(const float* viewMatrix, const float* projectionMatrix, uint LOD)
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
                float tmp[3] = { m_pos.x() * ((float)config.chunkWidth * config.quadSize), 0.f, m_pos.y() * ((float)config.chunkHeight * config.quadSize) };
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


            Vec3<float> dir(config.globalLight[0], config.globalLight[1], config.globalLight[2]);
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
                glUniform1f(maxHeightLoc, config.maxHeight);
            }

			GLint fogDistanceLoc = glGetUniformLocation(m_shaderProgram, "fog_distance");
			if (fogDistanceLoc != -1)
			{
				glUniform1f(fogDistanceLoc, config.fogDistance);
			}

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

    bool  Chunk::IsLODReady(uint LOD)
    {
        return (LOD >= m_minLOD);
    }

    bool  Chunk::IsLoaded()
    {
        return (m_minLOD != UINT_MAX);
    }
}