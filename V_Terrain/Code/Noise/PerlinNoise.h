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

#include "../Globals.h"

#include "NoiseMap.h"
#include "Extern/SivPerlinNoise.h"

namespace VTerrain
{
    class PerlinNoise
    {
    public:
        PerlinNoise();

        void SetSeed(uint seed);
        NoiseMap GenNoiseMap(Vec2<int> offset) const;
        float GetValue(int x, int y)  const;
        void SetCurve(std::function<float(float)> func);

	private:
        siv::SivPerlinNoise m_perlin;
        std::function<float(float)> m_heightCurve;
    };
}