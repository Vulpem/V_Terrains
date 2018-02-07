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