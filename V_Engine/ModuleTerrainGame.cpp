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
#include "Transform.h"

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
		float3 pos = gamePos;
		gamePos.z += (RPGT::config.maxHeight + cameraHeight);
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
	if (Time.PlayMode != Play::Stop || debugTurrets)
	{
		if (Time.PlayMode != Play::Stop)
		{
			UpdateGame();
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

void ModuleTerrainGame::UpdateGame()
{
	if (game == GameType::BulletHell)
	{
		float3 p = gamePos + float3(0, RPGT::config.maxHeight + cameraHeight, -(RPGT::config.maxHeight + cameraHeight)*1.5f);
		Transform* camT = App->camera->GetDefaultCam()->object->GetTransform();
		camT->SetGlobalPos(p);
		camT->LookAt(gamePos);

		Transform* shipT = player.controller->GetTransform();
		p = shipT->GetGlobalPos();
		p.z += verticalSpeed * Time.dt;
		p.y = RPGT::config.maxHeight + 10;
		shipT->SetGlobalPos(p);

		gamePos.z += verticalSpeed * Time.dt;

			viewPort* port = nullptr;
			float2 portPos = App->renderer3D->ScreenToViewPort(float2(App->input->GetMouseX(), App->input->GetMouseY()), &port);
			//Checking the click was made on a port
			if (port != nullptr)
			{
				//Normalizing the mouse position in port to [-1,1]
				portPos.x = portPos.x / (port->size.x / 2) - 1;
				portPos.y = portPos.y / (port->size.y / 2) - 1;
				LineSegment selectRay = port->camera->GetFrustum()->UnProjectLineSegment(portPos.x, -portPos.y);
				Ray ray = selectRay.ToRay();
				float t = (player.controller->GetTransform()->GetGlobalPos().y -ray.pos.y) / ray.dir.y;
				mousePos = ray.pos + ray.dir * t;

			}

	}
}

void ModuleTerrainGame::UpdateTurrets()
{
	std::stack<std::pair<int, int>> toErase;
	const float dt = Time.dt;
	for(std::map<std::pair<int, int>, Building*>::iterator it = turrets.begin(); it != turrets.end(); it++)
	{
		it->second->Update(dt);
		if (it->second->Destroyed())
		{
			toErase.push(it->first);
		}
	}
	while (toErase.empty() == false)
	{
		std::pair<int, int> t = toErase.top();
		toErase.pop();
		auto turret = turrets.find(t);
		if (turret != turrets.end())
		{
			delete turret->second;
			turrets.erase(turret);
		}
	}
}

void ModuleTerrainGame::UpdateBullets()
{
	const float dt = Time.dt;
	std::for_each(bullets.begin(), bullets.end(),
		[dt](Bullet& b) { b.Update(dt); });
	std::for_each(playerBullets.begin(), playerBullets.end(),
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
		if (Time.PlayMode != Play::Stop)
		{
			App->Stop();
		}
		else
		{
			App->Play(false);
		}
	}
	if (App->input->GetKey(SDL_SCANCODE_KP_1) == KEY_DOWN)
	{
		if (Time.PlayMode != Play::Stop)
		{
			App->Stop();
		}
		else
		{
			App->Play(true);
		}
	}
	if (App->input->GetKey(SDL_SCANCODE_KP_2) == KEY_DOWN)
	{
		debugTurrets = !debugTurrets;
	}
}

void ModuleTerrainGame::OnPlay()
{
	gamePos = float3::zero;
	setShip = true;
}

void ModuleTerrainGame::OnStop()
{
	if (bullets.empty() == false)
	{
		std::for_each(bullets.begin(), bullets.end(),
			[](Bullet& b) {b.Despawn(); });
	}
	if(playerBullets.empty() == false)
	{
		std::for_each(playerBullets.begin(), playerBullets.end(),
			[](Bullet& b) {b.Despawn(); });
	}
}

void ModuleTerrainGame::OnChunkLoad(int x, int y)
{
	Building* build = nullptr;

	if (game != GameType::BulletHell)
	{
		if (math::Abs(x) + math::Abs(y) > 5)
		{
			if (std::rand() % 1000 < 5)
			{
				build = new Building(App->GO->LoadGO("Assets/Tower/Tower1.fbx").front(), x, y);
				float3 p = build->base->GetTransform()->GetGlobalPos();
				p.y -= 80.f;
				build->base->GetTransform()->SetGlobalPos(p);
			}
			else if (std::rand() % 100 < 20)
			{
				build = new Turret(App->GO->LoadGO("Assets/Turrets/turret/turret.fbx").front(), x, y);
			}
		}
	}
	else
	{
		if (y > 3 && abs(x) <= 1)
		{
			if (std::rand() % 100 < 5)
			{
				build = new Building(App->GO->LoadGO("Assets/Tower/Tower1.fbx").front(), x, y);
				float3 p = build->base->GetTransform()->GetGlobalPos();
				p.y -= 80.f;
				build->base->GetTransform()->SetGlobalPos(p);
			}
			else if (std::rand() % 100 < 20 + 80 * (Time.GameRuntime / 120))
			{
				build = new Turret(App->GO->LoadGO("Assets/Turrets/turret/turret.fbx").front(), x, y);
			}
		}
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

void ModuleTerrainGame::InitBullets()
{
	GameObject* bullet = App->GO->LoadGO("Assets/Turrets/Bullet/Bullet.fbx").front();
	bullet->GetTransform()->SetGlobalPos(15, -15, 0);

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

	GameObject* bullet2 = App->GO->LoadGO("Assets/Turrets/Bullet/Bullet2.fbx").front();
	bullet2->GetTransform()->SetGlobalPos(-15, -15, 0);

	mat = bullet2->GetComponent<Material>().front();
	mat->SetAlphaType(AlphaTestTypes::ALPHA_BLEND);
	mat->SetAlphaTest(0.63f);
	mat->SetColor(0.074, 0.074, 0.9137, 0.9137);

	mat = bullet2->childs[0]->GetComponent<Material>().front();
	mat->SetColor(0, 0, 1);
	mat->ReadRes<R_Material>()->AssignShader("bullet2");

	mat = bullet2->childs[1]->GetComponent<Material>().front();
	mat->SetColor(0.8, 0, 1);
	mat->SetAlphaType(AlphaTestTypes::ALPHA_DISCARD);
	mat->SetAlphaTest(0.4f);
	mat->ReadRes<R_Material>()->AssignShader("bullet");

	bullets.resize(200);
	std::for_each(bullets.begin(), bullets.end(), [](Bullet& b) {b.Init(false); });
	playerBullets.resize(40);
	std::for_each(playerBullets.begin(), playerBullets.end(), [](Bullet& b) {b.Init(true); });

	bulletN = 0;
	playerBulletN = 0;
}

Bullet& ModuleTerrainGame::SpawnBullet(float3 pos, float3 dir, bool playerBullet)
{
	if (playerBullet == false)
	{
		if (bulletN >= bullets.size())
		{
			bulletN = 0;
		}
		bullets[bulletN++].Spawn(pos, dir, playerBullet);
		return bullets[bulletN - 1];
	}
	else
	{
		if (playerBulletN >= playerBullets.size())
		{
			playerBulletN = 0;
		}
		playerBullets[playerBulletN++].Spawn(pos, dir, playerBullet);
		return playerBullets[playerBulletN - 1];
	}
}
