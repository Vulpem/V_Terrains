#include "Turret.h"

#include "GameObject.h"
#include "AllComponents.h"

void Turret::Update(float dt)
{
	if (barrel != nullptr)
	{
		barrel->GetTransform()->LookAt(target);
	}
}
