//  RPG Terrains
//  Procedural terrain generation for modern C++
//  Copyright (C) 2018 David Hern�ndez L�zaro
//  
//  "RPG Terrains" is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or any later version.
//  
//  "RPG Terrains" is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//  
//  For more details, read "COPYING.txt" and "COPYING.LESSER.txt" included in this project.
//  You should have received a copy of the GNU General Public License along with RPG Terrains.  If not, see <http://www.gnu.org/licenses/>.
#ifndef __GLOBALS_V_
#define __GLOBALS_V_

typedef unsigned int uint;

#include "utils/IncludeSTD.h"
#include "Terrainconfig.h"
#include "utils/Vec3.h"
#include <windows.h>

#define SHOW_ERROR(...) RPGT::utils::ShowAlertPopup(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define ASSERT(expression, ...) if(!(expression)) { SHOW_ERROR(__VA_ARGS__); DebugBreak();}

namespace RPGT
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

		template <typename T,
			class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
			T Round(T a, int nDecimals = 0)
		{
			const int m = pow(10, nDecimals);
			a *= m;
			const float f = floor(a);
			const float f2 = floor(a + 0.5f);
			if (utils::IsAlmost(f, f2, 0.001f))
			{
				return f / m;
			}
			return ceil(f) / m;
		}

		template <typename T,
			class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
			T Mix(T a, T b, float n) { return (a * n) + (b * (1.f - n)); }

		int FormatCString(char *outBuffer, int outBufferSize, const char* format, va_list args);

		template<size_t BufferSize>
		int FormatCString(char(&outBuffer)[BufferSize], const char* format, va_list args)
		{
			static_assert(BufferSize > 0, "Output buffer size has to be greater than 0");
			return FormatCString(outBuffer, BufferSize, format, args);
		}

		void ShowAlertPopup(const char* file, const char* function, const int line, const char* format, ...);
    }
}
#endif // !__GLOBALS__