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
//  You should have received a copy of the GNU General Public License along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
#include "PerlinNoise.h"
#include <time.h>
#include <random>
#include <numeric>

namespace VTerrain
{

    PerlinNoise::PerlinNoise()
    {
        //TODO
        SetSeed(12345);//static_cast<uint>(time(NULL)));
    }

    void PerlinNoise::SetSeed(uint seed)
    {
        m_perlin.reseed(seed);
    }

    PerlinNoise::NoiseMap PerlinNoise::GenNoiseMap(Vec2<int> offset)
    {
        PerlinNoise::NoiseMap ret(config.chunkWidth + 3, config.chunkHeight + 3);
        config.noise.frequency = utils::Clamp(config.noise.frequency, 0.1f, 64.0f);
        config.noise.octaves = utils::Clamp(config.noise.octaves, 1u, 16u);

        const uint startX = -offset.x() * (config.chunkWidth) - 1;
        const uint startY = -offset.y() * (config.chunkHeight) - 1;

        for (int y = 0; y < static_cast<int>(ret.Height()); y++)
        {
            for (int x = 0; x < static_cast<int>(ret.Width()); x++)
            {
                ret[x + y * ret.Width()] = GetValue(x + startX, y + startY);
            }
        }
        return ret;
    }

    float PerlinNoise::GetValue(int x, int y)
    {
        const float dx = (config.chunkWidth) / config.noise.frequency;
        const float dy = (config.chunkHeight) / config.noise.frequency;
        return (float)m_perlin.octaveNoise0_1(x / dx, y / dy, config.noise.octaves, config.noise.lacunarity, config.noise.persistency);
    }
}