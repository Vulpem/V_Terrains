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
//  You should have received a copy of the GNU General Public License along with V Terrains.  If not, see <http://www.gnu.org/licenses/>.
#include "NoiseMap.h"

namespace VTerrain
{
        NoiseMap::NoiseMap() :
              m_width(0u)
            , m_height(0u)
        {
        }

        NoiseMap::NoiseMap(const uint w, const uint h) :
              m_width(w)
            , m_height(h)
        {
            m_data.resize(m_width*m_height);
        }

        NoiseMap::~NoiseMap()
        {
            m_data.clear();
        }

        void NoiseMap::Set(std::vector<float> data, uint width, uint height)
        {
            m_width = width;
            m_height = height;
            m_data = data;
        }

        void NoiseMap::Init(uint width, uint height)
        {
            m_width = width;
            m_height = height;
        }

        uint NoiseMap::Width() const
        {
            return m_width;
        }

        uint NoiseMap::Height() const
        {
            return m_height;
        }
}