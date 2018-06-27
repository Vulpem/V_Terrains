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
	void Hit(int amount);
	int health = 1000;

	GameObject* target = nullptr;
	GameObject* base = nullptr;
	bool vulnerable = false;
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

	GameObject* barrel = nullptr;
	GameObject* spawner = nullptr;
	float rotateSpeed = 90.f;
	float reloadTime = 1.f;

	Timer timer;
};