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


enum class GameType
{
	Exploration,
	BulletHell
};


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

	void UpdateGame();
	void UpdateTurrets();
	void UpdateBullets();
	void UpdatePlayer();
	void Input();
	void DebugKeys();

	void OnPlay() override;
	void OnStop() override;

	void OnChunkLoad(int x, int y);
	void OnChunkUnload(int x, int y);

	void InitBullets();
	Bullet& SpawnBullet(float3 pos, float3 dir, bool playerBullet = false);

    std::map<std::pair<int, int>, Building*> turrets;
	std::vector<Bullet> bullets;
	std::vector<Bullet> playerBullets;

	int bulletN = 0;
	int playerBulletN = 0;
	Ship player;
	bool debugTurrets = false;
	bool setShip = true;

	float3 gamePos = float3::zero;
	GameType game = GameType::BulletHell;

	float verticalSpeed = 400.f;

	float3 mousePos = float3::zero;

	float cameraHeight = 1300;
};

#endif