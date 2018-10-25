#include "PhysBody3D.h"
#include "Bullet\include\btBulletDynamicsCommon.h"

// =================================================
PhysBody3D::PhysBody3D(btRigidBody* body) : m_body(body)
{
	body->setUserPointer(this);
}

// ---------------------------------------------------------
PhysBody3D::~PhysBody3D()
{
	delete m_body;
}

// ---------------------------------------------------------
void PhysBody3D::Push(float x, float y, float z)
{
	m_body->applyCentralImpulse(btVector3(x, y, z));
}

// ---------------------------------------------------------
void PhysBody3D::GetTransform(float* matrix) const
{
	if(m_body != nullptr && matrix != NULL)
	{
		m_body->getWorldTransform().getOpenGLMatrix(matrix);
	}
}

// ---------------------------------------------------------
void PhysBody3D::SetTransform(const float* matrix)
{
	if(m_body != nullptr && matrix != NULL)
	{
		btTransform t;
		t.setFromOpenGLMatrix(matrix);
		m_body->setWorldTransform(t);
	}
}

// ---------------------------------------------------------
void PhysBody3D::SetPos(float x, float y, float z)
{
	btTransform t = m_body->getWorldTransform();
	t.setOrigin(btVector3(x, y, z));
	m_body->setWorldTransform(t);
}

void PhysBody3D::GetPos(float* x, float* y, float* z) const
{
	btTransform t = m_body->getWorldTransform();
	btVector3 pos = t.getOrigin();
	btVector3FloatData posFloat;
	pos.serializeFloat(posFloat);
	*x = posFloat.m_floats[0];
	*y = posFloat.m_floats[1];
	*z = posFloat.m_floats[2];
}

// ---------------------------------------------------------
void PhysBody3D::SetAsSensor(bool is_sensor)
{
	if(this->m_isSensor != is_sensor)
	{
		this->m_isSensor = is_sensor;
		if(is_sensor == true)
			m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
		else
			m_body->setCollisionFlags(m_body->getCollisionFlags() &~ btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}
}

// ---------------------------------------------------------
bool PhysBody3D::IsSensor() const
{
	return m_isSensor;
}

bool PhysBody3D::IsResting() const
{
	return !m_body->isActive();
}

void PhysBody3D::SetInactive()
{
	m_body->setActivationState(WANTS_DEACTIVATION);
}