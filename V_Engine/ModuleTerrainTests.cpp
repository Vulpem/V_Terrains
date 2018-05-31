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

void ShowError(const char * message)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message, nullptr);
}

ModuleTerrain::ModuleTerrain(Application* app, bool start_enabled) :
    Module(app, start_enabled)
    , m_resolution(RPGT::config.chunkHeightmapResolution)
    , m_frequency(RPGT::config.noise.frequency)
    , m_octaves(RPGT::config.noise.octaves)
    , m_lacunarity(RPGT::config.noise.lacunarity)
    , m_persistance (RPGT::config.noise.persistency)
    , m_maxHeight(RPGT::config.maxHeight)
	, m_fogDistance(RPGT::config.fogDistance)
    , m_currentHeightCurve("float func(float x)\n{ return pow(x, %i); }")
    , m_curvePow(2)
    , m_setCurvePow(2)
    , m_openShaderEditor(false)
{
	moduleName = "ModuleTerrainTests";

	RPGT::config.throwErrorFunc = ShowError;

    RPGT::SetHeightCurve(
    [](float x) {
        return pow(x, 2.f);
    });

    m_vertex = RPGT::GetDefaultVertexShader();
    m_vertex.reserve(m_vertex.capacity() + 1024);
    m_TCS = RPGT::GetDefaultTCS();
    m_TCS.reserve(m_TCS.capacity() + 1024);
    m_TES = RPGT::GetDefaultTES();
    m_TES.reserve(m_TES.capacity() + 1024);
    m_fragment = RPGT::GetDefaultFragmentShader();
    m_fragment.reserve(m_fragment.capacity() + 1024);
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
    RPGT::Init();

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
	RPGT::Update(pos.x, pos.z);

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
        const int HMresolution = static_cast<int>(RPGT::config.chunkSize);
        int p[2] = {
            floor((pos.x - floor(HMresolution / 2.f) + (HMresolution % 2 != 0)) / HMresolution + 1),
            floor((pos.z - floor(HMresolution / 2.f) + (HMresolution % 2 != 0)) / HMresolution + 1)
        };
        ImGui::Text("Current chunk:\nX: %i,   Z: %i", p[0], p[1]);
        ImGui::NewLine();
        if (ImGui::Button("Generate new random seed"))
        {
            RPGT::SetSeed(time(NULL));
        }

        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Separator();
		ImGui::Checkbox("Open shader editor", &m_openShaderEditor);
		if (ImGui::CollapsingHeader("Render"))
		{
			ImGui::SliderFloat("FogDistance", &RPGT::config.fogDistance, 0.0f, 100000.f, "%.3f", 4.f);
			ImGui::SliderFloat("WaterHeight", &RPGT::config.waterHeight, 0.0f, 1.f);
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
				RPGT::config.globalLight[0] = tmp.x;
				RPGT::config.globalLight[1] = tmp.y;
				RPGT::config.globalLight[2] = tmp.z;
			}

			ImGui::DragFloat3("Light vector", RPGT::config.globalLight);
			ImGui::NewLine();
		}
		if (ImGui::CollapsingHeader("Terrain generation"))
		{
			if (ImGui::SliderFloat("MaxHeight", &m_maxHeight, 0.0f, 8000.f, "%.3f", 3.f)) { RPGT::config.maxHeight = m_maxHeight; }
			ImGui::Separator();
			if (ImGui::DragInt("Heigtmap resolution", &m_resolution, 1.f, 4, 1024)) { RPGT::config.chunkHeightmapResolution = m_resolution; WantRegen(); }
			if (ImGui::DragFloat("Chunk Size", &RPGT::config.chunkSize, 1.f, 5.f, 2048.f)) { WantRegen(); }
            if (ImGui::DragInt("Min chunk polys", &(int)RPGT::config.chunkMinDensity, 1.f, 1, 64)) { WantRegen(); }

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
			if (ImGui::SliderInt("Ridged depth", &(int)RPGT::config.noise.ridgedDepth, 0, RPGT::config.noise.octaves)) { WantRegen(); }
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
					RPGT::SetHeightCurve([](float x) {return x; });
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
					RPGT::SetHeightCurve([n](float x) {return pow(x, n); });
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
					RPGT::SetHeightCurve([n](float x)
					{
						x = x * 2.f - 1.f;
						return (pow(x, n) + 1.f) * 0.5f;
					});
				}
                ImGui::Separator();
                if (ImGui::MenuItem(
                    "float func(float x)\n"
                    "{\n"
                    "   return 1.f / (1.f + exp(-P*(x * 2.f - 1.f)));\n"
                    "}"
                ))
                {
                    m_setCurvePow = m_curvePow;
                    m_currentHeightCurve =
                        "float func(float x)\n"
                        "{\n"
                        "   return 1.f / (1.f + exp(-P*(x * 2.f - 1.f)));\n"
                        "}";
                    int n = m_curvePow;
                    RPGT::SetHeightCurve([n](float x)
                    {
                        return 1.f / (1.f + exp((float)(-n)*(x * 2.f - 1.f)));
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
			ImGui::Checkbox("Chunk Borders", &RPGT::config.debug.renderChunkBorders);

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
				RPGT::ConditionalTexture tex = RPGT::GetTexture(n);
                bool changed = false;
                char tmp[8];
                sprintf_s(tmp, "##%i", n);

				ImGui::ColorButton(tmp, ImVec4(tex.color[0], tex.color[1], tex.color[2], 1.f));
				ImGui::SameLine();
				ImGui::Image((ImTextureID)tex.buf_diffuse, ImVec2(20, 20));
				ImGui::SameLine();
				ImGui::Image((ImTextureID)tex.buf_heightmap, ImVec2(20, 20));
				ImGui::SameLine();
                if (ImGui::BeginMenu((std::string("Texture ") + tmp).data()))
                {
                    if (ImGui::ColorEdit3((std::string("Color") + tmp).data(), tex.color)) { changed = true; }
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
                        RPGT::SetTexture(n, tex);
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
	RPGT::config.debug.wiredRender = port.useOnlyWires;
	RPGT::config.debug.renderLight = port.useLighting;
	RPGT::config.singleSidedFaces = port.useSingleSidedFaces;
	RPGT::config.debug.renderHeightmap = port.renderHeightMap;
	RPGT::Render(port.camera->GetViewMatrix().ptr(), port.camera->GetProjectionMatrix().ptr());
}

void ModuleTerrain::SetDefaultTextures()
{
    RPGT::ConditionalTexture tex;
    uint64_t UID;
    //Water
    tex.minHeight = 0.f;
    tex.maxHeight = 0.005f;
    tex.heightFade = 0.f;
    tex.minSlope = 0.f;
    tex.maxSlope = 1.f;
    tex.slopeFade = 0.f;
    tex.sizeMultiplier = 4.f;
    UID = App->resources->LinkResource("a_water", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_diffuse = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    UID = App->resources->LinkResource("a_water_hm", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_heightmap = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    RPGT::SetTexture(0, tex);

    //Sand
    tex.minHeight = 0.f;
    tex.maxHeight = 0.01f;
    tex.heightFade = 0.001f;
    tex.minSlope = 0.f;
    tex.maxSlope = 1.f;
    tex.slopeFade = 0.05f;
    tex.sizeMultiplier = 4.f;
    UID = App->resources->LinkResource("a_sand", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_diffuse = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    UID = App->resources->LinkResource("a_sand_hm", Component::Type::C_Texture);
    if (UID != 0) { tex.buf_heightmap = App->resources->Peek(UID)->Read<R_Texture>()->bufferID; }
    RPGT::SetTexture(1, tex);

    //Grass
    tex.color[0] = 108.f / 255.f;
    tex.color[1] = 136.f / 255.f;
    tex.color[2] = 177.f / 255.f;
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
    RPGT::SetTexture(4, tex);

    //Gravel
    tex.color[0] = 158.f / 255.f;
    tex.color[1] = 255.f / 255.f;
    tex.color[2] = 0.f / 255.f;
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
    RPGT::SetTexture(5, tex);

    //GreyRock
    tex.color[0] = 122.f / 255.f;
    tex.color[1] = 122.f / 255.f;
    tex.color[2] = 122.f / 255.f;
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
    RPGT::SetTexture(6, tex);

    //Snow
    tex.color[0] = 1.f;
    tex.color[1] = 1.f;
    tex.color[2] = 1.f;
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
    RPGT::SetTexture(7, tex);

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
    RPGT::SetTexture(8, tex);

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
    RPGT::SetTexture(9, tex);
}

void ModuleTerrain::GenMap()
{
    RPGT::config.noise.frequency = m_frequency;
    RPGT::config.noise.octaves = m_octaves;
    RPGT::config.noise.lacunarity = m_lacunarity;
    RPGT::config.noise.persistency = m_persistance;

    RPGT::RegenerateMesh();
    RPGT::CleanChunks();
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
            if (ImGui::Button("Save shaders"))
            {
                RPGT::SaveShader(m_vertex, "vertex.cpp");
                RPGT::SaveShader(m_TCS, "TCS.cpp");
                RPGT::SaveShader(m_TES, "TES.cpp");
                RPGT::SaveShader(m_fragment, "fragment.cpp");
            }

            if (ImGui::CollapsingHeader("Vertex shader"))
            {
                bool refresh = false;
                if (m_vertex.length() + 512 >= m_vertex.capacity()) { m_vertex.reserve(m_vertex.capacity() + 512); refresh = true; }
                if (ImGui::InputTextMultiline((refresh ? "##vertexShaderEditorRefreshing" : "##vertexShaderEditor"), (char*)m_vertex.data(), m_vertex.capacity(), ImVec2(ImGui::GetWindowWidth() - 50, 600), ImGuiInputTextFlags_::ImGuiInputTextFlags_AllowTabInput));
                {
                    //Since we're brute-forcing the characters into the string, this will update it's variables (like "length") to the real size. This a debug-only feature
                    m_vertex = m_vertex.data();
                    recompile = true;
                }
            }

            if (ImGui::CollapsingHeader("TCS shader"))
            {
                bool refresh = false;
                if (m_TCS.length() + 512 >= m_TCS.capacity()) { m_TCS.reserve(m_TCS.capacity() + 512); refresh = true; }
                if (ImGui::InputTextMultiline((refresh ? "##TCSShaderEditorRefreshing" : "##TCSShaderEditor"), (char*)m_TCS.data(), m_TCS.capacity(), ImVec2(ImGui::GetWindowWidth() - 50, 600), ImGuiInputTextFlags_::ImGuiInputTextFlags_AllowTabInput));
                {
                    //Since we're brute-forcing the characters into the string, this will update it's variables (like "length") to the real size. This a debug-only feature
                    m_TCS = m_TCS.data();
                    recompile = true;
                }
            }

            if (ImGui::CollapsingHeader("TES shader"))
            {
                bool refresh = false;
                if (m_TES.length() + 512 >= m_TES.capacity()) { m_TES.reserve(m_TES.capacity() + 512); refresh = true; }
                if (ImGui::InputTextMultiline((refresh ? "##TESShaderEditorRefreshing" : "##TESShaderEditor"), (char*)m_TES.data(), m_TES.capacity(), ImVec2(ImGui::GetWindowWidth() - 50, 600), ImGuiInputTextFlags_::ImGuiInputTextFlags_AllowTabInput));
                {
                    //Since we're brute-forcing the characters into the string, this will update it's variables (like "length") to the real size. This a debug-only feature
                    m_TES = m_TES.data();
                    recompile = true;
                }
            }

            if (ImGui::CollapsingHeader("Fragment shader"))
            {
                bool refresh = false;
                if (m_fragment.length() + 512 >= m_fragment.capacity()) { m_fragment.reserve(m_fragment.capacity() + 512); refresh = true; }
                if (ImGui::InputTextMultiline((refresh ? "##FragmentShaderEditorRefreshing" : "##FragmentShaderEditor"), (char*)m_fragment.data(), m_fragment.capacity(), ImVec2(ImGui::GetWindowWidth() - 50, 600), ImGuiInputTextFlags_::ImGuiInputTextFlags_AllowTabInput));
                {
                    //Since we're brute-forcing the characters into the string, this will update it's variables (like "length") to the real size. This a debug-only feature
                    m_fragment = m_fragment.data();
                    recompile = true;
                }
            }

            if (recompile)
            {
                m_shaderResult = RPGT::CompileShaders(m_fragment.data(), m_vertex.data(), m_TCS.data(), m_TES.data());
            }

            if (m_shaderResult.length() > 5)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "There were errors while compiling the default shaders:");
                ImGui::TextWrapped(m_shaderResult.data());
            }

            ImGui::End();
        }
    }
}