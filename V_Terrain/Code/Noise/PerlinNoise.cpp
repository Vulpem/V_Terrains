#include "PerlinNoise.h"

namespace VTerrain
{
    NoiseMap::NoiseMap(uint w, uint h) :
        m_width(w)
        , m_heigth(h)
        , m_data(new float[w*h])
    {
    }

    NoiseMap::~NoiseMap()
    {
        RELEASE_AR(m_data);
    }

    void NoiseMap::Set(float * data, uint len)
    {
        assert(data == nullptr);
        memcpy_s(m_data, m_width*m_heigth * sizeof(float), data, len * sizeof(float));
    }

    uint NoiseMap::Width()
    {
        return m_width;
    }

    uint NoiseMap::Heigth()
    {
        return m_heigth;
    }





    NoiseMap Noise::GenNoiseMap(uint width, uint heigth, uint offsetX, uint offsetY)
    {
        NoiseMap ret(width, heigth);
        return ret;
    }
}