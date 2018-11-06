#ifndef __GO__
#define __GO__

#define NAME_MAX_LEN 1024

#include "Globals.h"

#include<vector>
#include "Math.h"

#include "Component.h"
#include "Mesh.h"

class Transform;

class GameObject
{
public:
	GameObject();
	//Force the gameobject to have a certain UID. For loading purposes
	GameObject(uint64_t Uid);

	~GameObject();

	const uint64_t GetUID() { return m_uid; }

	void DrawOnEditor();
	void DrawLocator();
	void DrawAABB();
	void DrawOBB();

	//Be wary, deactivate this only for objects that the editor will take care of by itself. You won't be able to access them during runtime
	//TODO 
	void HideFromOutliner() { m_hiddenOnOutliner = true; }
	bool HiddenFromOutliner() { return m_hiddenOnOutliner; }

	void Select(bool renderNormals = false);
	void Unselect();

	void SetOriginalAABB();
	void UpdateAABB();

	void UpdateTransformMatrix();

	void SetActive(bool state, bool justPublic = false);
	bool IsActive();

	void SetStatic(bool Stat) { m_static = Stat; }
	bool IsStatic() { return m_static; }

	void SetName(const char* newName);
	const char* GetName();

	Component* AddComponent(ComponentType type, std::string res = std::string(""), bool forceCreation = false);

	template <typename typeComp>
	bool HasComponent();
	uint AmountOfComponent(ComponentType type);
	Transform* GetTransform();

	void Delete();

	void Save(pugi::xml_node& node);

	void RemoveComponent(Component* comp);

	template <typename typeComp>
	std::vector<typeComp*> GetComponents();

	template <typename typeComp>
	typeComp* GetComponent();

public:
	char m_name[NAME_MAX_LEN];
	AABB m_aabb;
	OBB m_obb;
//TODO move everything that can be in a component in there, clean GameObject
	std::vector<GameObject*> m_childs;
	GameObject* m_parent = nullptr;

	std::vector<Component*> m_components;
	bool m_selected = false;
	bool m_drawNormals = false;
	bool m_beingRendered = false;
private:
	uint64_t m_uid;

	bool m_active = true;
	bool m_publicActive = true;
	bool m_hiddenOnOutliner = false;
	bool m_static = false;

	static const uint m_nComponentTypes = (int)ComponentType::none;
	std::map<ComponentType, int> m_hasComponents;

	AABB m_originalAABB;

	Transform* m_transform = nullptr;
};

template <typename typeComp>
//TODO
//Pass by reference vector
std::vector<typeComp*> GameObject::GetComponents()
{
	std::vector<typeComp*> ret;
	for (Component* component : m_components)
	{
		typeComp* toReturn = dynamic_cast<typeComp*>(component);
		if (toReturn != nullptr)
		{
			ret.push_back(toReturn);
		}
	}
	return ret;
}

template <typename typeComp>
typeComp* GameObject::GetComponent()
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
bool GameObject::HasComponent()
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