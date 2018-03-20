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
#include "Include.h"

#include "Utils/Shaders.h"
#include "Chunk/ChunkManager.h"
#include "Utils/GenImage.h"

namespace VTerrain
{
	Config VTerrain::config = Config();
	ChunkManager chunkManager = ChunkManager();

    void Init()
    {
        std::string result;
        Chunk::m_shader = VTerrain::Shaders::CompileShader(nullptr, nullptr, nullptr, nullptr, result);
		assert(Chunk::m_shader.m_program != 0);

        const bool error = GenImage::Init();
        assert(error);
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




    std::string GetVertexShader()
    {
        return Shaders::m_defaultVertexShader;
    }

    std::string GetFragmentShader()
    {
        return  Shaders::m_defaultFragmentShader;
    }

    std::string GetTCS()
    {
        return Shaders::m_defaultTCSShader;
    }

    std::string GetTES()
    {
        return Shaders::m_defaultTESShader;
    }

    std::string CompileShaders(const char * frag, const char * vert, const char* TCS, const char* TES)
    {
        std::string result;
        Shader shader = Shaders::CompileShader(vert, frag, TCS, TES, result);
        if (shader.m_program != 0)
        {
            Chunk::m_shader = shader;
        }
        return result;
    }
}