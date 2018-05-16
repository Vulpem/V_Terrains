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
     Chunk::Chunk() :
        m_buf_heightmap(0u)
         , m_pos(0,0)
    {

    }

    void  Chunk::Regenerate(GeneratedChunk base)
    {
		if (m_buf_heightmap != 0)
		{
			Free();
		}

        m_buf_heightmap = GenImage::FromRGBA(base.m_data, base.m_size.x(), base.m_size.y());
        m_pos = base.m_pos;
    }

    void  Chunk::Free()
    {
		if (m_buf_heightmap != 0)
		{
			GenImage::FreeImage(m_buf_heightmap);
		}
    }

    void Chunk::BindHeightmap(int textureN) const
    {
        glActiveTexture(GL_TEXTURE0 + textureN);
        glBindTexture(GL_TEXTURE_2D, m_buf_heightmap);
    }

    void Chunk::BindModelMatrix(uint uniformLocation) const
    {
        const Vec3<float> chunkPos(m_pos.x() * config.chunkSize, 0.f, m_pos.y() * config.chunkSize);
        const float modelMatrix[] =
        {
            config.chunkSize,	0,					0,					0,
            0,					config.maxHeight,	0,					0,
            0,					0,					config.chunkSize,	0,
            chunkPos.x(),		chunkPos.y(),		chunkPos.z(),		1
        };

        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, modelMatrix);
    }

    bool  Chunk::IsLoaded() const
    {
        return (m_buf_heightmap != 0);
    }

	Vec2<int> Chunk::GetPos() const
	{
		return m_pos;
	}
}