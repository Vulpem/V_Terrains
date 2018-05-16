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
	Config VTerrain::config;
	ChunkManager chunkManager;

    void Init()
    {
        chunkManager.Init();
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

    std::string GetDefaultVertexShader()
    {
        return Shaders::GetDefaultVertexShader();
    }

    std::string GetDefaultFragmentShader()
    {
        return Shaders::GetDefaultFragmentShader();
    }

    std::string GetDefaultTCS()
    {
        return Shaders::GetDefaultTCSShader();
    }

    std::string GetDefaultTES()
    {
        return Shaders::GetDefaultTESShader();
    }

#if _DEBUG
    void SaveShader(const std::string & data, const char * fileName)
    {
        Shaders::SaveFile(data, fileName);
    }
#else
    void SaveShader(const std::string & data, const char * fileName)
    {
    }
#endif

    std::string CompileShaders(const char * frag, const char * vert, const char* TCS, const char* TES)
    {
        std::string result;
        Shader shader = Shaders::CompileShader(vert, frag, TCS, TES, result);
        if (shader.m_program != 0)
        {
            if (chunkManager.m_shader.m_program != 0)
            {
                Shaders::FreeShader(chunkManager.m_shader);
            }
            chunkManager.m_shader = shader;
        }
        return result;
    }

	const ConditionalTexture& GetTexture(int n)
	{
		return chunkManager.m_textures[n];
	}

    void SetTexture(int n, const ConditionalTexture & tex)
    {
        chunkManager.m_textures[n] = tex;
    }
}