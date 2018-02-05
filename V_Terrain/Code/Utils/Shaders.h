#pragma once

#include "Globals.h"

namespace VTerrain
{
    class Shaders
    {
    public:
        static std::string CompileShader(const char* vertexBuf, const char* fragmentBuf, uint & shaderProgram);
        static void FreeShader(uint shaderProgram);

        static std::string m_defaultVertexShader;
        static std::string m_defaultFragmentShader;
    };
}