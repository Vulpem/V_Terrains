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
            m_data.resize(w*h);
        }

        PerlinNoise::NoiseMap::~NoiseMap()
        {
        }

        void PerlinNoise::NoiseMap::Set(std::vector<float> data, uint width, uint heigth)
        {
            m_width = width + 2;
            m_height = heigth + 2;
            m_data = data;
        }

        uint PerlinNoise::NoiseMap::RealWidth() const
        {
            return m_width;
        }

        uint PerlinNoise::NoiseMap::RealHeight() const
        {
            return m_height;
        }

        uint PerlinNoise::NoiseMap::Width() const
        {
            return m_width - 2;
        }

        uint PerlinNoise::NoiseMap::Height() const
        {
            return m_height - 2;
        }
        float & PerlinNoise::NoiseMap::operator[](int index)
        {
            //TODONOW
            const int W = static_cast<int>(m_width);
            const int y = floorf((float)index / (float)W);
            const int x = index - y * W + y * 2;
            const int i = y*(W-2) + x;
            return m_data[0];
        }
}