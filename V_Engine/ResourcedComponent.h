#ifndef __RESOURCED_COMPONENT__
#define __RESOURCED_COMPONENT__

#include "Component.h"
#include "ModuleResourceManager.h"


class ResourcedComponent : public Component
{
public:
	ResourcedComponent(std::string resource, GameObject* linkedTo, ComponentType type) : Component(linkedTo, type)
	{
		LinkResource(resource);
	}

	~ResourcedComponent()
	{
		UnLinkResource();
	}

	template <typename T>
	T* ReadRes() const { return (T*)App->m_resourceManager->Peek(resource); }

	virtual bool MissingComponent() { return (resource == 0); }

protected:
	unsigned long long resource = 0;

private:
	void LinkResource(std::string fileName);
	void UnLinkResource();
};

#endif