#pragma once

#include "Globals.h"

namespace VTerrain
{
    class GenImage
    {
    public:
        static uint FromRGB(std::vector<float> color, unsigned int w, unsigned int h, bool normalize = false);
        static bool FreeImage(uint buffer);
    };
}