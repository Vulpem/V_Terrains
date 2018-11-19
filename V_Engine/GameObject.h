#ifndef __GO__
#define __GO__

#define NAME_MAX_LEN 255

#include "Globals.h"

#include<vector>
#include "Math.h"

#include "Component.h"
#include "Transform.h"

class Gameobject
{
public:
	Gameobject();
	Gameobject(uint64_t Uid);

	~Gameobject();

	const uint64_t GetUID() const { return m_uid; }

	void DrawAttributeEditorContent();

	void PositionChanged();

	void SetActive(bool state, bool justPublic = false);
	bool IsActive() const;

	void SetName(const char* newName);
	const char* GetName() const;

	Component* CreateComponent(ComponentType type, std::string resource = std::string(""), bool forceCreation = false);

	template <typename typeComp>
	bool HasComponent() const;
	Transform* GetTransform();
	const Transform* GetTransform() const;

	void Save(pugi::xml_node& node) const;

	void RemoveComponent(Component* comp);

	template <typename typeComp>
	void GetComponents(std::vector<typeComp*>& out) const;

	template <typename typeComp>
	typeComp* GetComponent() const;

	AABB GetObjectSpaceAABB() const;
	AABB GetAABB() const;
	OBB GetOBB() const;
	void DrawAABB() const;
	void DrawOBB() const;

	bool IsSelected() const;

	char m_name[NAME_MAX_LEN];
//TODO move everything that can be in a component in there, clean GameObject

	std::vector<Component*> m_components;
private:
	uint64_t m_uid;
	Transform m_transform;

	bool m_active = true;
	bool m_publicActive = true;
};

template <typename typeComp>
void Gameobject::GetComponents(std::vector<typeComp*>& out) const
{
	for (Component* component : m_components)
	{
		typeComp* toReturn = dynamic_cast<typeComp*>(component);
		if (toReturn != nullptr)
		{
			out.push_back(toReturn);
		}
	}
}

template <typename typeComp>
typeComp* Gameobject::GetComponent() const
{
	for (Component* component : m_components)
	{
		//TODO look into typeID(typeComp)
		typeComp* toReturn = dynamic_cast<typeComp*>(component);
		if (toReturn != nullptr)
		{
			return toReturn;
		}
	}
	return nullptr;
}

template <typename typeComp>
bool Gameobject::HasComponent() const
{
	return (GetComponent<typeComp>() != nullptr);
}
#endif