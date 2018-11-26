#ifndef __MODULE_SCENE_MANAGER_3D__
#define __MODULE_SCENE_MANAGER_3D__

#include "Module.h"
#include "Globals.h"
#include "Scene.h"

class ModuleSceneManager : public Module
{
public:
	ModuleSceneManager();
	~ModuleSceneManager();

	bool Init() override;
	UpdateStatus PreUpdate() override;
	UpdateStatus Update() override;
	UpdateStatus PostUpdate() override;

	void OnEnable() override;
	void OnDisable() override;

	void Render(const ViewPort& port) const override;

private:
	std::vector<Scene> m_scenes;
};

#endif