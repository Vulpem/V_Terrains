#include "PerlinNoise.h"

namespace VTerrain
{
        PerlinNoise::PerlinNoise::NoiseMap::NoiseMap() :
              m_width(0u)
            , m_heigth(0u)
        {
        }

        PerlinNoise::NoiseMap::NoiseMap(const uint w, const uint h) :
              m_width(w)
            , m_heigth(h)
        {
            m_data.resize(w*h);
        }

        PerlinNoise::NoiseMap::~NoiseMap()
        {
        }

        void PerlinNoise::NoiseMap::Set(std::vector<float> data, uint width, uint heigth)
        {
            m_width = width;
            m_heigth = heigth;
            m_data = data;
        }

        uint PerlinNoise::NoiseMap::Width()
        {
            return m_width;
        }

        uint PerlinNoise::NoiseMap::Heigth()
        {
            return m_heigth;
        }
}