#pragma once

struct Shader
{
	int program = -1;

	int modelMatrix = -1;
	int viewMatrix = -1;
	int projectionMatrix = -1;

	int materialColor = -1;
	int hasTexture = -1;
	int useLight = -1;
	int time = -1;
	int ambientColor = -1;
	int globalLightDir = -1;
	int fogDistance = -1;
	int fogColor = -1;
	int maxHeight = -1;
};