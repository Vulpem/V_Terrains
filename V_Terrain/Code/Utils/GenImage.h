//  V Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hernàndez Làzaro
//  
//  "V Terrains" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or any later version.
//  
//  "V Terrains" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//  
//  For more details, read "COPYING.txt" and "COPYING.LESSER.txt" included in this project.
//  You should have received a copy of the GNU General Public License along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
#pragma once

#include <vector>

namespace VTerrain
{
    class GenImage
    {
    public:
        static unsigned int FromRGB(const std::vector<float>& color, unsigned int w, unsigned int h);
        static unsigned int FromRGBA(const std::vector<float>& color, unsigned int w, unsigned int h);
        static void FreeImage(unsigned int& buffer);
    private:
        static void SetParams();
    };
}