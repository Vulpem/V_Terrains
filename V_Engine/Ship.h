#pragma once
#include "Globals.h"
#include "Math.h"

class GameObject;

class Ship
{
public:
	Ship();
	~Ship();

	void Init(GameObject* go, GameObject* cam);

	void Update(float dt);
	void HandleInput();

	int health = 100;
	float speed = 700;

	float3 targetRotation;
	float3 rotation;

	GameObject* controller = nullptr;
	GameObject* ship = nullptr;
	GameObject* cannon = nullptr;
	std::vector<GameObject*> thrusters;
	GameObject* camera = nullptr;
};