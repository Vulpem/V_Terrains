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
#pragma once

namespace RPGT
{
	template <typename T,
		class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    class Vec3
    {
	public:
		Vec3(T _x = 0, T _y = 0, T _z = 0) : d{ _x, _y, _z } {}
		T d[3];

		const T* Data() const { return d; }

		T& x() { return d[0]; }
		T x() const { return d[0]; }
		T& y() { return d[1]; }
		T y() const { return d[1]; }
		T& z() { return d[2]; }
		T z() const { return d[2]; }

		Vec3 operator+ (const Vec3& a) const { return Vec3(d[0] + a.d[0], d[1] + a.d[1], d[2] + a.d[2]); }
		Vec3 operator- (const Vec3& a) const { return Vec3(d[0] - a.d[0], d[1] - a.d[1], d[2] - a.d[2]); }
		Vec3 operator= (const Vec3& a) { d[0] = a.d[0]; d[1] = a.d[1]; d[2] = a.d[2]; return *this; }
		Vec3 operator* (T a) const { return Vec3(d[0]*a, d[1]*a, d[2]*a); }
		Vec3 operator/ (T a) const { return Vec3(d[0] / a, d[1] / a, d[2] / a); }
        bool operator== (const Vec3& a) const { return (d[0] == a.d[0] && d[1] == a.d[1] && d[2] == a.d[2]); }
        bool operator!= (const Vec3& a) const { return !(*this == a); }
        float LengthSqr() const { return ((float)d[0] * (float)d[0] + (float)d[1] * (float)d[1] + (float)d[2] * (float)d[2]); }
		float Length() const { return sqrtf((float)d[0]* (float)d[0] + (float)d[1]* (float)d[1] + (float)d[2] * (float)d[2]); }
		void Normalize()
		{
			const float len = Length();
			d[0] = d[0] / len;
			d[1] = d[1] / len;
			d[2] = d[2] / len;
		}
        Vec3<T> Normalized()
        {
            const float len = Length();
            return Vec3<T>(
                d[0] / len,
                d[1] / len,
                d[2] / len
                );
        }
		Vec3 Cross(const Vec3& a) const
		{
            return Vec3(
                d[1] * a.d[2] - d[2] * a.d[1],
                d[2] * a.d[0] - d[0] * a.d[2],
                d[0] * a.d[1] - d[1] * a.d[0]
            );
		}
    };

	template <typename T,
		class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
		class Vec2
	{
	public:
		Vec2(T _x = 0, T _y = 0) : d{ _x, _y } {}
		T d[2];

		const T* Data() const { return d; }

		T& x() { return d[0]; }
		T x() const { return d[0]; }
		T& y() { return d[1]; }
		T y() const { return d[1]; }

		Vec2 operator+ (const Vec2& a) const { return Vec2(d[0] + a.d[0], d[1] + a.d[1]); }
		Vec2 operator- (const Vec2& a) const { return Vec2(d[0] - a.d[0], d[1] - a.d[1]); }
		Vec2 operator= (const Vec2& a) { d[0] = a.d[0]; d[1] = a.d[1]; return *this; }
		Vec2 operator* (T a) const { return Vec2(d[0]*a, d[1]*a); }
		Vec2 operator/ (T a) const { return Vec2(d[0] / a, d[1] / a); }
        bool operator== (const Vec2& a) const { return (d[0] == a.d[0] && d[1] == a.d[1]); }
        bool operator!= (const Vec2& a) const { return !(*this == a); }
        bool operator< (const Vec2& a) const { return (d[0] < a.d[0] ? true : (d[0] == a.d[0] ? (d[1] < a.d[1]) : false)); }
        bool operator> (const Vec2& a) const { return (d[0] > a.d[0] ? true : (d[0] == a.d[0] ? (d[1] > a.d[1]) : false)); }
        float LengthSqr() const { return (float)d[0] * (float)d[0] + (float)d[1] * (float)d[1]; }
		float Length() const { return sqrtf((float)d[0]* (float)d[0] + (float)d[1]* (float)d[1]); }
		void Normalize()
		{
			const float len = Length();
			d[0] = d[0] / len;
			d[1] = d[1] / len;
		}
        Vec2<T> Normalized()
        {
            const float len = Length();
            return Vec2<T>(
                d[0] / len,
                d[1] / len
                );
        }
	};
}