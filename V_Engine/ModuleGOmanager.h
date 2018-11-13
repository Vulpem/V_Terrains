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
	ModuleGoManager(Application* app, bool start_enabled = true);
	~ModuleGoManager();

	bool Init() override;
	UpdateStatus PreUpdate() override;
	UpdateStatus Update() override;
	UpdateStatus PostUpdate() override;

	void Render(const ViewPort& port) const override;
	void CleanUp();


	GameObject* CreateEmpty(const char* name = NULL);
	GameObject* CreateCamera(const char* name = NULL);
	bool DeleteGameObject(GameObject* toErase);

	void SetStatic(bool Static, GameObject* GO);
	void SetChildsStatic(bool Static, GameObject* GO);

	void SaveScene(char* name) { m_wantToSaveScene = true; m_sceneName = name; }
	void LoadScene(char* name) { m_wantToLoadScene = true; m_sceneName = name;	m_wantToClearScene = true; }
	void ClearScene() { m_wantToClearScene = true; }

	template <typename C>
	std::vector<GameObject*> FilterCollisions(C col);
	bool RayCast(const LineSegment& ray, GameObject** OUT_gameobject = NULL, float3* OUT_position = NULL, float3* OUT_normal = NULL, bool onlyMeshes = true);

	GameObject* GetRoot() { return m_root; }

	void RenderGOs(const ViewPort& ViewPort);

	std::vector<GameObject*> LoadGO(const char* file_noFormat);

	std::multimap<ComponentType, Component*> m_components;
	Quad_Tree m_quadTree;
	bool m_drawQuadTree = false;
	std::vector<GameObject*> m_dynamicGO;

	//TODO: wth is this?
	GameObject* m_setting = nullptr;
	bool m_settingStatic = true;

private:
	void SaveSceneNow();
	void LoadSceneNow();
	void ClearSceneNow();

	Mesh_RenderInfo GetMeshData(Mesh* getFrom);

	void AddGOtoRoot(GameObject* GO);
	void CreateRootGameObject();
	void DeleteGOs();

	std::vector<GameObject*> m_toDelete;

	GameObject * m_root = nullptr;

	std::string m_sceneName;
	bool m_wantToSaveScene = false;
	bool m_wantToLoadScene = false;
	bool m_wantToClearScene = false;

	bool m_staticChildsPopUpIsOpen = false;
};


template<typename C>
//Returns a vector of all the GOs that collided with the shape passed
inline std::vector<GameObject*> ModuleGoManager::FilterCollisions(C col)
{
	std::vector<GameObject*> ret;// = quadTree.FilterCollisions(col);
	ret.reserve(m_dynamicGO.size());

	for (std::vector<GameObject*>::iterator it = m_dynamicGO.begin(); it != m_dynamicGO.end(); it++)
	{
		if ((*it)->IsActive() && (*it)->m_aabb.IsFinite() && (*it)->m_aabb.Intersects(col) == true)
		{
			ret.push_back(*it);
			(*it)->m_beingRendered = true;
		}
		else
		{
			(*it)->m_beingRendered = false;
		}
	}
	return ret;
}

#endif
