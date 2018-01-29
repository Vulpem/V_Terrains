#include "PerlinNoise.h"

namespace VTerrain
{
    NoiseMap PerlinNoise::GenNoiseMap(uint width, uint heigth, uint offsetX, uint offsetY)
    {
        NoiseMap ret(width, heigth);
        for (uint y = 0; y < heigth; y++)
        {
            for (uint x = 0; x < width; x++)
            {
                ret[y][x] = x + y/100.f;
            }
        }

        return ret;
    }
}