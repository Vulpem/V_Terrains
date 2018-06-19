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
#include "Chunk.h"

#include "ChunkMesh.h"
#include "../Utils/GenImage.h"
#include "../ExternalLibs/Glew/include/glew.h"

namespace RPGT
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
		m_data = base.m_data;
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

    void Chunk::GetPoint(float _x, float _y, float & height, float * normal) const
    {
		if (IsLoaded())
		{
			const float x = _x / (config.chunkSize / (config.chunkHeightmapResolution));
			const float y = _y / (config.chunkSize / (config.chunkHeightmapResolution));

			const int lx = static_cast<int>(floor(x));
			int mx = static_cast<int>(ceil(x));
			if (mx == lx) { mx++; }
			const int ly = static_cast<int>(floor(y));
			int my = static_cast<int>(ceil(y));
			if (my == ly) { my++; }

			const int row = config.chunkHeightmapResolution + 1;
			const int val = (ly * row + lx);

			if (normal != nullptr)
			{
				normal[0] = utils::Mix(
					utils::Mix(m_data[(val) * 4 + 0], m_data[(val + row) * 4 + 0], my - y),
					utils::Mix(m_data[(val + 1) * 4 + 0], m_data[(val + row + 1) * 4 + 0], my - y),
					mx - x
				) * 2.f - 1.f;
				normal[1] = utils::Mix(
					utils::Mix(m_data[(val) * 4 + 1], m_data[(val + row) * 4 + 1], my - y),
					utils::Mix(m_data[(val + 1) * 4 + 1], m_data[(val + row + 1) * 4 + 1], my - y),
					mx - x
				) * 2.f - 1.f;
				normal[2] = utils::Mix(
					utils::Mix(m_data[(val) * 4 + 2], m_data[(val + row) * 4 + 2], my - y),
					utils::Mix(m_data[(val + 1) * 4 + 2], m_data[(val + row + 1) * 4 + 2], my - y),
					mx - x
				) * 2.f - 1.f;
			}
			height = utils::Mix(
				utils::Mix(m_data[(val) * 4 + 3], m_data[(val + row) * 4 + 3], my - y),
				utils::Mix(m_data[(val + 1) * 4 + 3], m_data[(val + row + 1) * 4 + 3], my - y),
				mx - x
			);

			//height = m_data[val * 4 + 3];
		}
    }
}