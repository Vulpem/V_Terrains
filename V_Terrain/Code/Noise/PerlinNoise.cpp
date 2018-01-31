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

    float PerlinNoise::GetValue(int x, int y, int width, int height, const std::vector<NoiseData>& layers)
    {
        float ret = 0.0f;
        for (int n = 0; n < layers.size(); n++)
        {
            const NoiseData& d = layers[n];
            ret += (1 - m_instance.m_perlin.octaveNoise0_1(x / (width / utils::Clamp(d.m_frequency, 0.1f, 64.0f)), y / (height / utils::Clamp(d.m_frequency, 0.1f, 64.0f)), utils::Clamp(d.m_octaves, 1, 16))) * d.m_strength;
        }

        return ret;
    }

    PerlinNoise::NoiseMap PerlinNoise::GenNoiseMap(uint width, uint height, int offsetX, int offsetY, const std::vector<NoiseData>& layers)
    {
        PerlinNoise::NoiseMap ret(width, height);
        if (layers.size() > 0)
        {
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    ret.Data()[x + y * width] = GetValue(x + offsetX, y + offsetY, width, height, layers);
                }
            }
        }
        return ret;
    }
}