#include "pch.h"
#include "ComputeShader.h"

#include <glad/glad.h>

namespace Eppo
{
	ComputeShader::ComputeShader(const std::string& filepath)
	{
		// Read our shader source code from disk
		std::string fileContents;

		std::ifstream stream(filepath);
		EPPO_ASSERT(stream);

		stream.seekg(0, std::ios::end);
		size_t size = stream.tellg();

		if (size != -1)
		{
			fileContents.resize(size);
			stream.seekg(0, std::ios::beg);
			stream.read(&fileContents[0], fileContents.size());
		}

		const char* shaderSource = fileContents.c_str();

		// Compile our shader source code
		uint32_t computeID = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(computeID, 1, &shaderSource, 0);
		glCompileShader(computeID);
		CheckCompileErrors(computeID, "COMPUTE");

		// Create the shader program
		m_RendererID = glCreateProgram();
		glAttachShader(m_RendererID, computeID);
		glLinkProgram(m_RendererID);
		CheckCompileErrors(m_RendererID, "PROGRAM");

		// Cleanup since this is now baked into the program
		glDeleteShader(computeID);
	}

	void ComputeShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void ComputeShader::Dispatch(uint32_t x, uint32_t y, uint32_t z) const
	{
		glDispatchCompute(x, y, z);
	}

	void ComputeShader::MemBarrier() const
	{
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void ComputeShader::CheckCompileErrors(GLuint shader, std::string type)
	{
		// From learnopengl.com
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
}
