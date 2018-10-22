#ifndef __PhysBody3D_H__
#define __PhysBody3D_H__

#include <list>

class btRigidBody;
class Module;

// =================================================
struct PhysBody3D
{
	friend class ModulePhysics3D;
public:
	PhysBody3D(btRigidBody* body);
	~PhysBody3D();

	void Push(float x, float y, float z);
	void GetTransform(float* matrix) const;
	void SetTransform(const float* matrix) const;
	void SetPos(float x, float y, float z);
	void GetPos(float* x, float* y, float* z);
	void SetAsSensor(bool is_sensor);
	bool IsSensor() const;
	bool IsResting();
	void SetInactive();

public:
	std::list<Module*> m_collisionListeners;

private:
	btRigidBody* m_body = nullptr;
	bool m_isSensor = false;


};

#endif // __PhysBody3D_H__