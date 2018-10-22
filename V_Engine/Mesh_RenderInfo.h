#ifndef ___MESH_RENDERINFO__
#define ___MESH_RENDERINFO__

#include "Math.h"
#include "Shader.h"

class R_Mesh;

enum AlphaTestTypes
{
	ALPHA_OPAQUE = 0,
	ALPHA_DISCARD,
	ALPHA_BLEND
};

struct Mesh_RenderInfo
{
	const R_Mesh* m_origin = nullptr;

	bool m_drawWired = false;
	bool m_drawFilled = false;
	bool m_drawDoubleSidedFaces = false;
	bool m_drawNormals = false;

	bool m_hasNormals = false;
	bool m_hasUVs = false;

	float4 m_meshColor = float4::zero;
	float4 m_wiresColor = float4::zero;

	unsigned int m_nIndices = 0;
	unsigned int m_nVertices = 0;

	unsigned int m_indicesBuffer = 0;
	unsigned int m_dataBuffer = 0;
	unsigned int m_textureBuffer = 0;

	AlphaTestTypes m_alphaType = AlphaTestTypes::ALPHA_OPAQUE;
	int m_blendType = 0;
	float m_alphaTest = 0.2f;

	Shader m_shader;

	float4x4 m_transform;
};

#endif // !___MESH_RENDERINFO__

