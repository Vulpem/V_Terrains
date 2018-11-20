#ifndef __MATERIAL__
#define __MATERIAL__

#include "ResourcedComponent.h"
#include "Mesh_RenderInfo.h"

class Material :public ResourcedComponent
{
public:
	Material(std::string res, Gameobject* linkedTo);

	void PreUpdate() override;

	void EditorContent() override;

	void SaveSpecifics(pugi::xml_node& myNode) const override;
	void LoadSpecifics(pugi::xml_node& myNode) override;

	uint NofTextures() const;
	int GetTexture(uint n) const;
	bool AddTexture(std::string fileName);

	void SetColor(float r, float g, float b, float a = 1.0f);
	math::float4 GetColor() const;

	ComponentType GetType() const override { return ComponentType::material; }

	AlphaTestTypes GetAlphaType() const;
	void SetAlphaType(AlphaTestTypes type);

	float GetAlphaTest() const;
	void SetAlphaTest(float alphaTest);

	int GetBlendType() const;
	void SetBlendType(int blendType);

	Shader GetShader() const;

private:
	std::vector<uint> m_texturesToRemove;
	void RemoveTexturesNow();
};

#endif