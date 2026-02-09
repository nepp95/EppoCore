#include "pch.h"
#include "Shader.h"

#include <glad/glad.h>

#include <utility>

namespace Eppo
{
    Shader::Shader(const std::array<std::filesystem::path, 2>& filepaths)
    {
        // Read shader source from file
        const std::string vertexSource = ReadFile(filepaths[0]);
        const std::string fragmentSource = ReadFile(filepaths[1]);

        // Compile shaders
        int success = 0;

        const GLchar* vertexSrc = vertexSource.c_str();
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

            Log::Error("Shader compilation error: {}", infoLog.data());
        }

        const GLchar* fragmentSrc = fragmentSource.c_str();
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

            Log::Error("Shader compilation error: {}", infoLog.data());
        }

        // Create shader program
        m_RendererID = glCreateProgram();
        glAttachShader(m_RendererID, vertexShader);
        glAttachShader(m_RendererID, fragmentShader);
        glLinkProgram(m_RendererID);
        glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
        EP_ASSERT(success);

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

    std::string Shader::ReadFile(const std::filesystem::path& filepath)
    {
        std::ifstream in(filepath, std::ios::in | std::ios::binary);
        std::string result;

        if (in)
        {
            // Get size of source
            in.seekg(0, std::ios::end);
            size_t size = in.tellg();

            if (size != -1)
            {
                result.resize(in.tellg());
                in.seekg(0, std::ios::beg);

                // Read source
                in.read(&result[0], result.size());
            }
            else
            {
                Log::Error("Failed to read shader file '{}'", filepath);
            }
        }
        else
        {
            Log::Error("Failed to open shader file '{}'", filepath);
        }

        return result;
    }
}
