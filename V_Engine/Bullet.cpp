#include "Bullet.h"

#include "AllComponents.h"
#include "Application.h"
#include "ModuleGOmanager.h"
#include "R_Material.h"
#include "../V_Terrain/Code/Include.h"
#include "ModuleTerrainGame.h"
#include "Ship.h"
#include "Camera.h"
#include "ModuleCamera3D.h"

float Bullet::publicSpeed = float();

Bullet::Bullet()
{
	bullet = App->GO->LoadGO("Assets/Turrets/Bullet/Bullet.fbx").front();
	bullet->HideFromOutliner();
	bullet->GetTransform()->SetGlobalPos(0, -100000, 0);

	Material* mat = bullet->GetComponent<Material>().front();
	mat->SetAlphaType(AlphaTestTypes::ALPHA_BLEND);
	mat->SetAlphaTest(0.63f);
	mat->SetColor(0.9137, 0.074, 0.074, 0.9137);

	mat = bullet->childs[0]->GetComponent<Material>().front();
	mat->SetColor(1, 0, 0);

	mat = bullet->childs[1]->GetComponent<Material>().front();
	mat->SetColor(0.9, 0, 1);
	mat->SetAlphaType(AlphaTestTypes::ALPHA_DISCARD);
	mat->SetAlphaTest(0.4f);
	publicSpeed = speed;
}

Bullet::~Bullet()
{
	bullet->Delete();
}

void Bullet::Spawn(float3 pos, float3 dir)
{
	bullet->GetTransform()->SetGlobalPos(pos);
	direction = dir;
	loaded = true;
}

void Bullet::Despawn()
{
	bullet->GetTransform()->SetGlobalPos(0, -100, 0);
	loaded = false;
}

void Bullet::Update(float dt)
{
	if (loaded)
	{
		const float3 pos = bullet->GetTransform()->GetGlobalPos();
		bullet->GetTransform()->SetGlobalPos(pos + direction * speed * dt);

		if (timer.Read() > 100)
		{
			timer.Start();
			Line ray;
			ray.pos = bullet->GetTransform()->GetGlobalPos();
			ray.dir = bullet->GetTransform()->GetGlobalPos() - App->camera->GetDefaultCam()->GetPosition();
			if (ray.Intersects(App->game->player.ship->obb))
			{
				App->game->player.Hit(25);
				Despawn();
			}

			float height = 0;
			RPGT::GetPoint(pos.x, pos.z, height);
			if (pos.y > App->game->cameraHeight || height > pos.y)
			{
				Despawn();
			}
		}
	}
}
