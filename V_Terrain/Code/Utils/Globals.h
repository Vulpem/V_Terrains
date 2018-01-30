#ifndef __GLOBALS_V_
#define __GLOBALS_V_

typedef unsigned int uint;

#include "IncludeSTD.h"

namespace VTerrain
{
    namespace utils
    {
        template <typename T>
        T Clamp(T a, T min, T max) { return (a > max ? max : (a < min ? min : a)); }

        template <typename T>
        T Min(T a, T b) { return (a < b ? a : b); }

        template <typename T>
        T Max(T a, T b) { return (a > b ? a : b); }
    }
}
#endif // !__GLOBALS__