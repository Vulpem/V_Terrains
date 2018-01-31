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
        public:
            class Row
            {
                std::vector<float>& m_ptr;
                uint m_w;
                uint m_x;
            public:
                Row(std::vector<float>& ptr, const uint width, const uint x) : m_ptr(ptr), m_w(width), m_x(x) {};
                float& operator[] (uint y) { return m_ptr[y*m_w + m_x]; }
            };

            class CRow
            {
                const std::vector<float>& m_ptr;
                const uint m_w;
                const uint m_x;
            public:
                CRow(const std::vector<float>& ptr, const uint width, const uint x) : m_ptr(ptr), m_w(width), m_x(x) {};
                float operator[] (uint y) const { return m_ptr[y*m_w + m_x]; }
            };

        private:
            uint m_heigth;
            uint m_width;
            std::vector<float> m_data;

        public:
            NoiseMap();
            NoiseMap(const uint w, const uint h);
            ~NoiseMap();

            void Set(std::vector<float> data, uint width, uint heigth);
            uint Width();
            uint Heigth();
            const std::vector<float>& Data() const { return m_data; }
            std::vector<float>& Data() { return m_data; }

            Row operator[] (uint x) { return Row(m_data, m_width, x); }
            CRow operator[] (uint x) const { return CRow(m_data, m_width, x); }
        };
#pragma endregion

    public:
        PerlinNoise();

        static void SetSeed(uint seed);
        static float GetValue(int x, int y, int width, int height, float frequency, int octaves, float lacunarity, float persistency);
        static PerlinNoise::NoiseMap GenNoiseMap(uint width, uint height, int offsetX, int offsetY, float frequency, int octaves, float lacunarity, float persistency);

    private:
        static PerlinNoise m_instance;
        siv::SivPerlinNoise m_perlin;
    };
}