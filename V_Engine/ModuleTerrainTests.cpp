#include "Application.h"
#include "ModuleTerrainTests.h"

#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ViewPort.h"

#include "ModuleInput.h"
#include "imGUI\imgui.h"


#include "OpenGL.h"

ModuleTerrain::ModuleTerrain(Application* app, bool start_enabled) :
    Module(app, start_enabled)
    , m_offset(0.f, 0.f)
    , m_size(128, 128)
    , m_frequency(0.8f)
    , m_octaves(8)
    , m_lacunarity(2.0f)
    , m_persistance (0.4f)
    , m_simplifyRender(false)
    , m_simplifyRenderStep(0.2f)
    , m_maxHeight(100.f)
{
	moduleName = "ModuleTerrainTests";
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

    std::vector<float> tmp = { 0.f,0.f,0.f };
    m_heightmapBuffer = VTerrain::GenImage::FromRGB(tmp, 1, 1);
    GenMap();

    

    std::vector<float> img;
    for (uint y = 0; y < m_size.y; y++)
    {
        for (uint x = 0; x < m_size.x; x++)
        {
            const float val1 = 0.1f;
            const float val2 = 0.9f;
            const uint squareSize = 2;
			if (x < 1 || y < 1 || x >= m_size.x - 1 || y >= m_size.y - 1)
			{
				img.push_back(val1);
				img.push_back(val1);
				img.push_back(val2);
			}
			else
			{
				img.push_back(val2);
				img.push_back(val2);
				img.push_back(val2);
			}
        }
    }
	m_squaredPatternBuf = VTerrain::GenImage::FromRGB(img, m_size.x, m_size.y);
	VTerrain::Config::TMP::debugTexBuf = m_squaredPatternBuf;

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
    VTerrain::ChunkManager::Update(pos.x, pos.z);

    ImGui::SetNextWindowPos(ImVec2(0.f, 20.f));

    if (ImGui::Begin("TerrainTests"))
    {
        const int W = static_cast<int>(VTerrain::Config::chunkWidth);
        const int H = static_cast<int>(VTerrain::Config::chunkHeight);
        int p[2] = {
            floor((pos.x - floor(W / 2.f) + (W % 2 != 0)) / W + 1),
            floor((pos.z - floor(H / 2.f) + (H % 2 != 0)) / H + 1)
        };
        ImGui::DragInt2("CurrentChunk", p);

        int tmpLOD = VTerrain::Config::TMP::LOD;
        if (ImGui::SliderInt("LOD", &tmpLOD, 0,VTerrain::Config::nLODs - 1))
        {
            VTerrain::Config::TMP::LOD = tmpLOD;
        }


		if (ImGui::Checkbox("DebugSquaredPattern", &m_showDebugPattern))
		{
			if (m_showDebugPattern) { VTerrain::Config::TMP::debugTexBuf = m_squaredPatternBuf; }
			else { VTerrain::Config::TMP::debugTexBuf = 0; }
		}
		if (ImGui::SliderFloat("LightDir", &m_globalLightDir, -360, 360)
			|| ImGui::SliderFloat("LightHeight", &m_globalLightHeight, -90, 90))
		{
			float3 tmp(1.f, 0.f, 0.f); //cos(m_globalLightDir * DEGTORAD),0 ,sin(m_globalLightDir* DEGTORAD));
			Quat rot = Quat::FromEulerYZX(m_globalLightDir * DEGTORAD, m_globalLightHeight*DEGTORAD, 0.f);
			tmp = rot * tmp;
			VTerrain::Config::globalLight[0] = tmp.x;
			VTerrain::Config::globalLight[1] = tmp.y;
			VTerrain::Config::globalLight[2] = tmp.z;
		}

        ImGui::DragFloat3("GlobalLightDirection", VTerrain::Config::globalLight);

        if (ImGui::DragFloat("MaxHeight", &m_maxHeight, 0.1f, 0.1f, 64.f)) { WantRegen(); }

        if (ImGui::DragFloat2("Size", &m_size[0], 1.f, 5.f, 1024.f)) { WantRegen(); }

        if (ImGui::DragFloat2("Offset", &m_offset[0], 1.f)) { WantRegen(); }
        ImGui::Separator();

        if (ImGui::SliderFloat("##Frequency", &m_frequency, 0.1f, 20.f)) { WantRegen(); }
        if (ImGui::DragFloat("Frequency", &m_frequency, 0.1f, 0.1f, 64.f)) { WantRegen(); }
        ImGui::Separator();

        if (ImGui::SliderInt("##Octaves", &m_octaves, 1, 16)) { WantRegen(); }
        if (ImGui::DragInt("Octaves", &m_octaves, 1, 1, 16)) { WantRegen(); }
        ImGui::Separator();

        if (ImGui::SliderFloat("##Lacunarity", &m_lacunarity, 0.1f, 20.f)) { WantRegen(); }
        if (ImGui::DragFloat("Lacunarity", &m_lacunarity, 0.1f, 0.1f, 20.f)) { WantRegen(); }
        ImGui::Separator();

        if (ImGui::SliderFloat("##Persistance", &m_persistance, 0.01f, 1.f)) { WantRegen(); }
        if (ImGui::DragFloat("Persistance", &m_persistance, 0.1f, 0.01f, 1.f)) { WantRegen(); }
        ImGui::Separator();

        if (ImGui::Checkbox("Simplify render", &m_simplifyRender)) { WantRegen(); }
        if (ImGui::SliderFloat("Simplify render step", &m_simplifyRenderStep, 0.01f, 1.f)) { WantRegen(); }
        ImGui::NewLine();

        float width = ImGui::GetWindowWidth() - 50;
        ImGui::Image((void*)m_heightmapBuffer, ImVec2(width, width));
        ImGui::End();
    }

	if (m_wantRegen && m_regenTimer.Read() > 2000.0f)
	{
		m_regenTimer.Stop();
		m_wantRegen = false;
		GenMap();
		VTerrain::ChunkManager::RegenAll();
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
    VTerrain::GenImage::FreeImage(m_heightmapBuffer);
	return true;
}

void ModuleTerrain::Render(const viewPort & port)
{
	VTerrain::ChunkManager::Render(port.camera->GetViewMatrix().ptr(), port.camera->GetProjectionMatrix().ptr());
}

void ModuleTerrain::GenMap()
{
    VTerrain::Config::chunkHeight = m_size.x;
    VTerrain::Config::chunkWidth = m_size.y;
    VTerrain::Config::maxHeight = m_maxHeight;

    VTerrain::Config::Noise::frequency = m_frequency;
    VTerrain::Config::Noise::octaves = m_octaves;
    VTerrain::Config::Noise::lacunarity = m_lacunarity;
    VTerrain::Config::Noise::persistency = m_persistance;
}

void ModuleTerrain::WantRegen()
{
	m_wantRegen = true;
	m_regenTimer.Start();
}
