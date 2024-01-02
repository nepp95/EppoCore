#include "pch.h"
#include "Buffer.h"

#include <glad/glad.h>

namespace Eppo
{
	Buffer::Buffer(uint32_t size, uint32_t binding)
		: m_Size(size), m_Binding(binding)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	Buffer::~Buffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void Buffer::SetData(void* data, uint32_t size)
	{
		glNamedBufferSubData(m_RendererID, 0, size, data);
	}

	glm::vec4* Buffer::MapBuffer() const
	{
		return static_cast<glm::vec4*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, m_Size, GL_MAP_READ_BIT));
	}

	void Buffer::UnmapBuffer() const
	{
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
}
