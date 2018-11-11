#ifndef __GO__
#define __GO__

#define NAME_MAX_LEN 1024

#include "Globals.h"

#include<vector>
#include "Math.h"

#include "Component.h"
#include "Transform.h"

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
	Transform& GetTransform();

	void Delete();

	void Save(pugi::xml_node& node);

	void RemoveComponent(Component* comp);

	template <typename typeComp>
	void GetComponents(std::vector<typeComp*>& out);

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
	Transform m_transform;

	bool m_active = true;
	bool m_publicActive = true;
	bool m_hiddenOnOutliner = false;
	bool m_static = false;

	AABB m_originalAABB;
};

template <typename typeComp>
void GameObject::GetComponents(std::vector<typeComp*>& out)
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