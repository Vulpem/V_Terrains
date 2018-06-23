#include "Application.h"
#include "ModuleTerrainGame.h"

#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleEditor.h"
#include "GameObject.h"
#include "Transform.h"
#include "AllComponents.h"
#include "ModuleGOmanager.h"

#include "ModuleInput.h"

#include "AllResources.h"
#include "AllComponents.h"

#include "GlobalFunctions.h"

#include <time.h>
#include <algorithm>

ModuleTerrainGame::ModuleTerrainGame(Application* app, bool start_enabled) :
    Module(app, start_enabled)
{
	moduleName = "ModuleTerrainGame";
}

// Destructor
ModuleTerrainGame::~ModuleTerrainGame()
{}

// Called before render is available
bool ModuleTerrainGame::Init()
{
    bool ret = true;

	return ret;
}

bool ModuleTerrainGame::Start()
{
	bool ret = true;

	RPGT::config.chunkLoaded = [this](int x, int y) { this->OnChunkLoad(x, y); };
	RPGT::config.chunkUnloaded = [this](int x, int y) { this->OnChunkUnload(x, y); };

	player.Init(App->GO->LoadGO("Assets/Spaceships/MK6/MK6.fbx").front(), App->camera->GetDefaultCam()->object);

	InitBullets();

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleTerrainGame::PreUpdate()
{
	if (setShip)
	{
		float3 pos = float3::zero;
		RPGT::GetPoint(pos.x, pos.z, pos.y);
		if (pos.y != 0)
		{
			player.controller->GetTransform()->SetGlobalPos(pos.x, pos.y + 400, pos.z);
			setShip = false;
		}
	}
	return UPDATE_CONTINUE;
}

update_status ModuleTerrainGame::Update()
{
	if (Time.PlayMode || debugTurrets)
	{
		if (Time.PlayMode)
		{
			Input();
			UpdatePlayer();
		}
		UpdateBullets();
		UpdateTurrets();
	}
	if (DEBUG_KEYS)
	{
		DebugKeys();
	}

	return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleTerrainGame::PostUpdate()
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleTerrainGame::CleanUp()
{
	return true;
}

void ModuleTerrainGame::Render(const viewPort & port)
{
	
}

void ModuleTerrainGame::UpdateTurrets()
{
	const float dt = Time.dt;
	std::for_each(turrets.begin(), turrets.end(),
		[dt](auto& turret)
	{
		turret.second->Update(dt);
	});
}

void ModuleTerrainGame::UpdateBullets()
{
	const float dt = Time.dt;
	std::for_each(bullets.begin(), bullets.end(),
		[dt](Bullet& b) { b.Update(dt); });
}

void ModuleTerrainGame::UpdatePlayer()
{
	player.Update(Time.dt);
}

void ModuleTerrainGame::Input()
{
}

void ModuleTerrainGame::DebugKeys()
{
	if (App->input->GetKey(SDL_SCANCODE_KP_0) == KEY_DOWN)
	{
		Time.PlayMode = !Time.PlayMode;
	}
	if (App->input->GetKey(SDL_SCANCODE_KP_1) == KEY_DOWN)
	{
		debugTurrets = !debugTurrets;
	}
}

void ModuleTerrainGame::OnChunkLoad(int x, int y)
{
	Building* build = nullptr;
	if (math::Abs(x) + math::Abs(y) > 5)
	{
		if (std::rand() % 1000 < 5)
		{
			build = new Building(App->GO->LoadGO("Assets/Tower/Tower1.fbx").front(), x, y);
			float3 p = build->base->GetTransform()->GetGlobalPos();
			p.y -= 60.f;
			build->base->GetTransform()->SetGlobalPos(p);
		}
		else if (std::rand() % 100 < 20)
		{
			build = new Turret(App->GO->LoadGO("Assets/Turrets/turret/turret.fbx").front(), x, y);
		}

		if (build != nullptr)
		{
			turrets[std::make_pair(x, y)] = build;
		}
	}
}

void ModuleTerrainGame::OnChunkUnload(int x, int y)
{
	auto turret = turrets.find(std::make_pair(x, y));
	if (turret != turrets.end())
	{
		delete turret->second;
		turrets.erase(turret);
	}
}

void ModuleTerrainGame::InitBullets()
{
	GameObject* bullet = App->GO->LoadGO("Assets/Turrets/Bullet/Bullet.fbx").front();
	bullet->GetTransform()->SetGlobalPos(10, -100000, 0);

	Material* mat = bullet->GetComponent<Material>().front();
	mat->SetAlphaType(AlphaTestTypes::ALPHA_BLEND);
	mat->SetAlphaTest(0.63f);
	mat->SetColor(0.9137, 0.074, 0.074, 0.9137);

	mat = bullet->childs[0]->GetComponent<Material>().front();
	mat->SetColor(1, 0, 0);
	mat->ReadRes<R_Material>()->AssignShader("bullet2");

	mat = bullet->childs[1]->GetComponent<Material>().front();
	mat->SetColor(0.9, 0, 1);
	mat->SetAlphaType(AlphaTestTypes::ALPHA_DISCARD);
	mat->SetAlphaTest(0.4f);
	mat->ReadRes<R_Material>()->AssignShader("bullet");

	bullets.resize(512);
	bulletN = 0;
}

void ModuleTerrainGame::SpawnBullet(float3 pos, float3 dir)
{
	if (bulletN >= bullets.size())
	{
		bulletN = 0;
	}
	bullets[bulletN++].Spawn(pos, dir);
}
