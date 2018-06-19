#ifndef __MODULE_TERRAIN_GAME__
#define __MODULE_TERRAIN_GAME__

#include "Module.h"
#include "Globals.h"

#include "../V_Terrain/Code/Include.h"

#include "Math.h"

class GameObject;

class ModuleTerrainGame : public Module
{
public:
	ModuleTerrainGame(Application* app, bool start_enabled = true);
	~ModuleTerrainGame();

	bool Init();
	bool Start();
	update_status PreUpdate() override;
    update_status Update() override;
	update_status PostUpdate() override;
	bool CleanUp();

	void Render(const viewPort& port) override;

	GameObject* player;
};

#endif