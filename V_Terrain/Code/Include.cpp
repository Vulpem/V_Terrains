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
#include "Include.h"

#include "Utils/Shaders.h"
#include "Chunk/ChunkManager.h"

namespace VTerrain
{
	Config VTerrain::config = Config();
	ChunkManager chunkManager = ChunkManager();

    void Init()
    {
        Chunk::m_shader = VTerrain::Shaders::CompileShader(nullptr, nullptr);
    }

	void Update(int posX, int posY)
	{
		chunkManager.Update(posX, posY);
	}

	void Render(const float * viewMatrix, const float * projectionMatrix)
	{
		chunkManager.Render(viewMatrix, projectionMatrix);
	}

    void SetHeightCurve(std::function<float(float)> func)
    {
        chunkManager.SetHeightCurve(func);
    }

	void CleanChunks()
	{
		chunkManager.CleanChunks();
	}

    void SetSeed(unsigned int seed)
    {
        chunkManager.SetSeed(seed);
    }
}