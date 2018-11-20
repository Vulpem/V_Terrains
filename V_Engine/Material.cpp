#include "Material.h"

#include "GameObject.h"
#include "Application.h"

#include "R_Material.h"

#include "imGUI\imgui.h"

Material::Material(std::string res, Gameobject* linkedTo) : ResourcedComponent(linkedTo, ComponentType::material)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Material##%i", m_uid);
	m_name = tmp;
	LoadResource(res);
}

void Material::PreUpdate()
{
	RemoveTexturesNow();
}

void Material::EditorContent()
{
	R_Material* res = ReadRes<R_Material>();

#pragma region AddTexturePopup
	if (ImGui::BeginPopup("Add New Texture"))
	{
		std::vector<std::pair<std::string, std::vector<std::string>>> meshRes = App->m_resourceManager->GetAvaliableResources(ComponentType::texture);
		std::vector<std::pair<std::string, std::vector<std::string>>>::iterator fileIt = meshRes.begin();
		for (; fileIt != meshRes.end(); fileIt++)
		{
			if (ImGui::MenuItem(fileIt->first.data()))
			{
				res->m_textures.push_back(App->m_resourceManager->LinkResource(fileIt->second.front(), ComponentType::texture));
				break;
			}
		}
		ImGui::EndPopup();
	}
#pragma endregion

#pragma region ChangeShaderPopup
	if (ImGui::BeginPopup("Select Shader"))
	{
		if (ImGui::MenuItem("Default shader"))
		{
			App->m_resourceManager->UnlinkResource(res->m_shader);
			res->m_shader = 0;
		}
		std::vector<std::pair<std::string, std::vector<std::string>>> shadersRes = App->m_resourceManager->GetAvaliableResources(ComponentType::shader);
		std::vector<std::pair<std::string, std::vector<std::string>>>::iterator fileIt = shadersRes.begin();
		for (; fileIt != shadersRes.end(); fileIt++)
		{
			if (ImGui::MenuItem(fileIt->second.front().data()))
			{
				res->AssignShader(fileIt->second.front());
				break;
			}
		}
		ImGui::EndPopup();
	}
#pragma endregion
	uint64_t currentShader = res->m_shader;
	if (currentShader != 0)
	{
		ImGui::Text("Current shader: %s", App->m_resourceManager->Peek(currentShader)->m_name.data());
	}
	else
	{
		ImGui::Text("Current shader: Default shader");
	}

	if (ImGui::Button("Add new texture##AddTextureButton"))
	{
		ImGui::OpenPopup("Add New Texture");
	}
	ImGui::SameLine();
	if (ImGui::Button("Change Shader##ChangeShaderButton"))
	{
		ImGui::OpenPopup("Select Shader");
	}
	ImGui::Text("Blend type:");
	int alphaType = static_cast<int>(GetAlphaType());
	int prevAlphaType = static_cast<int>(alphaType);
	ImGui::RadioButton("Opaque", &alphaType, static_cast<int>(AlphaTestTypes::ALPHA_OPAQUE)); ImGui::SameLine();
	ImGui::RadioButton("AlphaTest", &alphaType, static_cast<int>(AlphaTestTypes::ALPHA_DISCARD)); ImGui::SameLine();
	ImGui::RadioButton("Blend", &alphaType, static_cast<int>(AlphaTestTypes::ALPHA_BLEND));
	if (alphaType != prevAlphaType)
	{
		SetAlphaType((AlphaTestTypes)alphaType);
	}

	if (alphaType != static_cast<int>(AlphaTestTypes::ALPHA_OPAQUE))
	{
		ImGui::Text("AlphaTest:");
		float tmp = GetAlphaTest();
		if (ImGui::DragFloat("##MaterialAlphaTest", &tmp, 0.01f, 0.0f, 1.0f))
		{
			SetAlphaTest(tmp);
		}
		if (alphaType == static_cast<int>(AlphaTestTypes::ALPHA_BLEND))
		{
			if (ImGui::CollapsingHeader("Alpha Blend Types"))
			{
				int blendType = GetBlendType();
				int lastBlendType = blendType;
				ImGui::RadioButton("Zero##BlendTypes", &blendType, GL_ZERO);
				ImGui::RadioButton("One##BlendTypes", &blendType, GL_ONE);
				ImGui::RadioButton("Src_Color##BlendTypes", &blendType, GL_SRC_COLOR);
				ImGui::RadioButton("One_Minus_Src_Color##BlendTypes", &blendType, GL_ONE_MINUS_SRC_COLOR);
				ImGui::RadioButton("Dst_Color##BlendTypes", &blendType, GL_DST_COLOR);
				ImGui::RadioButton("One_Minus_Dst_Color##BlendTypes", &blendType, GL_ONE_MINUS_DST_COLOR);
				ImGui::RadioButton("Src_Alpha##BlendTypes", &blendType, GL_SRC_ALPHA);
				ImGui::RadioButton("One_Minus_Src_Alpha##BlendTypes", &blendType, GL_ONE_MINUS_SRC_ALPHA);
				ImGui::RadioButton("Dst_Alpha##BlendTypes", &blendType, GL_DST_ALPHA);
				ImGui::RadioButton("One_Minus_Dst_Alpha##BlendTypes", &blendType, GL_ONE_MINUS_DST_ALPHA);
				ImGui::RadioButton("Constant_Color##BlendTypes", &blendType, GL_CONSTANT_COLOR);
				ImGui::RadioButton("One_Minus_Constant_Color##BlendTypes", &blendType, GL_ONE_MINUS_CONSTANT_COLOR);
				ImGui::RadioButton("Constant_Alpha##BlendTypes", &blendType, GL_CONSTANT_ALPHA);
				ImGui::RadioButton("One_Minus_Constant_Alpha##BlendTypes", &blendType, GL_ONE_MINUS_CONSTANT_ALPHA);
				if (blendType != lastBlendType)
				{
					SetBlendType(blendType);
				}
			}
		}
	}


	ImGui::NewLine();
	ImGui::Separator();
	ImGui::ColorEdit4("Color", res->m_color);
	if (res->m_textures.empty() == false)
	{
		for (uint n = 0; n < res->m_textures.size(); n++)
		{
			ImGui::Separator();
			char tmp[524];
			sprintf(tmp, "X##%u", n);
			if (ImGui::Button(tmp))
			{
				m_texturesToRemove.push_back(n);
			}
			Resource* resText = App->m_resourceManager->Peek(res->m_textures.at(n));
			if (resText != nullptr)
			{
				ImGui::SameLine();
				sprintf(tmp, "Id: %i    %s", n, resText->m_name.data());
				if (ImGui::TreeNode(tmp))
				{
					ImTextureID image = (void*)App->m_resourceManager->Peek(res->m_textures.at(n))->Read<R_Texture>()->m_bufferID;
					ImGui::Image(image, ImVec2(270, 270));

					ImGui::TreePop();
				}
			}
		}
	}
}

void Material::SaveSpecifics(pugi::xml_node& myNode) const
{
	if (m_resource)
	{
		Resource* res = App->m_resourceManager->Peek(m_resource);
		myNode.append_attribute("res") = res->m_name.data();
		pugi::xml_node color_n = myNode.append_child("Color");
		color_n.append_attribute("R") = ReadRes<R_Material>()->m_color[0];
		color_n.append_attribute("G") = ReadRes<R_Material>()->m_color[1];
		color_n.append_attribute("B") = ReadRes<R_Material>()->m_color[2];
		color_n.append_attribute("A") = ReadRes<R_Material>()->m_color[3];
	}
}

void Material::LoadSpecifics(pugi::xml_node & myNode)
{
	std::string resName = myNode.attribute("res").as_string();
	m_resource = App->m_resourceManager->LinkResource(resName.data(),GetType());

	pugi::xml_node col = myNode.child("Color");

	R_Material* res = ReadRes<R_Material>();
	if (res)
	{
		res->m_color[0] = col.attribute("R").as_float();
		res->m_color[1] = col.attribute("G").as_float();
		res->m_color[2] = col.attribute("B").as_float();
		res->m_color[3] = col.attribute("A").as_float();
	}
}

uint Material::NofTextures() const
{
	return ReadRes<R_Material>()->m_textures.size();
}

int Material::GetTexture(uint n) const
{
	if (IsEnabled() && m_texturesToRemove.empty() == true)
	{
		if (n >= 0 && n < ReadRes<R_Material>()->m_textures.size())
		{
			R_Material* mat = ReadRes<R_Material>();
			if (mat)
			{
				R_Texture* tex = App->m_resourceManager->Peek(mat->m_textures.at(n))->Read<R_Texture>();
				if (tex)
				{
					return tex->m_bufferID;
				}
			}
		}
	}
	return 0;
}

bool Material::AddTexture(std::string fileName)
{
	R_Material* res = ReadRes<R_Material>();
	res->m_textures.push_back(App->m_resourceManager->LinkResource(fileName, ComponentType::texture));
	return true;
}

void Material::SetColor(float r, float g, float b, float a)
{
	ReadRes<R_Material>()->m_color[0] = r;
	ReadRes<R_Material>()->m_color[1] = g;
	ReadRes<R_Material>()->m_color[2] = b;
	ReadRes<R_Material>()->m_color[3] = a;
}

math::float4 Material::GetColor() const
{
	return math::float4(ReadRes<R_Material>()->m_color);
}

AlphaTestTypes Material::GetAlphaType() const
{
	return ReadRes<R_Material>()->m_alphaType;
}

void Material::SetAlphaType(AlphaTestTypes type)
{
	ReadRes<R_Material>()->m_alphaType = type;
}

float Material::GetAlphaTest() const
{
	return ReadRes<R_Material>()->m_alphaTest;
}

void Material::SetAlphaTest(float alphaTest)
{
	ReadRes<R_Material>()->m_alphaTest = alphaTest;
}

int Material::GetBlendType() const
{
	return ReadRes<R_Material>()->m_blendType;
}

void Material::SetBlendType(int blendType)
{
	ReadRes<R_Material>()->m_blendType = blendType;
}

Shader Material::GetShader() const
{
	return ReadRes<R_Material>()->GetShaderProgram();
}

void Material::RemoveTexturesNow()
{
	if (m_texturesToRemove.empty() == false)
	{
		R_Material* matRes = ReadRes<R_Material>();
		if (matRes != nullptr)
		{
			for (int n = 0; n < m_texturesToRemove.size(); n++)
			{
				std::vector<uint64>::iterator it = matRes->m_textures.begin();
				for (int m = 0; it != matRes->m_textures.end(); it++)
				{
					if (m == m_texturesToRemove[n])
					{
						App->m_resourceManager->UnlinkResource(matRes->m_textures[m]);
						matRes->m_textures.erase(it);
						break;
					}
					m++;
				}
			}
		}
		m_texturesToRemove.clear();
	}
}
