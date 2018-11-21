#ifndef __MODULE__
#define __MODULE__

class Application;
struct PhysBody3D;
struct ViewPort;

#include "PugiXml\src\pugixml.hpp"
#include "Globals.h"

class Module
{
public:
	Module()
		: m_enabled(false)
	{}

	virtual ~Module() {}

	bool IsEnabled() const
	{ return m_enabled; }

	void Enable()
	{
		if (m_enabled == false)
		{
			m_enabled = true;
			OnEnable();
		}
	}

	void Disable()
	{
		if (m_enabled == true)
		{
			m_enabled = false;
			OnDisable();
		}
	}

	virtual bool Init() 
	{ return true; }

	virtual UpdateStatus PreUpdate()
	{ return UpdateStatus::Continue; }

	virtual UpdateStatus Update()
	{ return UpdateStatus::Continue; }

	virtual void Render(const ViewPort& port) const {}

	virtual UpdateStatus PostUpdate()
	{ return UpdateStatus::Continue; }

	virtual void OnEnable() {}
	virtual void OnDisable() {}

	virtual void OnPlay() {}
	virtual void OnStop() {}

	virtual void OnCollision(PhysBody3D* body1, PhysBody3D* body2) {}

	virtual void OnScreenResize(int width, int heigth) {  };
private:
	bool m_enabled;
};

#endif