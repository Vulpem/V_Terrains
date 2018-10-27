#ifndef __MODULE_RENDER_3D__
#define __MODULE_RENDER_3D__

#include "Module.h"
#include "Globals.h"
#include "Light.h"
#include <map>

#define MAX_LIGHTS 8

struct Mesh_RenderInfo;
struct ViewPort;
class Camera;

class ModuleRenderer3D : public Module
{
public:
	ModuleRenderer3D(Application* app, bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init() override;
	UpdateStatus PreUpdate() override;
	UpdateStatus PostUpdate() override;
	void CleanUp() override;

	void OnScreenResize(int width, int heigth) override;
	void UpdateProjectionMatrix(Camera* cam);

	void RenderBlendObjects();

	void DrawLine(float3 a, float3 b, float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f));
	void DrawBox(float3* corners, float4 color = float4(1.0f, 1.0f, 0.6f, 1.0f));
	void DrawLocator(float4x4 transform = float4x4::identity, float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f));
	void DrawLocator(float3 position, float4 color = float4(0.1f, 0.58f, 0.2f, 1.0f));
	void DrawMesh(Mesh_RenderInfo& meshInfo, bool renderBlends = false);

	ViewPort* HoveringViewPort();
	float2 ViewPortToScreen(const float2& pos_in_ViewPort, ViewPort** OUT_port = NULL);
	float2 ScreenToViewPort(const float2& pos_in_screen, ViewPort** OUT_port = NULL);

	uint AddViewPort(float2 pos, float2 size, Camera* cam);
	ViewPort* FindViewPort(uint ID);
	bool DeleteViewPort(uint ID);

	void SetViewPort(ViewPort& port);
public:
	std::vector<ViewPort> m_viewPorts;
	Light m_lights[MAX_LIGHTS];

	float4 m_ambientLight = float4(0.3f, 0.3f, 0.3f, 1.0f);
	float3 m_sunDirection = float3(1.0f, 1.0f, 1.0f);
	float3 m_clearColor = float3(0.78f, 0.81f, 0.84f);

private:
	void RenderMeshWired(const Mesh_RenderInfo& data);
	void RenderMeshFilled(const Mesh_RenderInfo& data);
	void RenderNormals(const Mesh_RenderInfo& data);

private:
	std::multimap<float, Mesh_RenderInfo> m_alphaObjects;
	ViewPort* m_currentViewPort = nullptr;

	SDL_GLContext m_GLcontext;

	bool m_usingLights = true;
	bool m_usingSingleSidedFaces = true;
	bool m_usingTextures = true;
};

#endif