#include "Ship.h"

#include "Application.h"
#include "GameObject.h"
#include "AllComponents.h"
#include "Globals.h"
#include "ModuleInput.h"

#include "ModuleCamera3D.h"
#include "ModuleEditor.h"
#include "ModuleGOmanager.h"

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

}

void Ship::Update(float dt)
{
	HandleInput();

	Transform* controllerT = controller->GetTransform();
	Transform* shipT = ship->GetTransform();

	
	const float  rotSpeed = 50.f;

	rotation += targetRotation * rotSpeed * dt;


	controllerT->SetLocalRot(float3(targetRotation.x, rotation.y, 0.f));
	shipT->RotateLocal(float3(0.f,0.f,rotation.z));

	controllerT->SetGlobalPos(controllerT->GetGlobalPos() + controllerT->Forward() * 2000 * dt);

	float3 cameraTarget = shipT->GetGlobalPos() +
		-shipT->Forward() * 100 +
		shipT->Up() * 50;

	camera->GetTransform()->SetGlobalPos(Lerp(camera->GetTransform()->GetGlobalPos(), cameraTarget, 1));
	camera->GetTransform()->LookAt(shipT->GetGlobalPos() + shipT->Forward() * 200.f);
}

void Ship::HandleInput()
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
