#pragma once
#include "Globals.h"
#include "Math.h"

class GameObject;

class Turret
{
public:
	void Update(GameObject* player);


	float3 originalRotation;
	float3 rotation;

	GameObject* base = nullptr;
	GameObject* rotator_Y = nullptr;
	GameObject* rotator_X = nullptr;
	GameObject* barrel = nullptr;
	GameObject* metal = nullptr;
};