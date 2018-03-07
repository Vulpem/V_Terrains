//  V Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hernàndez Làzaro
//  
//  "V Terrains" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or any later version.
//  
//  "V Terrains" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//  
//  For more details, read "COPYING.txt" and "COPYING.LESSER.txt" included in this project.
//  You should have received a copy of the GNU General Public License along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
#ifndef __GLOBALS_V_
#define __GLOBALS_V_

typedef unsigned int uint;

#include "IncludeSTD.h"
#include "../TerrainConfig.h"
#include "Vec3.h"
#include "../TerrainConfig.h"

namespace VTerrain
{
	Config config;

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