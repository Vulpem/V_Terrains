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
#include "Include.h"

#include "Utils/Shaders.h"
#include "Chunk/ChunkManager.h"
#include "Utils/GenImage.h"

namespace RPGT
{
	Config RPGT::config;
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

	void CleanChunks()
	{
		chunkManager.CleanChunks();
	}

    void RegenerateMesh()
    {
        chunkManager.GenerateMesh();
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

    void SaveShader(const std::string & data, const char * fileName)
    {
#if _DEBUG
        Shaders::SaveFile(data, fileName);
#endif
    }

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

    void GetPoint(float x, float y, float& height, float* normal)
    {
        chunkManager.GetPoint(x, y, height, normal);
    }

	bool RayCast(float * origin, float * direction, float * hitPoint, float * hitNormal)
	{
		return chunkManager.RayCast(origin, direction, hitPoint, hitNormal);
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