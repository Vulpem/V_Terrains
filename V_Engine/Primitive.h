#ifndef __PRIMITIVE__
#define __PRIMITIVE__

#include "Math.h"
#include "Color.h"

enum class PrimitiveTypes
{
	Primitive_Point,
	Primitive_Line,
	Primitive_Plane,
	Primitive_Cube,
	Primitive_Sphere,
	Primitive_Cylinder,
	Primitive_Teapot
};

class Primitive
{
public:

	Primitive();

	virtual void	Render() const;
	virtual void	InnerRender() const;
	void			SetPos(float x, float y, float z);
	void			SetRotation(float angle, const math::float3 u);
	void			Scale(float x, float y, float z);
	PrimitiveTypes	GetType() const;

public:
	Color m_color;
	math::float4x4 m_transform;
	bool m_axis,m_wire;

protected:
	PrimitiveTypes m_type;
};

// ============================================
class P_Cube : public Primitive
{
public :
	P_Cube();
	P_Cube(float sizeX, float sizeY, float sizeZ);
	void InnerRender() const;
public:
	math::float3 m_size;
};

// ============================================
class P_Sphere : public Primitive
{
public:
	P_Sphere();
	P_Sphere(float radius);
	void InnerRender() const;
public:
	float m_radius;
};

// ============================================
class P_Cylinder : public Primitive
{
public:
	P_Cylinder();
	P_Cylinder(float radius, float height);
	void InnerRender() const;
public:
	float m_radius;
	float m_height;
};

// ============================================
class P_Line : public Primitive
{
public:
	P_Line();
	P_Line(float x, float y, float z);
	void InnerRender() const;
public:
	math::float3 m_a;
	math::float3 m_b;
};

// ============================================
class P_Plane : public Primitive
{
public:
	P_Plane();
	P_Plane(float x, float y, float z, float d);
	void InnerRender() const;
public:
	math::float3 m_normal;
	float m_constant;
};

#endif