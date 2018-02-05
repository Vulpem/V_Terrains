#ifndef __V__TERRAIN__CONFIG__
#define __V__TERRAIN__CONFIG__

namespace VTerrain
{
    namespace Config
    {
        static float maxHeight = 15.f;
        static unsigned int chunkWidth = 50u;
        static unsigned int chunkHeight = 50u;

        namespace Noise
        {
            static float frequency = 10.f;
            static unsigned int octaves = 5u;
            static float lacunarity = 3.f;
            static float persistency = 0.4f;
        };
   };
}
#endif