#pragma once
#include "Globals.h"
#include "Math.h"

class GameObject;

class Building
{
public:
	Building(GameObject* go, int x, int y);
	~Building();

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

	GameObject* target = nullptr;
	GameObject* barrel = nullptr;
	float speed = 90.f;
};