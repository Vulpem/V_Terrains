#pragma once

#include "../Utils/Globals.h"
#include "../Utils/IncludeSTD.h"

namespace VTerrain
{
    /*
    Contains a 2D array of width and heigth.
    Can be accesed using [y][x] operator
    */
    class NoiseMap
    {
    private:
        uint m_heigth;
        uint m_width;
        float* m_data;
        static uint _m_maxMemWaste;

    public:
        NoiseMap();
        NoiseMap(const uint w, const uint h);
        NoiseMap(const NoiseMap& cpy);
        ~NoiseMap();

        void Set(float* data, uint width, uint heigth);
        uint Width();
        uint Heigth();

        void SetMaxWaste(uint nFloats) { _m_maxMemWaste = nFloats; }

        utils::Row<float> operator[] (uint index) { return utils::Row<float>(m_data + (index * m_width)); }
        utils::Row<float> operator[] (uint index) const { return utils::Row<float>(m_data + (index * m_width)); }
        NoiseMap operator= (const NoiseMap& cpy) { Set(cpy.m_data, cpy.m_width, cpy.m_heigth); return *this; }
    };
}