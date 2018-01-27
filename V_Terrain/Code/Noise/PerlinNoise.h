#pragma once

#include "../Utils/Globals.h"
#include "../Utils/IncludeSTD.h"

namespace VTerrain
{
    template<typename T>
    class Row
    {
        T* m_ptr;
    public:
        Row(T* ptr) : m_ptr(ptr) {};
        T& operator[] (uint index) { return ptr[index]; }
        T& operator[] (uint index) const { return ptr[index]; }
    };

    class NoiseMap
    {
    private:
        uint m_heigth;
        uint m_width;
        float* m_data;

    public:
        NoiseMap() = delete;
        NoiseMap(uint w, uint h);
        ~NoiseMap();

        void Set(float* data, uint len);
        uint Width();
        uint Heigth();

        Row<float> operator[] (uint index) { return Row<float>(m_data + (index * m_width)); }
        Row<float> operator[] (uint index) const { return Row<float>(m_data + (index * m_width)); }
    };



    static class Noise
    {
    public:
        NoiseMap GenNoiseMap(uint width, uint heigth, uint offsetX, uint offsetY);

    };
}