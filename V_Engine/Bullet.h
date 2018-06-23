#pragma once
#include "Globals.h"
#include "Math.h"

class GameObject;

class Bullet
{
	Bullet(float3 pos, float3 dir);
	~Bullet();

	void Update(float dt);

	float speed = 100.f;
	float3 direction;
	GameObject* bullet;
};