#pragma once

#include "Globals.h"

namespace VTerrain
{
    class GenImage
    {
    public:
        static uint FromRGB(const std::vector<float>& r, const std::vector<float>& g, const std::vector<float>& b, unsigned int w, unsigned int h, bool normalize = false);
        static uint BlackAndWhite(const std::vector<float>&, unsigned int w, unsigned int h, bool normalize = false);
        static bool FreeImage(uint buffer);
    };
}