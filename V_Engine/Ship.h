#pragma once
#include "Globals.h"
#include "Math.h"

class GameObject;

class Ship
{
public:
	Ship();
	~Ship();

	void Init(GameObject* go);

	void Update(float dt);
	void HandleInput();

	int health = 100;

	GameObject* ship = nullptr;
};