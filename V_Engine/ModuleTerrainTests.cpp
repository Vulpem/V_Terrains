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


#include "OpenGL.h"

#include <time.h>

ModuleTerrain::ModuleTerrain(Application* app, bool start_enabled) :
    Module(app, start_enabled)
    , m_size(128, 128)
    , m_frequency(0.8f)
    , m_octaves(8)
    , m_lacunarity(2.0f)
    , m_persistance (0.4f)
    , m_maxHeight(1000.f)
	, m_fogDistance(5000.f)
    , m_currentHeightCurve("float func(float x)\n{ return pow(x, %i); }")
    , m_curvePow(2)
    , m_setCurvePow(2)
    , m_openShaderEditor(false)
{
	moduleName = "ModuleTerrainTests";
    VTerrain::SetHeightCurve(
        [](float x)
    {
        return pow(x,2);
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
        const int W = static_cast<int>(VTerrain::config.chunkWidth);
        const int H = static_cast<int>(VTerrain::config.chunkHeight);
        int p[2] = {
            floor((pos.x - floor(W / 2.f) + (W % 2 != 0)) / W + 1),
            floor((pos.z - floor(H / 2.f) + (H % 2 != 0)) / H + 1)
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
        ImGui::Text("Render:");
        ImGui::Checkbox("Open shader editor", &m_openShaderEditor);
        int tmpLOD = VTerrain::config.tmp.LOD;
        if (ImGui::SliderInt("LOD", &tmpLOD, 0,VTerrain::config.nLODs - 1))
        {
            VTerrain::config.tmp.LOD = tmpLOD;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(  "Reduce the amount of triangles by a\n"
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
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Text("Terrain generation:");
        if (ImGui::SliderFloat("MaxHeight", &m_maxHeight, 0.0f, 8000.f, "%.3f", 3.f)) { VTerrain::config.maxHeight = m_maxHeight; }
        ImGui::Separator();
        if (ImGui::DragFloat2("Chunk Size", &m_size[0], 1.f, 5.f, 1024.f)) { WantRegen(); }

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
                VTerrain::SetHeightCurve([](float x) {return x;});
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
                VTerrain::SetHeightCurve([n](float x) {return pow(x, n);});
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

        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Separator();
        ImGui::Text("Debug visualization:");

        if (ImGui::Button("Reset Camera Height"))
        {
            float3 pos = App->camera->GetDefaultCam()->GetPosition();
            App->camera->GetDefaultCam()->object->GetTransform()->SetGlobalPos(pos.x, m_maxHeight, pos.y);
        }
        ImGui::Checkbox("Chunk Borders", &VTerrain::config.tmp.renderChunkBorders);
        ImGui::Checkbox("Render Heightmap", &VTerrain::config.tmp.renderHeightmap);

        if (ImGui::Checkbox("Multiple viewports", &App->Editor->multipleViews))
        {
            App->Editor->SwitchViewPorts();
        }
        if (App->Editor->multipleViews)
        {
            ImGui::Spacing();
            ImGui::Checkbox("Auto follow top cam", &App->camera->m_followCamera);
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
	VTerrain::Render(port.camera->GetViewMatrix().ptr(), port.camera->GetProjectionMatrix().ptr());
}

void ModuleTerrain::GenMap()
{
    VTerrain::config.chunkHeight = m_size.x;
    VTerrain::config.chunkWidth = m_size.y;
   

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
        if (ImGui::Begin("Default Shader Editor", &m_openShaderEditor))
        {
            bool recompile = false;
            const uint vertexLen = VTerrain::GetVertexShader().length() + 256;
            char* vertexBuf = new char[vertexLen];
            const uint fragmentLen = VTerrain::GetFragmentShader().length() + 256;
            char* fragmentBuf = new char[fragmentLen];

            strcpy(vertexBuf, VTerrain::GetVertexShader().data());
            if (ImGui::CollapsingHeader("Vertex shader"))
            {
                if (ImGui::InputTextMultiline("##vertexShaderEditor", vertexBuf, vertexLen, ImVec2(ImGui::GetWindowWidth(), 400)))
                {
                    recompile = true;
                }
            }

            strcpy(fragmentBuf, VTerrain::GetFragmentShader().data());
            if (ImGui::CollapsingHeader("Fragment shader"))
            {
                if (ImGui::InputTextMultiline("##fragmentShaderEditor", fragmentBuf, fragmentLen, ImVec2(ImGui::GetWindowWidth(), 400)))
                {
                    recompile = true;
                }
            }

            if (recompile)
            {
                m_shaderResult = VTerrain::CompileShaders(fragmentBuf, vertexBuf);
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