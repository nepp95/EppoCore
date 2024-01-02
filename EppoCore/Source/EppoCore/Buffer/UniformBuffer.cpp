#include "pch.h"
#include "UniformBuffer.h"

#include <glad/glad.h>

namespace Eppo
{
	UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding)
		: m_Size(size), m_Binding(binding)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	UniformBuffer::~UniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void UniformBuffer::SetData(void* data, uint32_t size)
	{
		// TODO: What if size is bigger than m_Size?
		glNamedBufferSubData(m_RendererID, 0, size, data);
	}

	void UniformBuffer::Recreate()
	{
		// TODO: Well if the size is bigger. Do this?
	}
}
