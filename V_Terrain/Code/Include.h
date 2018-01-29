#ifndef __VTERRAIN_INCLUDE__
#define __VTERRAIN_INCLUDE__

#include "Utils\Globals.h"

#include "Noise\NoiseMap.h"

namespace VTerrain
{
    NoiseMap GenNoiseMap(unsigned int width, unsigned int heigth, unsigned int offsetX, unsigned int offsetY);
};

#endif // !__VTERRAIN_INCLUDE__