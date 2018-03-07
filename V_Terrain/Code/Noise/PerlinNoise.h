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

#include "../Utils/Globals.h"

#include "Extern/SivPerlinNoise.h"

namespace VTerrain
{
    class PerlinNoise
    {
    public:
#pragma region NoiseMap
        /*
        Contains a 2D array of width and heigth.
        Can be accesed using [y][x] operator
        */
        class NoiseMap
        {
        private:
            uint m_height;
            uint m_width;
            std::vector<float> m_data;

        public:
            NoiseMap();
            NoiseMap(const uint w, const uint h);
            ~NoiseMap();

            void Set(std::vector<float> data, uint width, uint height);
            void Init(uint width, uint height);
            uint Width() const;
            uint Height() const;
            const std::vector<float>& Data() const { return m_data; }
            std::vector<float>& Data() { return m_data; }

            float& operator[] (int x) { return m_data[x]; };
			float operator[] (int x) const { return m_data[x]; }
        };
#pragma endregion

    public:
        PerlinNoise();

        void SetSeed(uint seed);
        PerlinNoise::NoiseMap GenNoiseMap(Vec2<int> offset);
    private:
        float GetValue(int x, int y);

        siv::SivPerlinNoise m_perlin;
    };
}