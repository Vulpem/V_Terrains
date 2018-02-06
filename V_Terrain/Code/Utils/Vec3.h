#pragma once

#include "Globals.h"

namespace VTerrain
{
	template <typename T,
		class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    class Vec3
    {
	public:
		Vec3() : x(0), y(0), z(0) {}
		T x,y,z;

    };
}