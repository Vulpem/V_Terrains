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

#include <time.h>

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

	//App->GO->LoadGO("Assets/Spaceships/MechaT.fbx");

	player = App->GO->LoadGO("Assets/Spaceships/MK6/MK6.fbx").front();
	player->GetTransform()->SetLocalScale(0.05f, 0.05f, 0.05f);
	player->GetTransform()->SetGlobalPos(float3(40, 0, 0));

	GameObject* turret = App->GO->LoadGO("Assets/Turrets/Turret01/Turret.fbx").front();
	return ret;
}

// PreUpdate: clear buffer
update_status ModuleTerrainGame::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModuleTerrainGame::Update()
{
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

void ModuleTerrainGame::OnChunkLoad(int x, int y)
{
	if (std::rand() % 100 < 20)
	{
		Turret turret;
		turret.base = App->GO->LoadGO("Assets/Spaceships/MK6/MK6.fbx").front();

		Transform* trans = turret.base->GetTransform();
		float sizeDif = 0.5 + (float)(std::rand() % 100) / 100.f;

		float3 pos = float3(
			x * RPGT::config.chunkSize + (std::rand()%(int)RPGT::config.chunkSize)- RPGT::config.chunkSize/2.f,
			RPGT::config.maxHeight,
			y * RPGT::config.chunkSize + (std::rand() % (int)RPGT::config.chunkSize) - RPGT::config.chunkSize / 2.f
		);

		RPGT::GetPoint(pos.x, pos.z, pos.y);
		trans->SetGlobalPos(pos);
		trans->SetLocalScale(0.1f * sizeDif, 0.1f * sizeDif, 1.f * sizeDif);
		trans->SetGlobalRot(90, 0, 0);

		turrets[std::make_pair(x, y)] = turret;
	}
}

void ModuleTerrainGame::OnChunkUnload(int x, int y)
{
	auto turret = turrets.find(std::make_pair(x, y));
	if (turret != turrets.end())
	{
		turret->second.base->Delete();
		turrets.erase(turret);
	}
}
