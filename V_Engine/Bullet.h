#pragma once
#include "Globals.h"
#include "Math.h"
#include "Timer.h"

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
	float speed = 500.f;
	static float publicSpeed;
	float3 direction;
	GameObject* bullet;

	Timer timer;
};