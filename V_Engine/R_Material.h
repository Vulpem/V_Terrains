#ifndef  __R_MATERIAL__
#define __R_MATERIAL__

#include "R_Resource.h"
#include "R_Texture.h"
#include "R_Shader.h"
#include "ModuleResourceManager.h"
#include "Mesh_RenderInfo.h"
#include "Shader.h"

class R_Material : public Resource
{
public:
	R_Material() :Resource() { m_alphaTest = 0.2f; }
	R_Material(uint64_t UID) : Resource(UID) { m_alphaTest = 0.2f; }

	~R_Material()
	{
		if (m_shader != 0)
		{
			App->m_resourceManager->UnlinkResource(m_shader);
		}

		if (m_textures.empty() == false)
		{
			for (std::vector<uint64_t>::iterator it = m_textures.begin(); it != m_textures.end(); it++)
			{
				App->m_resourceManager->UnlinkResource(*it);
			}
		}
	}

	bool AssignShader(std::string shaderName)
	{
		uint64_t res = App->m_resourceManager->LinkResource(shaderName, Component::Type::C_Shader);
		if (res == 0) { return false; }
		if (m_shader != 0)
		{
			App->m_resourceManager->UnlinkResource(m_shader);
		}
		m_shader = res;
	}

	Shader GetShaderProgram()
	{
		if (m_shader != 0)
		{			
			return ((R_Shader*)App->m_resourceManager->Peek(m_shader))->m_shaderProgram;
		}
		return App->m_resourceManager->GetDefaultShader();
	}
	

	Component::Type GetType() { return Component::Type::C_material; }

	float m_color[5] = { 1.0f, 1.0f, 1.0f,1.0f };
	std::vector<uint64_t> m_textures;
	uint64_t m_shader = 0;

	AlphaTestTypes m_alphaType = AlphaTestTypes::ALPHA_OPAQUE;
	int m_blendType = GL_ONE_MINUS_SRC_ALPHA;
	float m_alphaTest;

};

#endif