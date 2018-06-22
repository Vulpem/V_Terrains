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

	player.Init(App->GO->LoadGO("Assets/Spaceships/MK6/MK6.fbx").front());
	player.ship->GetTransform()->SetLocalScale(0.05f, 0.05f, 0.05f);
	return ret;
}

// PreUpdate: clear buffer
update_status ModuleTerrainGame::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModuleTerrainGame::Update()
{
	if (Time.PlayMode || DEBUG_KEYS)
	{
		Input();
		UpdateBullets();
		UpdateTurrets();
		UpdatePlayer();
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
}

void ModuleTerrainGame::OnChunkLoad(int x, int y)
{
	Building* build = nullptr;
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

void ModuleTerrainGame::OnChunkUnload(int x, int y)
{
	auto turret = turrets.find(std::make_pair(x, y));
	if (turret != turrets.end())
	{
		delete turret->second;
		turrets.erase(turret);
	}
}
