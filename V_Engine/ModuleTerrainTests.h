#ifndef __MODULE_TERRAIN_TESTS__
#define __MODULE_TERRAIN_TESTS__

#include "Module.h"
#include "Globals.h"

#include "../V_Terrain/Code/Include.h"

#include "Math.h"

#define COL_N 61
#define COL_D 25

void ShowError(const char* message);

class ModuleTerrain : public Module
{
public:
    ModuleTerrain(Application* app, bool start_enabled = true);
	~ModuleTerrain();

	bool Init();
	bool Start();
	update_status PreUpdate() override;
    update_status Update() override;
	update_status PostUpdate() override;
	bool CleanUp();

	void SaveTerrainConfig(std::string configName);
	void LoadTerrainConfig(std::string configName);
	void LoadTerrainNow(std::string configName);

	void SetImage(int n, std::string textureFile);
	void SetHeightmap(int n, std::string hmfile);

	void Render(const viewPort& port) override;

	void SetDefaultTextures();

    void GenMap();
	void WantRegen();
    void ShaderEditor();

    //NoiseGeneration
    uint32_t seed;

    bool m_openShaderEditor;
    std::string m_shaderResult;

    std::string m_vertex;
    std::string m_TCS;
    std::string m_TES;
    std::string m_fragment;

    int m_maxTexturesGL;

	bool m_wantRegen;
	Timer m_regenTimer;

    float m_frequency;
    int m_octaves;
    float m_lacunarity;
    float m_persistance;

    int m_resolution;
    std::string m_currentHeightCurve;

    float m_maxHeight;
	float m_fogDistance;
	float m_variableFogDistance;
	Timer m_smoothRegen;
	bool m_regening = false;
	bool m_generatedMap = false;

    uint m_shaderProgram;

    int m_curvePow = 1;
    int m_setCurvePow = 1;

    float m_chunkSize = 256.f;

    float m_globalLightDir = 0.f;
    float m_globalLightHeight = 0.f;
	std::string terrainToLoad;

	bool m_calcCollisions = false;
	float3 m_terrainPos[COL_N][COL_N];
	float3 m_terrainNormal[COL_N][COL_N];

	std::string textures[10];
	std::string heightmaps[10];

	char terrainConfigName[256];
};

#endif