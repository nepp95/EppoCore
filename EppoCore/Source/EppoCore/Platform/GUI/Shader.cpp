#include "pch.h"
#include "Shader.h"

#include <glad/glad.h>

namespace Eppo
{
    Shader::Shader()
    {
        const std::string vertexShaderSource = R"(
            #version 450 core

            layout (location = 0) in vec2 inPosition;
            layout (location = 1) in vec4 inColor;

            layout (location = 0) out vec4 outColor;

            layout(std140, binding = 0) uniform Camera
            {
                mat4 u_ProjectionMatrix;
            };

            void main()
            {
                outColor = inColor;

                vec3 position = vec3(inPosition, 1.0);
                gl_Position = u_ProjectionMatrix * vec4(position, 1.0);
            }
        )";

        const std::string fragmentShaderSource = R"(
            #version 450 core

            layout (location = 0) in vec4 inColor;

            layout (location = 0) out vec4 outFragColor;

            void main()
            {
                outFragColor = inColor;
            }
        )";

        // Compile shaders
        int success = 0;

        const GLchar* vertexSrc = vertexShaderSource.c_str();
        const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            int maxLength = 0;
            glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<char> infoLog(maxLength);
            glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

            EPPO_ERROR("Shader compilation error: {}", infoLog.data());
        }

        const GLchar* fragmentSrc = fragmentShaderSource.c_str();
        const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            int maxLength = 0;
            glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<char> infoLog(maxLength);
            glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

            EPPO_ERROR("Shader compilation error: {}", infoLog.data());
        }

        // Create shader program
        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vertexShader);
        glAttachShader(m_RendererID, fragmentShader);
        glLinkProgram(m_RendererID);
        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
        EPPO_ASSERT(success)

        // Cleanup
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    Shader::~Shader()
    {
        glDeleteProgram(m_RendererID);
    }

    void Shader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    uint32_t Shader::GetRendererID() const
    {
        return m_RendererID;
    }
}
