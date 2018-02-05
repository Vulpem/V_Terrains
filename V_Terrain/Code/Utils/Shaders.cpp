#include "Shaders.h"

#include "../ExternalLibs/Glew/include/glew.h"

namespace VTerrain
{
    std::string Shaders::m_defaultVertexShader = std::string(
        "#version 330 core\n"
        "\n"
        "layout (location = 0) in vec3 position;\n"
        "layout (location = 1) in vec3 normal;\n"
        "layout (location = 2) in vec2 texCoord;\n"
        "\n"
        "out vec4 ourColor;\n"
        "out vec2 TexCoord;\n"
        "\n"
        "uniform mat4 model_matrix;\n"
        "uniform mat4 view_matrix;\n"
        "uniform mat4 projection_matrix;\n"
        "uniform vec4 material_color;\n"
        "uniform vec3 global_light_direction;\n"
        "uniform vec4 ambient_color;\n"
        "uniform int use_light;\n"
        "\n"
        "void main()\n"
        "{\n"
        "	mat4 transform = projection_matrix * view_matrix * model_matrix;\n"
        "	gl_Position = transform * vec4(position, 1.0f);\n"
        "   if (use_light != 0)\n"
        "   {\n"
        "		vec3 norm = mat3(model_matrix) * normal;\n"
        "		float light_intensity = dot(global_light_direction, norm);\n"
        "		light_intensity = max(light_intensity,ambient_color.x);\n"
        "		ourColor = material_color * light_intensity;\n"
        "		ourColor.w = material_color.w;\n"
        "   }\n"
        "   else\n"
        "   {\n"
        "		ourColor = material_color;\n"
        "   }\n"
        "	TexCoord = texCoord;\n"
        "}\n"
    );

    std::string Shaders::m_defaultFragmentShader = std::string(
        "#version 330 core\n"
        "\n"
        "in vec4 ourColor;\n"
        "in vec2 TexCoord;\n"
        "\n"
        "out vec4 color;\n"
        "\n"
        "uniform sampler2D ourTexture;\n"
        "uniform int has_texture;\n"
        "\n"
        "void main()\n"
        "{\n"
        "if(has_texture != 0) {\n"
        "color = ourColor * texture(ourTexture, TexCoord);\n"
        "} else {\n"
        "color = ourColor; \n"
        "}\n"
        "}\n"
    );

    std::string Shaders::CompileShader(const char * vertexBuf, const char * fragmentBuf, uint & shaderProgram)
    {
        std::string ret;
        bool error = false;
        shaderProgram = 0;
        GLint success;

        ret += "\n------ Vertex shader ------\n";
        GLuint vertexShader;
        if (vertexBuf == nullptr)
        {
            ret += "- No vertex shader found. Using default vertex shader.\n";
            vertexBuf = m_defaultVertexShader.c_str();
        }

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexBuf, NULL);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (success == 0)
        {
            error = true;
            GLchar infoLog[512];
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            ret += infoLog;
            ret += '\n';
        }
        else
        {
            ret += "Compilation succesfull\n";
        }

        ret += "\n------ Fragment shader ------\n";
        GLuint fragmentShader;
        if (fragmentBuf == nullptr)
        {
            ret += "- No fragment shader found. Using default fragment shader.\n";
            fragmentBuf = m_defaultFragmentShader.c_str();
        }

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentBuf, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (success == 0)
        {
            error = true;
            GLchar infoLog[512];
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            ret += infoLog;
            ret += '\n';
        }
        else
        {
            ret += "Compilation succesfull\n";
        }

        GLuint program;
        program = glCreateProgram();

        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (success == 0)
        {
            error = true;
            GLchar infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            ret += "\n------ Shader Program ------\n";
            ret += infoLog;
            ret += '\n';
        }

        if (program != 0 && error == false)
        {
            shaderProgram = program;
        }
        else

        {
            ret += "Error Compiling shader";
        }

        glDetachShader(shaderProgram, vertexShader);
        glDetachShader(shaderProgram, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return ret;
    }

    void Shaders::FreeShader(uint shaderProgram)
    {
        glDeleteProgram(shaderProgram);
    }
}