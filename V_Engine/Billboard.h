#ifndef __BILLBOARD__
#define __BILLBOARD__

#include "Component.h"

class Billboard :public Component
{
public:
	Billboard(GameObject* linkedTo);

	void UpdateNow(const float3& point, const float3& up = float3::zero);
	void EditorContent();

	void SaveSpecifics(pugi::xml_node& myNode);
	void LoadSpecifics(pugi::xml_node& myNode);

	ComponentType GetType() const override { return ComponentType::billboard; }

	float3 m_localForward = float3(0, 0, 1);
	float3 m_localUp = float3(0, 1, 0);
};

#endif