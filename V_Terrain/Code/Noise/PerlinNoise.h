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
            std::vector<double> m_data;

        public:
            NoiseMap();
            NoiseMap(const uint w, const uint h);
            ~NoiseMap();

            void Set(std::vector<double> data, uint width, uint heigth);
            uint Width() const;
            uint Height() const;
            const std::vector<double>& Data() const { return m_data; }
            std::vector<double>& Data() { return m_data; }

            double& operator[] (uint x) { return m_data[x]; }
            double operator[] (uint x) const { return m_data[x]; }
        };
#pragma endregion

    public:
        PerlinNoise();

        static void SetSeed(uint seed);
        static PerlinNoise::NoiseMap GenNoiseMap(int offsetX, int offsetY);
    private:
        static double GetValue(int x, int y);

        static PerlinNoise m_instance;
        siv::SivPerlinNoise m_perlin;
    };
}