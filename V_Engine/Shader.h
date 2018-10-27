#pragma once

struct Shader
{
	int m_program = -1;

	int m_modelMatrix = -1;
	int m_viewMatrix = -1;
	int m_projectionMatrix = -1;

	int m_materialColor = -1;
	int m_hasTexture = -1;
	int m_useLight = -1;
	int m_time = -1;
	int m_ambientColor = -1;
	int m_globalLightDir = -1;
	int m_fogDistance = -1;
	int m_fogColor = -1;
	int m_maxHeight = -1;
};