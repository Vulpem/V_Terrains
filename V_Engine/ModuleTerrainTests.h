#ifndef __MODULE_TERRAIN_TESTS__
#define __MODULE_TERRAIN_TESTS__

#include "Module.h"
#include "Globals.h"

#include "../V_Terrain/Code/Include.h"

#include "Math.h"

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

	void Render(const viewPort& port) override;

    void GenMap();
	void WantRegen();

    //NoiseGeneration
    uint32_t seed;

	bool m_wantRegen;
	Timer m_regenTimer;

    float m_frequency;
    int m_octaves;
    float m_lacunarity;
    float m_persistance;

    float2 m_size;
    float2 m_offset;

    float m_maxHeight;
	float m_fogDistance;

    uint m_shaderProgram;

    int m_curvePow = 1;

    float m_globalLightDir = 0.f;
    float m_globalLightHeight = 0.f;
};

#endif