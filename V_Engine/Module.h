#ifndef __MODULE__
#define __MODULE__

class Application;
struct PhysBody3D;
struct ViewPort;

#include "PugiXml\src\pugixml.hpp"
#include "Globals.h"

class Module
{
private :
	bool enabled;

protected:
	std::string moduleName;

public:
	Application* App;

	Module(Application* parent, bool start_enabled = true) : App(parent)
	{
		enabled = start_enabled;
	}

	virtual ~Module()
	{}

	bool IsEnabled() const
	{
		return enabled;
	}

	void Enable()
	{
		if (enabled == false)
		{
			enabled = true;
			Start();
		}
	}

	bool Disable()
	{
		bool ret = true;
		if (enabled == true)
		{
			enabled = false;
			ret = CleanUp();
		}
		return ret;
	}

	virtual bool Init() 
	{
		return true; 
	}

	virtual bool Start()
	{
		return true;
	}

	virtual UpdateStatus PreUpdate()
	{
		return UPDATE_CONTINUE;
	}

	virtual UpdateStatus Update()
	{
		return UPDATE_CONTINUE;
	}

	virtual void Render(const ViewPort& port)
	{

	}

	virtual UpdateStatus PostUpdate()
	{
		return UPDATE_CONTINUE;
	}

	virtual bool CleanUp() 
	{ 
		enabled = false;
		return true; 
	}

	virtual void OnPlay() {}
	virtual void OnStop() {}

	std::string GetName()
	{
		return moduleName;
	}

	virtual void OnCollision(PhysBody3D* body1, PhysBody3D* body2)
	{}

	virtual void OnScreenResize(int width, int heigth) {  };
};

#endif