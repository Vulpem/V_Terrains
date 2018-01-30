#include "PerlinNoise.h"
#include <time.h>
#include <random>
#include <numeric>

namespace VTerrain
{
    PerlinNoise PerlinNoise::m_instance = PerlinNoise();

    PerlinNoise::PerlinNoise()
    {
        SetSeed(static_cast<uint>(time(NULL)));
    }

    void PerlinNoise::SetSeed(uint seed)
    {
        m_instance.m_seed = seed;

        P().clear();
        P().resize(256);
        std::iota(P().begin(), P().end(), 0);
        std::default_random_engine randEngine(seed);
        std::shuffle(P().begin(), P().end(), randEngine);
        P().insert(P().end(), P().begin(), P().end());
    }

    float PerlinNoise::GetValue(uint x, uint y, float scale)
    {
        return float(x + y * 100u);
    }

    PerlinNoise::NoiseMap PerlinNoise::GenNoiseMap(uint width, uint heigth, uint offsetX, uint offsetY)
    {
        PerlinNoise::NoiseMap ret(width, heigth);
        for (uint y = 0; y < heigth; y++)
        {
            for (uint x = 0; x < width; x++)
            {
                ret[x][y] = GetValue(offsetX + x, offsetY + y);
            }
        }
        return ret;
    }
}