
#include "Globals.h"
#include "OpenGL.h"
#include "Primitive.h"


// ------------------------------------------------------------
Primitive::Primitive() : m_transform(math::float4x4::identity), m_color(White), m_wire(false), m_axis(false), m_type(PrimitiveTypes::Primitive_Point)
{}

// ------------------------------------------------------------
PrimitiveTypes Primitive::GetType() const
{
	return m_type;
}

// ------------------------------------------------------------
void Primitive::Render() const
{
	glPushMatrix();
	glMultMatrixf(m_transform.ptr());

	if(m_axis == true)
	{
		glDisable(GL_DEPTH_TEST);
		// Draw Axis Grid
		glBegin(GL_LINES);

		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

		glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, 0.1f, 0.0f); glVertex3f(1.1f, -0.1f, 0.0f);
		glVertex3f(1.1f, 0.1f, 0.0f); glVertex3f(1.0f, -0.1f, 0.0f);

		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

		glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-0.05f, 1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
		glVertex3f(0.05f, 1.25f, 0.0f); glVertex3f(0.0f, 1.15f, 0.0f);
		glVertex3f(0.0f, 1.15f, 0.0f); glVertex3f(0.0f, 1.05f, 0.0f);

		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);

		glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-0.05f, 0.1f, 1.05f); glVertex3f(0.05f, 0.1f, 1.05f);
		glVertex3f(0.05f, 0.1f, 1.05f); glVertex3f(-0.05f, -0.1f, 1.05f);
		glVertex3f(-0.05f, -0.1f, 1.05f); glVertex3f(0.05f, -0.1f, 1.05f);

		glEnd();

		glEnable(GL_DEPTH_TEST);
	}

	glColor3f(m_color.r, m_color.g, m_color.b);

	if(m_wire)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		

	InnerRender();

	glPopMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// ------------------------------------------------------------
void Primitive::InnerRender() const
{
	glPointSize(5.0f);

	glBegin(GL_POINTS);

	glVertex3f(0.0f, 0.0f, 0.0f);

	glEnd();

	glPointSize(1.0f);
}

// ------------------------------------------------------------
void Primitive::SetPos(float x, float y, float z)
{
	m_transform.Translate(x, y, z);
}

// ------------------------------------------------------------
void Primitive::SetRotation(float angle, const math::float3 u)
{
	m_transform.RotateAxisAngle(u, angle);
}

// ------------------------------------------------------------
void Primitive::Scale(float x, float y, float z)
{
	m_transform.Scale(x, y, z);
}

// CUBE ============================================
P_Cube::P_Cube() : Primitive(), m_size(1.0f, 1.0f, 1.0f)
{
	m_type = PrimitiveTypes::Primitive_Cube;
}

P_Cube::P_Cube(float sizeX, float sizeY, float sizeZ) : Primitive(), m_size(sizeX, sizeY, sizeZ)
{
	m_type = PrimitiveTypes::Primitive_Cube;
}

void P_Cube::InnerRender() const
{	
	float sx = m_size.x * 0.5f;
	float sy = m_size.y * 0.5f;
	float sz = m_size.z * 0.5f;

	glBegin(GL_TRIANGLES);

#pragma region directMode
	//Back
	glColor4f(m_color.r, m_color.g, m_color.b, m_color.a);

	glNormal3f(0, 0, -1);
	glVertex3f(sx, sy, -sz);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f(-sx, sy, -sz);

	glVertex3f(sx, sy, -sz);
	glVertex3f(sx, -sy, -sz);
	glVertex3f(-sx, -sy, -sz);

	//Left
	glNormal3f(1, 0, 0);
	glVertex3f(sx, sy, -sz);
	glVertex3f(sx, -sy, sz);
	glVertex3f(sx, -sy, -sz);

	glVertex3f(sx, sy, -sz);
	glVertex3f(sx, sy, sz);
	glVertex3f(sx, -sy, sz);

	//Front
	glNormal3f(0, 0, 1);
	glVertex3f(sx, sy, sz);
	glVertex3f(-sx, sy, sz);
	glVertex3f(sx, -sy, sz);

	glVertex3f(-sx, sy, sz);
	glVertex3f(-sx, -sy, sz);
	glVertex3f(sx, -sy, sz);

	//Right
	glNormal3f(-1, 0, 0);
	glVertex3f(-sx, sy, sz);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f(-sx, -sy, sz);

	glVertex3f(-sx, sy, sz);
	glVertex3f(-sx, sy, -sz);
	glVertex3f(-sx, -sy, -sz);

	//Top
	glNormal3f(0, 1, 0);
	glVertex3f(sx, sy, sz);
	glVertex3f(sx, sy, -sz);
	glVertex3f(-sx, sy, sz);

	glVertex3f(-sx, sy, sz);
	glVertex3f(sx, sy, -sz);
	glVertex3f(-sx, sy, -sz);

	//Down
	glNormal3f(0, -1, 0);
	glVertex3f(sx, -sy, sz);
	glVertex3f(-sx, -sy, sz);
	glVertex3f(sx, -sy, -sz);
	
	glVertex3f(-sx, -sy, sz);
	glVertex3f(-sx, -sy, -sz);
	glVertex3f(sx, -sy, -sz);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
#pragma endregion
	glEnd();
}

// SPHERE ============================================
P_Sphere::P_Sphere() : Primitive(), m_radius(1.0f)
{
	m_type = PrimitiveTypes::Primitive_Sphere;
}

P_Sphere::P_Sphere(float radius) : Primitive(), m_radius(radius)
{
	m_type = PrimitiveTypes::Primitive_Sphere;
}

void P_Sphere::InnerRender() const
{
	//glutSolidSphere(radius, 25, 25);
}


// CYLINDER ============================================
P_Cylinder::P_Cylinder() : Primitive(), m_radius(1.0f), m_height(1.0f)
{
	m_type = PrimitiveTypes::Primitive_Cylinder;
}

P_Cylinder::P_Cylinder(float radius, float height) : Primitive(), m_radius(radius), m_height(height)
{
	m_type = PrimitiveTypes::Primitive_Cylinder;
}

void P_Cylinder::InnerRender() const
{
	int n = 30;

	// Cylinder Bottom
	glBegin(GL_POLYGON);
	
	for(int i = 360; i >= 0; i -= (360 / n))
	{
		float a = i * math::pi / 180; // degrees to radians
		glVertex3f(-m_height*0.5f, m_radius * cos(a), m_radius * sin(a));
	}
	glEnd();

	// Cylinder Top
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, 1.0f);
	for(int i = 0; i <= 360; i += (360 / n))
	{
		float a = i * math::pi / 180; // degrees to radians
		glVertex3f(m_height * 0.5f, m_radius * cos(a), m_radius * sin(a));
	}
	glEnd();

	// Cylinder "Cover"
	glBegin(GL_QUAD_STRIP);
	for(int i = 0; i < 480; i += (360 / n))
	{
		float a = i * math::pi / 180; // degrees to radians

		glVertex3f(m_height*0.5f,  m_radius * cos(a), m_radius * sin(a) );
		glVertex3f(-m_height*0.5f, m_radius * cos(a), m_radius * sin(a) );
	}
	glEnd();
}

// LINE ==================================================
P_Line::P_Line() : Primitive(), m_a(0, 0, 0), m_b(1, 1, 1)
{
	m_type = PrimitiveTypes::Primitive_Line;
}

P_Line::P_Line(float x, float y, float z) : Primitive(), m_a(0, 0, 0), m_b(x, y, z)
{
	m_type = PrimitiveTypes::Primitive_Line;
}

void P_Line::InnerRender() const
{
	glBegin(GL_LINES);

	glVertex3f(m_a.x, m_a.y, m_a.z);
	glVertex3f(m_b.x, m_b.y, m_b.z);

	glEnd();
}

// PLANE ==================================================
P_Plane::P_Plane() : Primitive(), m_normal(0, 1, 0), m_constant(1)
{
	m_type = PrimitiveTypes::Primitive_Plane;
}

P_Plane::P_Plane(float x, float y, float z, float d) : Primitive(), m_normal(x, y, z), m_constant(d)
{
	m_type = PrimitiveTypes::Primitive_Plane;
}

void P_Plane::InnerRender() const
{
	glBegin(GL_LINES);

	float d = 20.0f;

	for(float i = -d; i <= d; i += 1.0f)
	{
		glVertex3f(i, 0.0f, -d);
		glVertex3f(i, 0.0f, d);
		glVertex3f(-d, 0.0f, i);
		glVertex3f(d, 0.0f, i);
	}

	glEnd();
}