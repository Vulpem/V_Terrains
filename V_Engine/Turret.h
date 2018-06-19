#pragma once
#include "Globals.h"
#include "Math.h"

class GameObject;

class Turret
{
public:
	void Update(float dt);

	float3 target;

	float speed = 90.f;

	float3 originalRotation;
	float3 rotation;

	GameObject* base = nullptr;
	GameObject* barrel = nullptr;
};