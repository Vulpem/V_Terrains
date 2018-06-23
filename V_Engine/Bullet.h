#pragma once
#include "Globals.h"
#include "Math.h"

class GameObject;

class Bullet
{
public:
	Bullet();
	~Bullet();

	void Spawn(float3 pos, float3 dir);
	void Despawn();
	void Update(float dt);

	bool loaded = false;
	float speed = 100.f;
	float3 direction;
	GameObject* bullet;
};