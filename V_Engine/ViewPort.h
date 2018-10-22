#ifndef __VIEWPORT__
#define __VIEWPORT__

#include "Math.h"
#include "Camera.h"
#include "ModuleWindow.h"

struct ViewPort
{
	ViewPort(float2 pos, float2 size, Camera* cam, uint id)
		: m_pos(pos)
		, m_size(size)
		, m_camera(cam)
		, m_ID(id)
	{};

	bool m_active = true;
	bool m_withUI = true;
	bool m_autoRender = true;

	bool m_useLighting = true;
	bool m_useSingleSidedFaces = true;
	bool m_useOnlyWires = false;
	bool m_renderHeightMap = false;
	bool m_renderTerrain = true;
	bool m_renderTerrainCollisions = false;
	bool m_renderBoundingBoxes = false;
	bool m_renderChunkBorders = false;

	Camera* m_camera = nullptr;
	float2 m_pos = float2::zero;
	float2 m_size = float2(100,100);

	uint m_ID = -1;

	void SetCameraMatrix()
	{
		if (m_camera != nullptr)
		{
			m_camera->aspectRatio = m_size.x / m_size.y;
			m_camera->SetHorizontalFOV(m_camera->GetFrustum()->horizontalFov);
		}
	}

};


#endif // !__VIEWPORT__
