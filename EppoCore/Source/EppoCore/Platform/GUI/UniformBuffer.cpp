#include "pch.h"
#include "UniformBuffer.h"

#include <glad/glad.h>

namespace Eppo
{
    UniformBuffer::UniformBuffer(const uint32_t size, const uint32_t binding)
    {
        glCreateBuffers(1, &m_RendererID);
        glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
    }

    UniformBuffer::~UniformBuffer()
    {
        glDeleteBuffers(1, &m_RendererID);
    }

    void UniformBuffer::SetData(const void* data, const uint32_t size) const
    {
        glNamedBufferSubData(m_RendererID, 0, size, data);
    }
}
