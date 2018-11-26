#ifndef __MODULE_GEOMETRY__
#define __MODULE_GEOMETRY__

#include "Module.h"
#include "Globals.h"

#include "Math.h"
#include <vector>
#include <stack>
#include <map>
#include "QuadTree.h"

#include "GameObject.h"
#include "Mesh.h"

struct ViewPort;

//TODO this needs to be separated into a Component Manager and a Scene manager. And actually create a "scene" class or concept
class ModuleGoManager : public Module
{
public:
	
	// ----------------------- Module Defaults ------------------------------
	ModuleGoManager();
	~ModuleGoManager();

	UpdateStatus PreUpdate() override;
	UpdateStatus Update() override;
	UpdateStatus PostUpdate() override;

	Gameobject* CreateGameobject(const char* name = NULL);
	Gameobject* CreateCamera(const char* name = NULL);

	void DeleteGameObject(Gameobject* toErase);
	void DeleteComponent(Component* toErase);

	void SetStatic(bool Static, Gameobject* GO);
	void SetChildsStatic(bool Static, Gameobject* GO);

	void RenderGOs(const ViewPort& ViewPort) const;

	std::vector<Gameobject*> LoadGO(const char* file_noFormat);

	template<typename CompType>
	std::vector<CompType*> GetComponentsByType(ComponentType type) const;

	std::multimap<ComponentType, Component*> m_components;

	//TODO: wth is this?
	Gameobject* m_setting = nullptr;
	bool m_settingStatic = true;

private:
	//TODO: move into scene manager
	void SaveSceneNow();
	void LoadSceneNow();

	Mesh_RenderInfo GetMeshData(Mesh* getFrom) const;
	void DeleteGOs();

	std::vector<Gameobject*> m_toDelete;

	bool m_staticChildsPopUpIsOpen = false;
};


template<typename C>
//Returns a vector of all the GOs that collided with the shape passed
inline std::vector<Gameobject*> ModuleGoManager::FilterCollisions(C col) const
{
	std::vector<Gameobject*> ret = m_quadTree.FilterCollisions(col);

	for (auto go : m_dynamicGO)
	{
		AABB goAABB = go->GetAABB();
		if (go->IsActive() && goAABB.IsFinite() && goAABB.Intersects(col))
		{
			ret.push_back(go);
		}
	}
	return ret;
}

//TODO remove this function and the double ComponentType sending
template<typename CompType>
inline std::vector<CompType*> ModuleGoManager::GetComponentsByType(ComponentType type) const
{
	std::vector<CompType*> toReturn;
	for (auto iterator = m_components.find(type); iterator != m_components.end() && iterator->second->GetType() == type; iterator++)
	{
		toReturn.push_back(dynamic_cast<CompType*>(iterator->second));
	}
	return toReturn;
}

#endif