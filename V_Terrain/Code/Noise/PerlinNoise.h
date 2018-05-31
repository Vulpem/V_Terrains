//  RPG Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hern�ndez L�zaro
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

#include "NoiseMap.h"

namespace RPGT
{
    class PerlinNoise
    {
    public:
        PerlinNoise();

        void SetSeed(uint seed);
        NoiseMap GenNoiseMap(Vec2<int> offset) const;
        void SetCurve(std::function<float(float)> func);

	private:
        float GetValue(int x, int y)  const;

        siv::SivPerlinNoise m_perlin;
        std::function<float(float)> m_heightCurve;
    };
}