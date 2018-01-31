#include "PerlinNoise.h"
#include <time.h>
#include <random>
#include <numeric>



namespace VTerrain
{
    PerlinNoise PerlinNoise::m_instance = PerlinNoise();

    PerlinNoise::PerlinNoise()
    {
        SetSeed(12345);//static_cast<uint>(time(NULL)));
    }

    void PerlinNoise::SetSeed(uint seed)
    {
        m_instance.m_perlin.reseed(seed);
    }

    float PerlinNoise::GetValue(int x, int y, int width, int height, float frequency, int octaves, float lacunarity, float persistency)
    {
        frequency = utils::Clamp(frequency, 0.1f, 64.0f);
        octaves = utils::Clamp(octaves, 1, 16);
        return m_instance.m_perlin.octaveNoise0_1(x / (width / frequency), y / (height / frequency), octaves, lacunarity, persistency);
    }

    PerlinNoise::NoiseMap PerlinNoise::GenNoiseMap(uint width, uint height, int offsetX, int offsetY, float frequency, int octaves, float lacunarity, float persistency)
    {
        PerlinNoise::NoiseMap ret(width, height);
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    ret.Data()[x + y * width] = GetValue(x + offsetX, y + offsetY, width, height, frequency, octaves, lacunarity, persistency);
                }
            }
        return ret;
    }
}