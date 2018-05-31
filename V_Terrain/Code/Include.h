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
#ifndef __RPGT_INCLUDE__
#define __RPGT_INCLUDE__

#include "Terrainconfig.h"
#include "Chunk/ConditionalTexture.h"

namespace RPGT
{
   //Global Funcs

    void Init();
	void Update(int posX, int posY);
    void Render(const float * viewMatrix, const float * projectionMatrix);
    void SetHeightCurve(std::function<float(float)> func);
	void CleanChunks();
    void RegenerateMesh();
    void SetSeed(unsigned int seed);

    std::string CompileShaders(const char * frag, const char * vert, const char* TCS, const char* TES);

    float GetHeightAt(float x, float y);

	const ConditionalTexture& GetTexture(int n);
    void SetTexture(int n, const ConditionalTexture& tex);

    //TMP
    std::string GetDefaultVertexShader();
    std::string GetDefaultFragmentShader();
    std::string GetDefaultTCS();
    std::string GetDefaultTES();
    void SaveShader(const std::string& data, const char* fileName);
}

#endif // !__RPGT_INCLUDE__
