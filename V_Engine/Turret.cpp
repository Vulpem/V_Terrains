#include "Turret.h"

#include "Application.h"
#include "GameObject.h"
#include "AllComponents.h"
#include "Globals.h"

#include "ModuleCamera3D.h"
#include "ModuleEditor.h"

#include "ModuleTerrainGame.h"

#include "../V_Terrain/Code/Include.h"

#include "Bullet.h"

Building::Building(GameObject * go, int x, int y) :
	base (go)
{
	Transform* trans = base->GetTransform();
	float sizeDif = 0.5 + (float)(std::rand() % 100) / 100.f;

	float3 pos = float3(
		x * RPGT::config.chunkSize + (std::rand() % (int)RPGT::config.chunkSize) - RPGT::config.chunkSize / 2.f,
		RPGT::config.maxHeight,
		y * RPGT::config.chunkSize + (std::rand() % (int)RPGT::config.chunkSize) - RPGT::config.chunkSize / 2.f
	);
	RPGT::GetPoint(pos.x, pos.z, pos.y);
	trans->SetGlobalPos(pos);
	trans->SetLocalScale(2.f * sizeDif, 2.f * sizeDif, 2.f * sizeDif);

	target = App->game->player.ship;
}

Building::~Building()
{
	base->Delete();
	base = nullptr;
}

void Building::Update(float dt)
{
	if (health <= 0)
	{
		Destroy();
	}
	if (base->aabb.Contains(target->GetTransform()->GetGlobalPos()))
	{
		App->game->player.Hit(25);
	}
	VirtualUpdate(dt);
}

void Building::Destroy()
{
	destroyed = true;
}


Turret::Turret(GameObject * go, int x, int y)
	: Building(go, x, y)
{
	barrel = base->childs.front();

	float3 normal;
	float3 pos = base->GetTransform()->GetGlobalPos();
	RPGT::GetPoint(pos.x, pos.z, pos.y, normal.ptr());
	base->GetTransform()->SetLocalScale(2.f, 2.f, 2.f);
	base->GetTransform()->SetGlobalRot(Quat::RotateFromTo(float3(0, 1, 0), normal).ToEulerXYZ()*RADTODEG);


	float3 barrelPos = barrel->GetTransform()->GetGlobalPos();
	float3 barrelScale = barrel->GetTransform()->GetGlobalScale();
	barrel->parent = base->parent;
	barrel->parent->childs.push_back(barrel);
	base->childs.clear();

	barrel->GetTransform()->SetGlobalPos(barrelPos);
	barrel->GetTransform()->SetLocalScale(barrelScale.x, barrelScale.y, barrelScale.z);

	spawner = barrel->childs.front();

	timer.Start();
}

Turret::~Turret()
{
	barrel->Delete();
	barrel = nullptr;
}

void Turret::VirtualUpdate(float dt)
{
	if (!App->game->debugTurrets)
	{
		target = App->game->player.ship;
	}
	else
	{
		target = App->camera->GetDefaultCam()->object;
	}
	if (barrel != nullptr && target != nullptr)
	{
		float3 t = target->GetTransform()->GetGlobalPos()
			+ float3(0,0,1) * App->game->verticalSpeed;
		barrel->GetTransform()->LookAt(t, base->GetTransform()->Up());
		Shoot();
	}
}

void Turret::Shoot()
{
	if (timer.Read() / 1000 > reloadTime)
	{
		timer.Start();
		if (target->GetTransform()->GetGlobalPos().DistanceSq(base->GetTransform()->GetGlobalPos()) < shootingDistance * shootingDistance)
		{
			App->game->SpawnBullet(spawner->GetTransform()->GetGlobalPos(), barrel->GetTransform()->Forward());
		}
	}
}

