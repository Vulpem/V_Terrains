
#ifndef __LIGHT__
#define __LIGHT__

#include "Color.h"

#include "Math.h"

struct Light
{
	Light();

	void Init();
	void SetPos(float x, float y, float z);
	void Active(bool active);
	void Render();

	Color m_ambient;
	Color m_diffuse;
	math::float3 m_position;

	int m_ref;
	bool m_on;
};

#endif