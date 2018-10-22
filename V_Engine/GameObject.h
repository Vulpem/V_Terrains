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

	Component* AddComponent(Component::Type type, std::string res = std::string(""), bool forceCreation = false);

	bool HasComponent(Component::Type type);
	uint AmountOfComponent(Component::Type type);
	Transform* GetTransform();

	void Delete();

	void Save(pugi::xml_node& node);

	//For system use, do not call
	void RemoveComponent(Component* comp);

#pragma region GetComponents
	//GetComponent function
	template <typename typeComp>
	std::vector<typeComp*> GetComponent()
	{
		std::vector<typeComp*> ret;
		if (HasComponent(typeComp::GetType()))
		{
			std::vector<Component*>::iterator it = m_components.begin();
			while (it != m_components.end())
			{
				//Remember to add a "static GetType()" function to all created components
				if ((*it)->GetType() == typeComp::GetType())
				{
					ret.push_back((typeComp*)(*it));
				}
				it++;
			}
		}
		return ret;
	}
#pragma endregion

public:
	char m_name[NAME_MAX_LEN];
	AABB m_aabb;
	OBB m_obb;

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

	static const uint m_nComponentTypes = Component::Type::C_None;
	int m_hasComponents[Component::Type::C_None];

	AABB m_originalAABB;

	Transform* m_transform = nullptr;

};

#endif