
#ifndef __COLOR__
#define __COLOR__

struct Color
{
	float m_r, m_g, m_b, m_a;
	
	Color() : m_r(0.0f), m_g(0.0f), m_b(0.0f), m_a(1.0f)
	{}

	Color(float r, float g, float b, float a = 1.0f) : m_r(r), m_g(g), m_b(b), m_a(a)
	{}

	void Set(float r, float g, float b, float a = 1.0f)
	{
		m_r = r;
		m_g = g;
		m_b = b;
		m_a = a;
	}

	float* Ptr() { return &m_r; }
};

extern Color Red;
extern Color Green;
extern Color Blue;
extern Color Black;
extern Color White;

#endif