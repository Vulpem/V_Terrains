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
#pragma once

namespace VTerrain
{
	class ConditionalTexture
	{
	public:
		const float* Get() const
		{
            //Please ignore this ugly cheat
            *(float*)&hasTexture = (buf_diffuse != 0 ? 10.f : 0.f);
			return color;
		}

		unsigned int buf_diffuse = 0u;
		unsigned int buf_heightmap = 0u;

        float color[3] = { 1.f,1.f,1.f };
		float minSlope = 0.f;
		float maxSlope = 1.f;
		float minHeight = 0.f;
		float maxHeight = 0.f;
		float sizeMultiplier = 4.f;
		float heightFade = 10.f;
		float slopeFade = 0.01f;
	private:
		float hasTexture = 0.f;
	};
}