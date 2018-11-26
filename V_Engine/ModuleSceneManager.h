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

	void Render(const ViewPort& port) const override;

private:
	void RenderGOs(const ViewPort& port) const;

	Scene* m_activeRenderScene = nullptr;
	std::vector<Scene> m_scenes;
};

#endif