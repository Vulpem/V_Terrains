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
#include "PerlinNoise.h"
#include <time.h>
#include <random>
#include <numeric>

namespace VTerrain
{

    PerlinNoise::PerlinNoise()
        : m_heightCurve([](float a) { return a; })
    {
        SetSeed(static_cast<uint>(time(NULL)));
    }

    void PerlinNoise::SetSeed(uint seed)
    {
        m_perlin.reseed(seed);
    }

    NoiseMap PerlinNoise::GenNoiseMap(Vec2<int> offset)  const
    {
        NoiseMap ret(config.chunkHeightmapResolution + 3, config.chunkHeightmapResolution + 3);
        config.noise.frequency = utils::Clamp(config.noise.frequency, 0.1f, 64.0f);
        config.noise.octaves = utils::Clamp(config.noise.octaves, 1u, 16u);

        const uint startX = -offset.x() * config.chunkHeightmapResolution - 1u;
        const uint startY = -offset.y() * config.chunkHeightmapResolution - 1u;

		//TODO remove this test thingies
		static float min = 1000;
		static float max = -1000;
        for (uint y = 0; y < ret.Height(); y++)
        {
            for (uint x = 0; x < ret.Width(); x++)
            {
                ret[x + y * ret.Width()] = GetValue(x + startX, y + startY);
				if (ret[x + y * ret.Width()] < min) { min = ret[x + y * ret.Width()]; }
				if (ret[x + y * ret.Width()] > max) { max = ret[x + y * ret.Width()]; }

            }
        }
        return ret;
    }

    float PerlinNoise::GetValue(int x, int y)  const
    {
		const float dp = (config.chunkHeightmapResolution / config.noise.frequency);
        return m_heightCurve(m_perlin.octaveNoise0_1(x / dp, y / dp, config.noise.octaves, config.noise.lacunarity, config.noise.persistency));
    }
    void PerlinNoise::SetCurve(std::function<float(float)> func)
    {
        m_heightCurve = func;
    }
}