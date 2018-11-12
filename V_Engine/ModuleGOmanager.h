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

class ModuleGoManager : public Module
{
public:
	
	// ----------------------- Module Defaults ------------------------------------------------------

	ModuleGoManager(Application* app, bool start_enabled = true);
	~ModuleGoManager();

	bool Init() override;
	UpdateStatus PreUpdate() override;
	UpdateStatus Update() override;
	UpdateStatus PostUpdate() override;

	void Render(const ViewPort& port) const override;

	void CleanUp();


	// ----------------------- GO Management ------------------------------------------------------
private:
	GameObject* m_root = nullptr;
public:
	//Create an empty GameObject
	GameObject* CreateEmpty(const char* name = NULL);
	//Create a gameobject with just a Camera attached to it
	GameObject* CreateCamera(const char* name = NULL);
	//Create a copy of the passed GO
	GameObject* DuplicateGO(GameObject* toCopy);

	//Load a vGO avaliable in the resources
	std::vector<GameObject*> LoadGO(const char* file_noFormat);

	//Delete a GameObject
	bool DeleteGameObject(GameObject* toErase);

	//Set a single GO to the passed Static value
	void SetStatic(bool Static, GameObject* GO);
	//Set a GO and all his childs to the passed Static value
	void SetChildsStatic(bool Static, GameObject* GO);


	// ----------------------- Scene Management ------------------------------------------------------

	void SaveScene(char* name) { m_wantToSaveScene = true; m_sceneName = name; }
	void LoadScene(char* name) { m_wantToLoadScene = true; m_sceneName = name;	m_wantToClearScene = true; }
	void ClearScene() { m_wantToClearScene = true; }

private:
	std::string m_sceneName;
	bool m_wantToSaveScene = false;
	bool m_wantToLoadScene = false;
	bool m_wantToClearScene = false;
	void SaveSceneNow();
	void LoadSceneNow();
	void ClearSceneNow();
public:

	// ----------------------- UTILITY ------------------------------------------------------

	template <typename C>
	//Returns a vector of all the GOs that collided with the shape passed
	std::vector<GameObject*> FilterCollisions(C col);

	/*Check if the ray collides with any GameObject
	-return bool: wether if the ray collided with something or not
	-OUT_Gameobject: gameobject the ray collided with. If there's none, nullptr is returned
	-OUT_position: the position where the ray collided. If it didn't, it will return (-1,-1).
	-OUT_normal: the direction of the normal of the surface where the ray collided. If it didn't, it will return (-1,-1).*/
	bool RayCast(const LineSegment& ray, GameObject** OUT_gameobject = NULL, float3* OUT_position = NULL, float3* OUT_normal = NULL, bool onlyMeshes = true);

	//Returns the root GO. Only read
	GameObject* GetRoot() { return m_root; }

	//Render all the GameObjects onto a viewport.
	//If "exclusiveGOs" vector is empty, all visible GOs will be rendered. Otherwise, only the passed objects will be rendered
	void RenderGOs(const ViewPort& ViewPort);
private:
	//Get all the info necessary to render a mesh
	Mesh_RenderInfo GetMeshData(Mesh* getFrom);

	void AddGOtoRoot(GameObject* GO);
	void CreateRootGameObject();
	void DeleteGOs();
	// TODO make a vector
	std::vector<GameObject*> m_toDelete;

public:
	//Map with all the components of all the GOs in the scene
	std::multimap<ComponentType, Component*> m_components;

	// ----------------------- Collision filtering ------------------------------------------------------

	Quad_Tree m_quadTree;
	bool m_drawQuadTree = false;
	std::vector<GameObject*> m_dynamicGO;


	// -------- UI TMP STUFF ------------
	//TODO: wth is this?
	GameObject* m_setting = nullptr;
	bool m_settingStatic = true;
private:
	bool m_staticChildsPopUpIsOpen = false;
};


template<typename C>
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
