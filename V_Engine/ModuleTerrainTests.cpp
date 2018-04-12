#include "Application.h"
#include "ModuleTerrainTests.h"

#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleEditor.h"
#include "ViewPort.h"
#include "GameObject.h"
#include "Transform.h"

#include "ModuleInput.h"
#include "imGUI\imgui.h"

#include "AllResources.h"

#include "OpenGL.h"

#include <time.h>

ModuleTerrain::ModuleTerrain(Application* app, bool start_enabled) :
    Module(app, start_enabled)
    , m_resolution(32)
    , m_frequency(0.8f)
    , m_octaves(8)
    , m_lacunarity(2.0f)
    , m_persistance (0.4f)
    , m_maxHeight(1000.f)
	, m_fogDistance(5000.f)
    , m_currentHeightCurve("float func(float x)\n{ return x; }")
    , m_curvePow(2)
    , m_setCurvePow(2)
    , m_openShaderEditor(false)
{
	moduleName = "ModuleTerrainTests";
    VTerrain::SetHeightCurve(
        [](float x)
    {
        return x;
    }
    );
}

// Destructor
ModuleTerrain::~ModuleTerrain()
{}

// Called before render is available
bool ModuleTerrain::Init()
{
    bool ret = true;
	return ret;
}

bool ModuleTerrain::Start()
{
	bool ret = true;
    VTerrain::Init();

    App->camera->GetDefaultCam()->object->GetTransform()->SetGlobalPos(0.f, m_maxHeight, 0.f);
    GenMap();

	SetDefaultTextures();

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_maxTexturesGL);

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleTerrain::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModuleTerrain::Update()
{
    float3 pos = App->camera->GetDefaultCam()->GetPosition();
	VTerrain::Update(pos.x, pos.z);

    ImGui::SetNextWindowPos(ImVec2(0.f, 20.f));

    ImGui::SetNextWindowSize(ImVec2(350, (App->window->GetWindowSize().y - 20) / 4 * 3 - 20));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;


    if (ImGui::Begin("TerrainTests", 0, flags))
    {
        ImGui::Text(
            "Use WASD to move the camera around.\n"
            "Q and E to move up and down.\n"
            "Hold right click to pan.\n"
            "Hold shift to move faster.\n"
            "Press space for top view.");
        ImGui::Separator();
        ImGui::Text("Max Fragment textures: %i", m_maxTexturesGL);
        ImGui::Separator();
        const int HMresolution = static_cast<int>(VTerrain::config.chunkSize);
        int p[2] = {
            floor((pos.x - floor(HMresolution / 2.f) + (HMresolution % 2 != 0)) / HMresolution + 1),
            floor((pos.z - floor(HMresolution / 2.f) + (HMresolution % 2 != 0)) / HMresolution + 1)
        };
        ImGui::Text("Current chunk:\nX: %i,   Z: %i", p[0], p[1]);
        ImGui::NewLine();
        if (ImGui::Button("Generate new random seed"))
        {
            VTerrain::SetSeed(time(NULL));
        }

        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Separator();
		ImGui::Checkbox("Open shader editor", &m_openShaderEditor);
		if (ImGui::CollapsingHeader("Render"))
		{
			ImGui::SliderInt("Amount of LODs", &(int)VTerrain::config.nLODs, 0, 64);
			ImGui::SliderFloat("LOD distance", &VTerrain::config.LODdistance, 0.1f, 10000.f, "%.3f", 2.f);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Reduce the amount of triangles by a\n"
					"power of this value.\n"
					"If set to 0, they're set automatically\n"
					"from distance to camera.");

			ImGui::SliderFloat("FogDistance", &VTerrain::config.fogDistance, 0.0f, 100000.f, "%.3f", 4.f);
			ImGui::SliderFloat("WaterHeight", &VTerrain::config.waterHeight, 0.0f, m_maxHeight);
			ImGui::NewLine();
			ImGui::Text("Global light:");
			bool lightDirChanged = false;
			if (ImGui::SliderFloat("Direction", &m_globalLightDir, -360, 360)) { lightDirChanged = true; }
			if (ImGui::SliderFloat("Height", &m_globalLightHeight, -90, 90)) { lightDirChanged = true; }

			if (lightDirChanged)
			{
				float3 tmp(1.f, 0.f, 0.f);
				Quat rot = Quat::FromEulerYZX(m_globalLightDir * DEGTORAD, -m_globalLightHeight * DEGTORAD, 0.f);
				tmp = rot * tmp;
				VTerrain::config.globalLight[0] = tmp.x;
				VTerrain::config.globalLight[1] = tmp.y;
				VTerrain::config.globalLight[2] = tmp.z;
			}

			ImGui::DragFloat3("Light vector", VTerrain::config.globalLight);
			ImGui::NewLine();
		}
		if (ImGui::CollapsingHeader("Terrain generation"))
		{
			if (ImGui::SliderFloat("MaxHeight", &m_maxHeight, 0.0f, 8000.f, "%.3f", 3.f)) { VTerrain::config.maxHeight = m_maxHeight; }
			ImGui::Separator();
			if (ImGui::DragInt("Heigtmap resolution", &m_resolution, 1.f, 5, 1024)) { VTerrain::config.chunkHeightmapResolution = m_resolution; }
			if (ImGui::DragFloat("Chunk Size", &VTerrain::config.chunkSize, 1.f, 0.1f, 256.f)) { WantRegen(); }


			ImGui::Separator();

			if (ImGui::SliderFloat("##Frequency", &m_frequency, 0.0001f, 2.f)) { WantRegen(); }
			if (ImGui::DragFloat("Frequency", &m_frequency, 0.01f, 0.0001f, 2.f)) { WantRegen(); }
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(
					"Wavelength of the noise generation.\n"
					"Will reset all chunks.");
			ImGui::Separator();

			if (ImGui::SliderInt("##Octaves", &m_octaves, 1, 15)) { WantRegen(); }
			if (ImGui::DragInt("Octaves", &m_octaves, 1, 1, 15)) { WantRegen(); }
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(
					"Amount of noisemaps generated and\n"
					"overlapped to generate the final result.\n"
					"Will reset all chunks.");
			ImGui::Separator();

			if (ImGui::SliderFloat("##Lacunarity", &m_lacunarity, 0.01f, 8.f)) { WantRegen(); }
			if (ImGui::DragFloat("Lacunarity", &m_lacunarity, 0.01f, 0.01f, 8.f)) { WantRegen(); }
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(
					"Octave frequency multiplier.\n"
					"Will reset all chunks.");
			ImGui::Separator();

			if (ImGui::SliderFloat("##Persistance", &m_persistance, 0.001f, 1.f)) { WantRegen(); }
			if (ImGui::DragFloat("Persistance", &m_persistance, 0.01f, 0.01f, 1.f)) { WantRegen(); }
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(
					"Strength multiplier for each octave.\n"
					"Will reset all chunks.");
			ImGui::Separator();
			if (ImGui::BeginMenu("Set Height Curve"))
			{
				ImGui::InputInt("P", &m_curvePow);
				ImGui::Text("Functions:");
				ImGui::Separator();
				if (ImGui::MenuItem(
					"float func (float x)\n"
					"{ return x; }"
				))
				{
					m_currentHeightCurve =
						"float func (float x)\n"
						"{ return x; }";
					VTerrain::SetHeightCurve([](float x) {return x; });
				}
				ImGui::Separator();
				if (ImGui::MenuItem(
					"float func(float x)\n"
					"{ return pow(x, P); }"
				))
				{
					m_setCurvePow = m_curvePow;
					m_currentHeightCurve =
						"float func(float x)\n"
						"{ return pow(x, %i); }";
					int n = m_curvePow;
					VTerrain::SetHeightCurve([n](float x) {return pow(x, n); });
				}
				ImGui::Separator();
				if (ImGui::MenuItem(
					"float func(float x)\n"
					"{\n"
					"   x = x * 2.f - 1.f;\n"
					"   return (pow(x, P) + 1.f) * 0.5f;\n"
					"}"
				))
				{
					m_setCurvePow = m_curvePow;
					m_currentHeightCurve =
						"float func(float x)\n"
						"{\n"
						"   x = x * 2.f - 1.f;\n"
						"   return (pow(x, %i) + 1.f) * 0.5f;\n"
						"}";
					int n = m_curvePow;
					VTerrain::SetHeightCurve([n](float x)
					{
						x = x * 2.f - 1.f;
						return (pow(x, n) + 1.f) * 0.5f;
					});
				}

				ImGui::EndMenu();
			}
			ImGui::Separator();
			ImGui::Text(m_currentHeightCurve.data(), m_setCurvePow);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(
					"Function that will be applied to.\n"
					"all heightmap pixels.\n"
					"Recieves a value from 0 to 1 and\n"
					"returns another one in the same range.\n"
					"Will reset all chunks.");
		}
		if (ImGui::CollapsingHeader("Debug visualization"))
		{
			if (ImGui::Button("Reset Camera Height"))
			{
				float3 pos = App->camera->GetDefaultCam()->GetPosition();
				App->camera->GetDefaultCam()->object->GetTransform()->SetGlobalPos(pos.x, m_maxHeight, pos.y);
			}
			ImGui::Checkbox("Chunk Borders", &VTerrain::config.debug.renderChunkBorders);

			if (ImGui::Checkbox("Multiple viewports", &App->Editor->multipleViews))
			{
				App->Editor->SwitchViewPorts();
			}
			if (App->Editor->multipleViews)
			{
				ImGui::Spacing();
				ImGui::Checkbox("Auto follow top cam", &App->camera->m_followCamera);
			}

		}
		if (ImGui::CollapsingHeader("Textures"))
		{
			for (int n = 0; n < 10; n++)
			{
				VTerrain::ConditionalTexture tex = VTerrain::GetTexture(n);
                bool changed = false;
                char tmp[8];
                sprintf_s(tmp, "##%i", n);

				ImGui::ColorButton(tmp, ImVec4(tex.color.x(), tex.color.y(), tex.color.z(), 1.f));
				ImGui::SameLine();
				ImGui::Image((ImTextureID)tex.buf_diffuse, ImVec2(20, 20));
				ImGui::SameLine();
				ImGui::Image((ImTextureID)tex.buf_heightmap, ImVec2(20, 20));
				ImGui::SameLine();
                if (ImGui::BeginMenu((std::string("Texture ") + tmp).data()))
                {
                    if (ImGui::ColorEdit3((std::string("Color") + tmp).data(), tex.color.d)) { changed = true; }
					ImGui::Separator();
                    if (ImGui::SliderFloat((std::string("MinHeight") + tmp).data(), &tex.minHeight, 0.f, 1.0f)) { changed = true; }
                    if (ImGui::SliderFloat((std::string("MaxHeight") + tmp).data(), &tex.maxHeight, 0.f, 1.0f)) { changed = true; }
					if (ImGui::SliderFloat((std::string("HeightFadeDistance") + tmp).data(), &tex.heightFade, 0.f, 1.0f)) { changed = true; }
					ImGui::Separator();
                    if (ImGui::SliderFloat((std::string("MinSlope") + tmp).data(), &tex.minSlope, 0.f, 1.f)) { changed = true; }
                    if (ImGui::SliderFloat((std::string("MaxSlope") + tmp).data(), &tex.maxSlope, 0.f, 1.f)) { changed = true; }
					if (ImGui::SliderFloat((std::string("SlopeFadeDistance") + tmp).data(), &tex.slopeFade, 0.f, 1.f)) { changed = true; }
					ImGui::Separator();
					if (ImGui::SliderFloat((std::string("TextureSizeMultiplier") + tmp).data(), &tex.sizeMultiplier, 1.f, 10.f)) { changed = true; }
					ImGui::Separator();

#pragma region AddTexturePopup
                    if (ImGui::BeginPopup((std::string("Set Diffuse") + tmp).data()))
                    {
						if (ImGui::MenuItem((std::string("None##DIFF") + tmp).data()))
						{
							tex.buf_diffuse = 0;
							changed = true;
						}
                        std::vector<std::pair<std::string, std::vector<std::string>>> meshRes = App->resources->GetAvaliableResources(Component::Type::C_Texture);
                        std::vector<std::pair<std::string, std::vector<std::string>>>::iterator fileIt = meshRes.begin();
                        for (; fileIt != meshRes.end(); fileIt++)
                        {
                            if (ImGui::MenuItem(fileIt->first.data()))
                            {
                                uint64_t UID = App->resources->LinkResource(fileIt->second.front(), Component::Type::C_Texture);
                                tex.buf_diffuse = App->resources->Peek(UID)->Read<R_Texture>()->bufferID;
                                changed = true;
                                break;
                            }
                        }
                        ImGui::EndPopup();
                    }
#pragma endregion

#pragma region AddTexturePopup
                    if (ImGui::BeginPopup((std::string("Set Diff Heightmap") + tmp).data()))
                    {
						if (ImGui::MenuItem((std::string("None##HM") + tmp).data()))
						{
							tex.buf_heightmap = 0;
							changed = true;
						}
                        std::vector<std::pair<std::string, std::vector<std::string>>> meshRes = App->resources->GetAvaliableResources(Component::Type::C_Texture);
                        std::vector<std::pair<std::string, std::vector<std::string>>>::iterator fileIt = meshRes.begin();
                        for (; fileIt != meshRes.end(); fileIt++)
                        {
                            if (ImGui::MenuItem(fileIt->first.data()))
                            {
                                uint64_t UID = App->resources->LinkResource(fileIt->second.front(), Component::Type::C_Texture);
                                tex.buf_heightmap = App->resources->Peek(UID)->Read<R_Texture>()->bufferID;
                                changed = true;
                                break;
                            }
                        }
                        ImGui::EndPopup();
                    }
#pragma endregion

                    if (ImGui::Button((std::string("Diffuse##AddTextureButton") + tmp).data(), ImVec2(125, 20)))
                    {
                        ImGui::OpenPopup((std::string("Set Diffuse") + tmp).data());
                    }
                    ImGui::SameLine();
                    if (ImGui::Button((std::string("Heightmap##AddTextureButton") + tmp).data(), ImVec2(125, 20)))
                    {
                        ImGui::OpenPopup((std::string("Set Diff Heightmap") + tmp).data());
                    }

                    ImGui::Image((ImTextureID)tex.buf_diffuse, ImVec2(125, 125));
                    ImGui::SameLine();
                    ImGui::Image((ImTextureID)tex.buf_heightmap, ImVec2(125, 125));

                    if (changed)
                    {
                        VTerrain::SetTexture(n, tex);
                    }
                    ImGui::EndMenu();
                }
			}
		}

		ImGui::End();
    }

    ShaderEditor();

	if (m_wantRegen && m_regenTimer.Read() > 2000.0f)
	{
		m_regenTimer.Stop();
		m_wantRegen = false;
		GenMap();
	}
    return UPDATE_CONTINUE;
}

// PostUpdate present buffer to screen
update_status ModuleTerrain::PostUpdate()
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleTerrain::CleanUp()
{
	return true;
}

void ModuleTerrain::Render(const viewPort & port)
{
	VTerrain::config.debug.wiredRender = port.useOnlyWires;
	VTerrain::config.debug.renderLight = port.useLighting;
	VTerrain::config.singleSidedFaces = port.useSingleSidedFaces;
	VTerrain::config.debug.renderHeightmap = port.renderHeightMap;
	VTerrain::Render(port.camera->GetViewMatrix().ptr(), port.camera->GetProjectionMatrix().ptr());
}

void ModuleTerrain::SetDefaultTextures()
{
    VTerrain::ConditionalTexture tex;
    uint64_t UID;
    //Water
    tex.minHeight = 0.f;
    tex.maxHeight = 0.285f;
    tex.heightFade = 0.010f;
    tex.minSlope = 0.f;
    tex.maxSlope = 1.f;
    tex.slopeFade = 0.f;
    tex.sizeMultiplier = 4.f;
    UID = App->resources->LinkResource("a_water", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_diffuse = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    UID = App->resources->LinkResource("a_water_hm", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_heightmap = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    VTerrain::SetTexture(0, tex);

    //Sand
    tex.minHeight = 0.f;
    tex.maxHeight = 0.319f;
    tex.heightFade = 0.01f;
    tex.minSlope = 0.f;
    tex.maxSlope = 1.f;
    tex.slopeFade = 0.05f;
    tex.sizeMultiplier = 4.f;
    UID = App->resources->LinkResource("a_sand", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_diffuse = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    UID = App->resources->LinkResource("a_sand_hm", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_heightmap = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    VTerrain::SetTexture(1, tex);

    //Grass
    tex.color = VTerrain::Vec3<float>(108.f / 255.f, 136.f / 255.f, 177.f / 255.f);
    tex.minHeight = 0.045f;
    tex.maxHeight = 0.550f;
    tex.heightFade = 0.010f;
    tex.minSlope = 0.28f;
    tex.maxSlope = 0.82f;
    tex.slopeFade = 0.05f;
    tex.sizeMultiplier = 4.f;
    UID = App->resources->LinkResource("a_grass", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_diffuse = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    UID = App->resources->LinkResource("a_grass_hm", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_heightmap = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    VTerrain::SetTexture(4, tex);

    //Gravel
    tex.color = VTerrain::Vec3<float>(158.f / 255.f, 255.f / 255.f, 0.f / 255.f);
    tex.minHeight = 0.f;
    tex.maxHeight = 0.520f;
    tex.heightFade = 0.010f;
    tex.minSlope = 0.28f;
    tex.maxSlope = 0.882f;
    tex.slopeFade = 0.05f;
    tex.sizeMultiplier = 4.f;
    UID = App->resources->LinkResource("a_gravel", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_diffuse = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    UID = App->resources->LinkResource("a_gravel_hm", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_heightmap = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    VTerrain::SetTexture(5, tex);

    //GreyRock
    tex.color = VTerrain::Vec3<float>(122.f / 255.f, 122.f / 255.f, 122.f / 255.f);
    tex.minHeight = 0.742f;
    tex.maxHeight = 1.f;
    tex.heightFade = 0.010f;
    tex.minSlope = 0.87f;
    tex.maxSlope = 0.95f;
    tex.slopeFade = 0.05f;
    tex.sizeMultiplier = 4.f;
    UID = App->resources->LinkResource("a_rock_grey", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_diffuse = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    UID = App->resources->LinkResource("a_rock_grey_hm", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_heightmap = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    VTerrain::SetTexture(6, tex);

    //Snow
    tex.color = VTerrain::Vec3<float>(1.f, 1.f, 1.f);
    tex.minHeight = 0.618f;
    tex.maxHeight = 1.f;
    tex.heightFade = 0.010f;
    tex.minSlope = 0.f;
    tex.maxSlope = 0.88f;
    tex.slopeFade = 0.05f;
    tex.sizeMultiplier = 4.f;
    UID = App->resources->LinkResource("a_snow", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_diffuse = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    UID = App->resources->LinkResource("a_snow_hm", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_heightmap = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    VTerrain::SetTexture(7, tex);

    //Grey rock
    tex.minHeight = 0.f;
    tex.maxHeight = 1.f;
    tex.heightFade = 0.010f;
    tex.minSlope = 0.954f;
    tex.maxSlope = 1.f;
    tex.slopeFade = 0.05f;
    tex.sizeMultiplier = 4.f;
    UID = App->resources->LinkResource("a_rock_grey", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_diffuse = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    UID = App->resources->LinkResource("a_rock_grey_hm", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_heightmap = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    VTerrain::SetTexture(8, tex);

    //Grass
    tex.minHeight = 0.f;
    tex.maxHeight = 1.f;
    tex.heightFade = 0.010f;
    tex.minSlope = 0.0f;
    tex.maxSlope = 1.f;
    tex.slopeFade = 0.05f;
    tex.sizeMultiplier = 4.f;
    UID = App->resources->LinkResource("a_grass", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_diffuse = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    UID = App->resources->LinkResource("a_grass_hm", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_heightmap = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    VTerrain::SetTexture(9, tex);
}

void ModuleTerrain::GenMap()
{
    VTerrain::config.noise.frequency = m_frequency;
    VTerrain::config.noise.octaves = m_octaves;
    VTerrain::config.noise.lacunarity = m_lacunarity;
    VTerrain::config.noise.persistency = m_persistance;

    VTerrain::CleanChunks();
}

void ModuleTerrain::WantRegen()
{
	m_wantRegen = true;
	m_regenTimer.Start();
}

void ModuleTerrain::ShaderEditor()
{
    if (m_openShaderEditor)
    {
        if (ImGui::Begin("Default Shader Editor", &m_openShaderEditor, ImGuiWindowFlags_NoCollapse))
        {
            bool recompile = false;
            const uint vertexLen = VTerrain::GetVertexShader().length() + 256;
            char* vertexBuf = new char[vertexLen];
            const uint fragmentLen = VTerrain::GetFragmentShader().length() + 256;
            char* fragmentBuf = new char[fragmentLen];
            const uint TESLen = VTerrain::GetTES().length() + 256;
            char* TESbuf = new char[TESLen];
            const uint TCSlen = VTerrain::GetTCS().length() + 256;
            char* TCSbuf = new char[TCSlen];

            strcpy(vertexBuf, VTerrain::GetVertexShader().data());
            if (ImGui::CollapsingHeader("Vertex shader"))
            {
                if (ImGui::InputTextMultiline("##vertexShaderEditor", vertexBuf, vertexLen, ImVec2(ImGui::GetWindowWidth(), 600)));
                {
                    recompile = true;
                }
            }

            strcpy(TCSbuf, VTerrain::GetTCS().data());
            if (ImGui::CollapsingHeader("Tesselation Control Shader"))
            {
                if (ImGui::InputTextMultiline("##TCSEditor", TCSbuf, TCSlen, ImVec2(ImGui::GetWindowWidth(), 600)))
                {
                    recompile = true;
                }
            }

            strcpy(TESbuf, VTerrain::GetTES().data());
            if (ImGui::CollapsingHeader("Tesselation Evaluation Shader"))
            {
                if (ImGui::InputTextMultiline("##TESEditor", TESbuf, TESLen, ImVec2(ImGui::GetWindowWidth(), 600)))
                {
                    recompile = true;
                }
            }

			strcpy(fragmentBuf, VTerrain::GetFragmentShader().data());
			if (ImGui::CollapsingHeader("Fragment shader"))
			{
				if (ImGui::InputTextMultiline("##fragmentShaderEditor", fragmentBuf, fragmentLen, ImVec2(ImGui::GetWindowWidth(), 600)))
				{
					recompile = true;
				}
			}

            if (recompile)
            {
                m_shaderResult = VTerrain::CompileShaders(fragmentBuf, vertexBuf, TCSbuf, TESbuf);
            }

            if (m_shaderResult.length() > 5)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "There were errors while compiling the default shaders:");
                ImGui::TextWrapped(m_shaderResult.data());
            }
            RELEASE_ARRAY(vertexBuf);
            RELEASE_ARRAY(fragmentBuf);
            ImGui::End();
        }
    }
}