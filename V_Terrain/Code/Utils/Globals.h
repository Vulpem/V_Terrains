#ifndef __GLOBALS_V_
#define __GLOBALS_V_

typedef unsigned int uint;

#include "IncludeSTD.h"
#include "../TerrainConfig.h"
#include "Vec3.h"

namespace VTerrain
{
    namespace utils
    {
        static constexpr float PI = 3.14159265359f;
        static constexpr float PI2 = 6.28318530718f;

        static constexpr float RAD_TO_DEG = 57.2957795f;
        static constexpr float DEG_TO_RAD = 0.0174532925f;

        template <typename T,
            class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
        T Clamp(T a, T min, T max) { return (a > max ? max : (a < min ? min : a)); }

		template <typename T>
			void Swap(T& a, T& b) { T c = a; a = b; b = c; }

        template <typename T,
            class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
        T Min(T a, T b) { return (a < b ? a : b); }

        template <typename T,
            class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
        T Max(T a, T b) { return (a > b ? a : b); }

        template <typename T,
            class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
            bool IsAlmost(T a, T b, T range) { return (abs(a - b) < range); }
    }
}
#endif // !__GLOBALS__