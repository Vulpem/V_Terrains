#ifndef __GO__
#define __GO__

#define NAME_MAX_LEN 255

#include "Globals.h"

#include<vector>
#include "Math.h"

#include "Component.h"
#include "Transform.h"

class GameObject
{
public:
	GameObject();
	GameObject(uint64_t Uid);

	~GameObject();

	const uint64_t GetUID() { return m_uid; }

	void DrawOnEditor();
	void DrawAABB();
	void DrawOBB();

	//Be wary, deactivate this only for objects that the editor will take care of by itself. You won't be able to access them during runtime
	void HideFromOutliner() { m_hiddenOnOutliner = true; }
	bool HiddenFromOutliner() { return m_hiddenOnOutliner; }

	void Select();
	void Unselect();

	void UpdateTransformMatrix();

	void SetActive(bool state, bool justPublic = false);
	bool IsActive();

	void SetStatic(bool Static) { m_static = Static; }
	bool IsStatic() const { return m_static; }

	void SetName(const char* newName);
	const char* GetName() const;

	Component* CreateComponent(ComponentType type, std::string resource = std::string(""), bool forceCreation = false);

	template <typename typeComp>
	bool HasComponent() const;
	Transform* GetTransform();
	const Transform* GetTransform() const;

	void Save(pugi::xml_node& node);

	void RemoveComponent(Component* comp);

	template <typename typeComp>
	void GetComponents(std::vector<typeComp*>& out) const;

	template <typename typeComp>
	typeComp* GetComponent() const;

	AABB GetObjectSpaceAABB() const;
	AABB GetAABB() const;
	OBB GetOBB() const;
	char m_name[NAME_MAX_LEN];
//TODO move everything that can be in a component in there, clean GameObject

	std::vector<Component*> m_components;
	bool m_selected = false;
private:
	uint64_t m_uid;
	Transform m_transform;

	bool m_active = true;
	bool m_publicActive = true;
	bool m_hiddenOnOutliner = false;
	bool m_static = false;
};

template <typename typeComp>
void GameObject::GetComponents(std::vector<typeComp*>& out) const
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
typeComp* GameObject::GetComponent() const
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
bool GameObject::HasComponent() const
{
	for (Component* component : m_components)
	{
		if (dynamic_cast<typeComp*>(component) != nullptr)
		{
			return true;
		}
	}
	return false;
}
#endif