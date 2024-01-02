#include "pch.h"
#include "Buffer.h"

#include <glad/glad.h>

namespace Eppo
{
	Buffer::Buffer(uint32_t size)
		: m_Size(size)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_RendererID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	Buffer::Buffer(void* data, uint32_t size)
		: m_Size(size)
	{
		glCreateBuffers(1, &m_RendererID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_RendererID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW);
	}

	Buffer::~Buffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void Buffer::Bind() const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
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
