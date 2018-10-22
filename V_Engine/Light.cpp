#include "Globals.h"
#include "Light.h"
#include "OpenGL.h"

//#include <gl/GLU.h>

Light::Light() : m_ref(-1), m_on(false), m_position(0.0f, 0.0f, 0.0f)
{}

void Light::Init()
{
	glLightfv(m_ref, GL_AMBIENT, &m_ambient);
	glLightfv(m_ref, GL_DIFFUSE, &m_diffuse);
}

void Light::SetPos(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
}

void Light::Render()
{
	if(m_on)
	{
		float pos[] = { m_position.x, m_position.y, m_position.z, 1.0f};
		glLightfv(m_ref, GL_POSITION, pos);
	}
}

void Light::Active(bool active)
{
	if(m_on != active)
	{
		m_on = !m_on;

		if(m_on)
			glEnable(m_ref);
		else
			glDisable(m_ref);
	}
}