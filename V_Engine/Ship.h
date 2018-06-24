#pragma once
#include "Globals.h"
#include "Math.h"
#include "Timer.h"

class GameObject;

class Ship
{
public:
	Ship();
	~Ship();

	void Init(GameObject* go, GameObject* cam);
	void Hit(int amount);

	void Update(float dt);
	void HandleInput();

	int health = 100;
	float speedExplore = 700;
	float speedBulletHell = 500;
	float acceleration = 4000;

	float3 targetRotation;
	float3 rotation;

	GameObject* controller = nullptr;
	GameObject* ship = nullptr;
	GameObject* cannon = nullptr;
	std::vector<GameObject*> thrusters;
	GameObject* camera = nullptr;
	float3 desiredMovement = float3::zero;
	float3 currentMovement = float3::zero;

	int immunityTime = 750;

	Timer hitTimer;
};