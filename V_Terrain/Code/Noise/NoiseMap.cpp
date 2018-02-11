//  V Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hern�ndez L�zaro
//  
//  "V Terrains" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or any later version.
//  
//  "V Terrains" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//  
//  For more details, read "COPYING.txt" and "COPYING.LESSER.txt" included in this project.
//  You should have received a copy of the GNU General Public License along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
#include "PerlinNoise.h"

namespace VTerrain
{
        PerlinNoise::PerlinNoise::NoiseMap::NoiseMap() :
              m_width(0u)
            , m_height(0u)
        {
        }

        PerlinNoise::NoiseMap::NoiseMap(const uint w, const uint h) :
              m_width(w + 2)
            , m_height(h + 2)
        {
            m_data.resize(m_width*m_height);
        }

        PerlinNoise::NoiseMap::~NoiseMap()
        {
        }

        void PerlinNoise::NoiseMap::Set(std::vector<float> data, uint width, uint height)
        {
            m_width = width + 2;
            m_height = height + 2;
            m_data = data;
        }

        void PerlinNoise::NoiseMap::Init(uint width, uint height)
        {
            m_width = width + 2;
            m_height = height + 2;
        }

        uint PerlinNoise::NoiseMap::Width() const
        {
            return m_width;
        }

        uint PerlinNoise::NoiseMap::Height() const
        {
            return m_height;
        }
}