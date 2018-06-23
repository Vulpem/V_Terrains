#ifndef __MODULE_TERRAIN_GAME__
#define __MODULE_TERRAIN_GAME__

#include "Module.h"
#include "Globals.h"
#include "Turret.h"
#include "Ship.h"
#include "Bullet.h"

#include "../V_Terrain/Code/Include.h"

#include "Math.h"
#include <map>

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

	void UpdateTurrets();
	void UpdateBullets();
	void UpdatePlayer();
	void Input();
	void DebugKeys();

	void OnChunkLoad(int x, int y);
	void OnChunkUnload(int x, int y);

	void InitBullets();
	void SpawnBullet(float3 pos, float3 dir);

    std::map<std::pair<int, int>, Building*> turrets;
	std::vector<Bullet> bullets;
	int bulletN = 0;
	Ship player;
	bool debugTurrets = false;
};

#endif