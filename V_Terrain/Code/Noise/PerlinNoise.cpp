#include "PerlinNoise.h"
#include <time.h>
#include <random>
#include <numeric>



namespace VTerrain
{
    PerlinNoise PerlinNoise::m_instance = PerlinNoise();

    PerlinNoise::PerlinNoise()
    {
        //TODO
        SetSeed(12345);//static_cast<uint>(time(NULL)));
    }

    void PerlinNoise::SetSeed(uint seed)
    {
        m_instance.m_perlin.reseed(seed);
    }

    PerlinNoise::NoiseMap PerlinNoise::GenNoiseMap(int offsetX, int offsetY)
    {
        PerlinNoise::NoiseMap ret(Config::chunkWidth + 1, Config::chunkHeight + 1);
        Config::Noise::frequency = utils::Clamp(Config::Noise::frequency, 0.1f, 64.0f);
        Config::Noise::octaves = utils::Clamp(Config::Noise::octaves, 1u, 16u);

        for (int y = 0; y < static_cast<int>(ret.Height()); y++)
        {
            for (int x = 0; x < static_cast<int>(ret.Width()); x++)
            {
                ret[x + y * ret.Width()] = GetValue(x - offsetX * (Config::chunkWidth), y - offsetY * (Config::chunkWidth));
            }
        }
        return ret;
    }

    float PerlinNoise::GetValue(int x, int y)
    {
        float dx = static_cast<float>(Config::chunkWidth) / Config::Noise::frequency;
        float dy = static_cast<float>(Config::chunkHeight) / Config::Noise::frequency;
        return (float)m_instance.m_perlin.octaveNoise0_1(x / dx, y / dy, Config::Noise::octaves, Config::Noise::lacunarity, Config::Noise::persistency);
    }
}