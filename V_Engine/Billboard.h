#ifndef __BILLBOARD__
#define __BILLBOARD__

#include "Component.h"

class Billboard :public Component
{
public:
	Billboard(Gameobject* linkedTo);

	void UpdateDirection(const float3& lookAt, const float3& up = float3::zero);
	void EditorContent() override;

	void SaveSpecifics(pugi::xml_node& myNode) override;
	void LoadSpecifics(pugi::xml_node& myNode) override;

	ComponentType GetType() const override { return ComponentType::billboard; }

	float3 m_localForward = float3(0, 0, 1);
	float3 m_localUp = float3(0, 1, 0);
};

#endif