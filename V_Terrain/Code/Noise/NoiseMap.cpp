#include "NoiseMap.h"

namespace VTerrain
{
    uint NoiseMap::_m_maxMemWaste = 1024u;

    NoiseMap::NoiseMap() :
        m_width(.0f)
        , m_heigth(.0f)
        , m_data(nullptr)
    {
    }

    NoiseMap::NoiseMap(const uint w, const uint h) :
        m_width(w)
        , m_heigth(h)
        , m_data(new float[w*h])
    {
    }

    NoiseMap::NoiseMap(const NoiseMap & cpy) :
       NoiseMap(cpy.m_width,cpy.m_width)
    {
        memcpy_s(m_data, sizeof(float) * m_width * m_heigth, cpy.m_data, sizeof(float) * cpy.m_width * cpy.m_heigth);
    }

    NoiseMap::~NoiseMap()
    {
        if (m_data != nullptr) { delete[] m_data; m_data = nullptr; }
    }

    void NoiseMap::Set(float * data, uint width, uint heigth)
    {
        if (   m_data == nullptr
            || m_width * m_heigth < width * heigth
            || m_width * m_heigth - width * heigth > _m_maxMemWaste)
        {
            if (m_data != nullptr)
            {
                delete[] m_data;
            }
            m_data = new float[width * heigth];
        }
        m_width = width;
        m_heigth = heigth;
        memcpy_s(m_data, m_width*m_heigth * sizeof(float), data, width * heigth * sizeof(float));
    }

    uint NoiseMap::Width()
    {
        return m_width;
    }

    uint NoiseMap::Heigth()
    {
        return m_heigth;
    }
}