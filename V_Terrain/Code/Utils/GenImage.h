#pragma once

#include "Globals.h"

namespace VTerrain
{
    class GenImage
    {
    public:
        static uint FromRGB(const std::vector<float>& color, unsigned int w, unsigned int h);
        static void FreeImage(uint& buffer);
    };
}