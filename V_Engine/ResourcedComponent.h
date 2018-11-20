#ifndef __RESOURCED_COMPONENT__
#define __RESOURCED_COMPONENT__

#include "Component.h"
#include "ModuleResourceManager.h"


class ResourcedComponent : public Component
{
public:
	ResourcedComponent(Gameobject* linkedTo, ComponentType type) : Component(linkedTo, type)
	{}

	~ResourcedComponent()
	{ UnLinkResource(); }

	void LoadResource(std::string resourceToLoad);

	template <typename T>
	T* ReadRes() const
	{ return (T*)App->m_resourceManager->Peek(m_resource); }

	virtual bool IsResourceMissing() const override { return (m_resource == 0); }

protected:
	unsigned long long m_resource = 0;

private:
	void LinkResource(std::string fileName);
	void UnLinkResource();
};

#endif