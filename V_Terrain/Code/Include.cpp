#include "Include.h"

#include "Noise\PerlinNoise.h"

namespace VTerrain
{
    void Init()
    {
        VTerrain::Shaders::CompileShader(nullptr, NULL, MeshGenerator::Mesh::m_shaderProgram);
    }
}