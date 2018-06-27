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

	void Init(bool player = false);
	void Spawn(float3 pos, float3 dir, bool playerBullet = false);
	void Despawn();
	void Update(float dt);

	bool loaded = false;
	float speed = 700.f;
	float3 direction;
	GameObject* bullet;

	Timer timer;
	bool playerBullet = false;
};