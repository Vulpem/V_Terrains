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

    void  Chunk::Draw(const Shader& shader, const Vec3<float>& cameraPos) const
    {
        if (IsLoaded())
        {
            const float tmp[3] = { m_pos.x() * config.chunkSize, 0.f, m_pos.y() * config.chunkSize };
            glUniform3fv(shader.loc_position_offset, 1, tmp);


			//TODO: improve this nonsense
			int density[6];
			density[0] = utils::Max(1,          (int)config.nLODs - static_cast<int>((cameraPos - Vec3<float>(tmp[0],                   tmp[1], tmp[2]  )).Length() / config.LODdistance));
			density[1] = density[0];

			density[2] = utils::Max(density[0], (int)config.nLODs - static_cast<int>((cameraPos - Vec3<float>(tmp[0],					tmp[1], tmp[2] - config.chunkSize	)).Length() / config.LODdistance));
			density[3] = utils::Max(density[0], (int)config.nLODs - static_cast<int>((cameraPos - Vec3<float>(tmp[0] - config.chunkSize,tmp[1], tmp[2]						)).Length() / config.LODdistance));
			density[4] = utils::Max(density[0], (int)config.nLODs - static_cast<int>((cameraPos - Vec3<float>(tmp[0],					tmp[1], tmp[2] + config.chunkSize	)).Length() / config.LODdistance));
			density[5] = utils::Max(density[0], (int)config.nLODs - static_cast<int>((cameraPos - Vec3<float>(tmp[0] + config.chunkSize,tmp[1], tmp[2]						)).Length() / config.LODdistance));

			glUniform1iv(shader.loc_tesselationDensity, 6, density);


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_buf_heightmap);

            glDrawElements(GL_PATCHES, 6/*nIndices*/, GL_UNSIGNED_INT, (void*)0);
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