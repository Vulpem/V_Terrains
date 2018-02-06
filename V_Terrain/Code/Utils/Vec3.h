#pragma once

#include "Globals.h"

namespace VTerrain
{
	template <typename T,
		class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    class Vec3
    {
	public:
		Vec3(T _x = 0, T _y=0, T _z=0) : x(_x), y(_y), z(_z) {}
		T x,y,z;

		Vec3 operator+ (Vec3 a) { return Vec3(x + a.x, y + a.y, z + a.z); }
		Vec3 operator- (Vec3 a) { return Vec3(x - a.x, y - a.y, z - a.z); }
		Vec3 operator= (Vec3 a) { x = a.x; y = a.y; z = a.z; return *this; }
		Vec3 operator* (T a) { return Vec3(x*a, x*y, x*z); }
    };

	template <typename T,
		class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
		class Vec2
	{
	public:
		Vec2(T _x = 0, T _y = 0) : x(_x), y(_y) {}
		T x, y;

		Vec2 operator+ (Vec2 a) { return Vec2(x + a.x, y + a.y); }
		Vec2 operator- (Vec2 a) { return Vec2(x - a.x, y - a.y); }
		Vec2 operator= (Vec2 a) { x = a.x; y = a.y; return *this; }
		Vec2 operator* (T a) { return Vec2(x*a, x*y); }
	};
}