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
#pragma once

#include "../Globals.h"
#include "ChunkRequests.h"
#include "ChunkMesh.h"
#include "../Utils/Shaders.h"

#include "ConditionalTexture.h"

namespace RPGT
{
	class Chunk
	{
	public:
		Chunk();

		void Regenerate(GeneratedChunk base);
		void Free();
		void BindHeightmap(int textureN) const;
        void BindModelMatrix(uint uniformLocation) const;
		bool IsLoaded()  const;
		Vec2<int> GetPos() const;
        void GetPoint(float x, float y, float & height, float * normal) const;

    private:
		Vec2<int> m_pos;
		uint m_buf_heightmap;
		std::vector<float> m_data;
	};
}