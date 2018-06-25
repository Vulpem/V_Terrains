#include "Ship.h"

#include "Application.h"
#include "GameObject.h"
#include "AllComponents.h"
#include "Globals.h"
#include "ModuleInput.h"

#include "ModuleCamera3D.h"
#include "ModuleEditor.h"
#include "ModuleGOmanager.h"
#include "ModuleTerrainGame.h"

#include "../V_Terrain/Code/Include.h"

Ship::Ship()
{

}

Ship::~Ship()
{

}

void Ship::Init(GameObject * go, GameObject* cam)
{
	camera = cam;

	controller = go;
	ship = go->childs.front();
	cannon = ship->childs.front();
	for (int n = 1; n < ship->childs.size(); n++)
	{
		thrusters.push_back(ship->childs[n]);
	}

	rotation = float3::zero;
	ship->GetComponent<Material>().front()->SetColor(1, 1, 1, 0);

}

void Ship::Hit(int amount)
{
	if (hitTimer.Read() > immunityTime)
	{
		hitTimer.Start();
		health <= amount;
		if (health <= 0)
		{
			//TODO
			//Loose;
		}
	}
}

void Ship::Update(float dt)
{
	HandleInput();

	Transform* controllerT = controller->GetTransform();
	Transform* shipT = ship->GetTransform();

	if (hitTimer.Read() < immunityTime)
	{
		if (hitTimer.Read() % 300 > 150)
		{
			ship->GetComponent<Material>().front()->SetAlphaType(AlphaTestTypes::ALPHA_OPAQUE);
		}
		else
		{
			ship->GetComponent<Material>().front()->SetAlphaType(AlphaTestTypes::ALPHA_BLEND);
		}
	}
	else
	{
		ship->GetComponent<Material>().front()->SetAlphaType(AlphaTestTypes::ALPHA_OPAQUE);
	}

	if (App->game->game == GameType::BulletHell)
	{
		
		ship->GetTransform()->LookAt(float3(App->game->mousePos.x, controller->GetTransform()->GetGlobalPos().y,App->game->mousePos.z));
		if (App->input->GetMouseButton(1))
		{
			if (reloadTimer.Read() > reloadTime * 1000.f)
			{
				App->game->SpawnBullet(cannon->GetTransform()->GetGlobalPos(), cannon->GetTransform()->Forward(), true).speed = speedBulletHell * 2.f;

				reloadTimer.Start();
			}
		}

		if (desiredMovement.x != currentMovement.x || desiredMovement.y != currentMovement.y || desiredMovement.z != currentMovement.z)
		{
			if (desiredMovement.LengthSq() > 0.0f)
			{
				desiredMovement.Normalize();
				desiredMovement *= speedBulletHell;
			}
			float3 speedChange = (desiredMovement - currentMovement).Normalized() * acceleration * Time.dt;
			currentMovement += speedChange;

			if (currentMovement.x * (currentMovement.x - speedChange.x) < 0) { currentMovement.x = 0; }
			if (currentMovement.z * (currentMovement.z - speedChange.z) < 0) { currentMovement.z = 0; }

			controller->GetTransform()->Translate(currentMovement * Time.dt);
		}
	}
	else
	{
		const float  rotSpeed = 50.f;

		rotation += targetRotation * rotSpeed * dt;


		controllerT->SetLocalRot(float3(targetRotation.x, rotation.y, 0.f));
		shipT->RotateLocal(float3(0.f, 0.f, rotation.z));

		controllerT->SetGlobalPos(controllerT->GetGlobalPos() + controllerT->Forward() * speedExplore * dt);

		float3 cameraTarget = shipT->GetGlobalPos() +
			-shipT->Forward() * 100 +
			shipT->Up() * 50;

		camera->GetTransform()->SetGlobalPos(Lerp(camera->GetTransform()->GetGlobalPos(), cameraTarget, 1));
		camera->GetTransform()->LookAt(shipT->GetGlobalPos() + shipT->Forward() * 200.f);
	}
}

void Ship::HandleInput()
{
	if (App->game->game == GameType::BulletHell)
	{
		desiredMovement = float3::zero;
		if (App->input->GetKey(SDL_SCANCODE_D))
		{
			desiredMovement.x -= 1;
		}
		if (App->input->GetKey(SDL_SCANCODE_A))
		{
			desiredMovement.x += 1;
		}
		if (App->input->GetKey(SDL_SCANCODE_W))
		{
			desiredMovement.z += 1;
		}
		if (App->input->GetKey(SDL_SCANCODE_S))
		{
			desiredMovement.z -= 1;
		}
	}
	else
	{
		targetRotation = float3::zero;

		if (App->input->GetKey(SDL_SCANCODE_D))
		{
			targetRotation.y -= 1;
			targetRotation.z += 0.35f;
		}
		if (App->input->GetKey(SDL_SCANCODE_A))
		{
			targetRotation.y += 1;
			targetRotation.z -= 0.35f;
		}
		if (App->input->GetKey(SDL_SCANCODE_W))
		{
			targetRotation.x -= 1;
		}
		if (App->input->GetKey(SDL_SCANCODE_S))
		{
			targetRotation.x += 1;
		}

		if (App->input->GetKey(SDL_SCANCODE_E))
		{
			targetRotation.z += 0.35f;
		}
		if (App->input->GetKey(SDL_SCANCODE_Q))
		{
			targetRotation.z -= 0.35f;
		}
	}
}
