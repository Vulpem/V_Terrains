#include "Ship.h"

#include "Application.h"
#include "GameObject.h"
#include "AllComponents.h"
#include "Globals.h"

#include "ModuleCamera3D.h"
#include "ModuleEditor.h"

#include "../V_Terrain/Code/Include.h"

Ship::Ship()
{

}

Ship::~Ship()
{

}

void Ship::Init(GameObject * go)
{
	ship = go;
}

void Ship::Update(float dt)
{
}

void Ship::HandleInput()
{
}
