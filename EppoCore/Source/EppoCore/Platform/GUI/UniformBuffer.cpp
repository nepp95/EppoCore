#include "pch.h"
#include "UniformBuffer.h"

namespace Eppo
{
    UniformBuffer::UniformBuffer(Device& device, const uint32_t size, const uint32_t binding)
        : m_Binding(binding),
          m_Buffer(
              device,
              GpuBufferSpecification{
                  .Size = size, .Usage = BufferUsage::Uniform, .Memory = MemoryUsage::CpuToGpu, .DebugName = "Uniform buffer" }
          )
    {
    }

    UniformBuffer::~UniformBuffer() = default;

    auto UniformBuffer::SetData(const void* data, const uint32_t size) -> void
    {
        m_Buffer.Upload(data, size);
    }
}
