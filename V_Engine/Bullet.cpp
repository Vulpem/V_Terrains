#include "Bullet.h"

#include "AllComponents.h"
#include "Application.h"
#include "ModuleGOmanager.h"
#include "R_Material.h"

Bullet::Bullet(float3 pos, float3 dir)
{
	bullet = App->GO->LoadGO("Assets/Turrets/Bullet/Bullet.fbx").front();

	bullet->GetTransform()->SetGlobalPos(pos);

	Material* mat = bullet->GetComponent<Material>().front();
	mat->SetAlphaType(AlphaTestTypes::ALPHA_BLEND);
	mat->SetAlphaTest(0.63f);
	mat->SetColor(0.9137, 0.074, 0.074, 0.9137);

	mat = bullet->childs[0]->GetComponent<Material>().front();
	mat->SetColor(1, 0, 0);

	mat = bullet->childs[1]->GetComponent<Material>().front();
	mat->SetColor(0.9, 0, 1);
	mat->SetAlphaType(AlphaTestTypes::ALPHA_DISCARD);
	mat->SetAlphaTest(0.4f);
}

Bullet::~Bullet()
{
	bullet->Delete();
}

void Bullet::Update(float dt)
{
	bullet->GetTransform()->SetGlobalPos(bullet->GetTransform()->GetGlobalPos() + direction * speed * dt);
}
