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

        static void SetSeed(uint seed);
        static PerlinNoise::NoiseMap GenNoiseMap(int offsetX, int offsetY);
    private:
        static float GetValue(int x, int y);

        static PerlinNoise m_instance;
        siv::SivPerlinNoise m_perlin;
    };
}