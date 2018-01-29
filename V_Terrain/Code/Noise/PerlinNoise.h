#pragma once

#include "../Utils/Globals.h"
#include "../Utils/IncludeSTD.h"
#include "NoiseMap.h"

namespace VTerrain
{
    class PerlinNoise
    {
    public:
        static NoiseMap GenNoiseMap(uint width, uint heigth, uint offsetX, uint offsetY);
    };
}