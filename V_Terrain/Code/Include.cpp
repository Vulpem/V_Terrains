#include "Include.h"

#include "Noise\PerlinNoise.h"

namespace VTerrain
{
    NoiseMap GenNoiseMap(unsigned int width, unsigned int heigth, unsigned int offsetX, unsigned int offsetY)
    {
        return PerlinNoise::GenNoiseMap(width, heigth, offsetX, offsetY);
    }
    
}