#include "PerlinNoise.h"

namespace VTerrain
{
        PerlinNoise::PerlinNoise::NoiseMap::NoiseMap() :
              m_width(0u)
            , m_height(0u)
        {
        }

        PerlinNoise::NoiseMap::NoiseMap(const uint w, const uint h) :
              m_width(w)
            , m_height(h)
        {
            m_data.resize(w*h);
        }

        PerlinNoise::NoiseMap::~NoiseMap()
        {
        }

        void PerlinNoise::NoiseMap::Set(std::vector<double> data, uint width, uint heigth)
        {
            m_width = width;
            m_height = heigth;
            m_data = data;
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