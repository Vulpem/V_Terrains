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
	Module(Application* app, bool start_enabled = true)
		: App(app)
		, m_enabled(start_enabled)
	{}

	virtual ~Module()
	{}

	bool IsEnabled() const
	{ return m_enabled; }

	void Enable()
	{
		if (m_enabled == false)
		{
			m_enabled = true;
			Start();
		}
	}

	void Disable()
	{
		if (m_enabled == true)
		{
			m_enabled = false;
			CleanUp();
		}
	}

	virtual bool Init() 
	{ return true; }

	virtual void Start()
	{ }

	virtual UpdateStatus PreUpdate()
	{ return UpdateStatus::Continue; }

	virtual UpdateStatus Update()
	{ return UpdateStatus::Continue; }

	virtual void Render(const ViewPort& port) const
	{}

	virtual UpdateStatus PostUpdate()
	{ return UpdateStatus::Continue; }

	virtual void CleanUp()
	{ }

	virtual void OnPlay() {}
	virtual void OnStop() {}

	virtual void OnCollision(PhysBody3D* body1, PhysBody3D* body2)
	{}

	virtual void OnScreenResize(int width, int heigth) {  };

	Application* App;
private:
	bool m_enabled;
};

#endif