#pragma once

#include "Globals.h"

namespace VTerrain
{
	template <typename T,
		class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    class Vec3
    {
	public:
		Vec3(T _x = 0, T _y = 0, T _z = 0) : d{ _x, _y, _z } {}
		T d[3];

		T& x() { return d[0]; }
		T x() const { return d[0]; }
		T& y() { return d[1]; }
		T y() const { return d[1]; }
		T& z() { return d[2]; }
		T z() const { return d[2]; }

		Vec3 operator+ (Vec3 a) const { return Vec3(d[0] + a.d[0], d[1] + a.d[1], d[2] + a.d[2]); }
		Vec3 operator- (Vec3 a) const { return Vec3(d[0] - a.d[0], d[1] - a.d[1], d[2] - a.d[2]); }
		Vec3 operator= (Vec3 a) { d[0] = a.d[0]; d[1] = a.d[1]; d[2] = a.d[2]; return *this; }
		Vec3 operator* (T a) const { return Vec3(d[0]*a, d[1]*a, d[2]*a); }
		Vec3 operator/ (T a) const { return Vec3(d[0] / a, d[1] / a, d[2] / a); }
        bool operator== (Vec3 a) const { return (d[0] == a.d[0] && d[1] == a.d[1] && d[2] == a.d[2]); }
        bool operator!= (Vec3 a) const { return !(*this == a); }
		float Length() const { return sqrt(d[0]*d[0] + d[1]*d[1] + d[2] * d[2]); }
		void Normalize()
		{
			const float len = Length();
			d[0] = d[0] / len;
			d[1] = d[1] / len;
			d[2] = d[2] / len;
		}
		const T* Data() const { return d; }
    };

	template <typename T,
		class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
		class Vec2
	{
	public:
		Vec2(T _x = 0, T _y = 0) : d{ _x, _y } {}
		T d[2];

		T& x() { return d[0]; }
		T x() const { return d[0]; }
		T& y() { return d[1]; }
		T y() const { return d[1]; }

		Vec2 operator+ (Vec2 a) const { return Vec2(d[0] + a.d[0], d[1] + a.d[1]); }
		Vec2 operator- (Vec2 a) const { return Vec2(d[0] - a.d[0], d[1] - a.d[1]); }
		Vec2 operator= (Vec2 a) { d[0] = a.d[0]; d[1] = a.d[1]; return *this; }
		Vec2 operator* (T a) const { return Vec2(d[0]*a, d[1]*a); }
		Vec2 operator/ (T a) const { return Vec2(d[0] / a, d[1] / a); }
        bool operator== (Vec2 a) const { return (d[0] == a.d[0] && d[1] == a.d[1]); }
        bool operator!= (Vec2 a) const { return !(*this == a); }
		float Length() const { return sqrt(d[0]*d[0] + d[1]*d[1]); }
		void Normalize()
		{
			const float len =Length();
			d[0] = d[0] / len;
			d[1] = d[1] / len;
		}
		const T* Data() const { return d; }
	};
}