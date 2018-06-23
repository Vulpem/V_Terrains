#pragma once
#include "Globals.h"
#include "Math.h"
#include "Timer.h"

class GameObject;

class Building
{
public:
	Building(GameObject* go, int x, int y);
	virtual ~Building();

	void Update(float dt);
	void Destroy();
	int health = 100;

	GameObject* base = nullptr;
	bool Destroyed() { return destroyed; }
private:
	virtual void VirtualUpdate(float dt) {};
	bool destroyed = false;
};

class Turret : public Building
{
public:
	Turret(GameObject* go, int x, int y);
	~Turret();
	void VirtualUpdate(float dt);

	void Shoot();

	GameObject* target = nullptr;
	GameObject* barrel = nullptr;
	GameObject* spawner = nullptr;
	float rotateSpeed = 90.f;
	float reloadTime = 0.7f;
	float shootingDistance = 5000.f;

	Timer timer;
};