#pragma once

#include "EppoCore/GPU/GpuBuffer.h"

namespace Eppo
{
    class Device;

    // Host-writable uniform buffer for one binding in the push descriptor set (set 0)
    class UniformBuffer
    {
    public:
        UniformBuffer(Device& device, uint32_t size, uint32_t binding);
        ~UniformBuffer();

        UniformBuffer(const UniformBuffer&) = delete;
        auto operator=(const UniformBuffer&) -> UniformBuffer& = delete;
        UniformBuffer(UniformBuffer&&) = delete;
        auto operator=(UniformBuffer&&) -> UniformBuffer& = delete;

        auto SetData(const void* data, uint32_t size) -> void;

        [[nodiscard]] auto GetBuffer() const -> const GpuBuffer& { return m_Buffer; }
        [[nodiscard]] auto GetBinding() const -> uint32_t { return m_Binding; }

    private:
        uint32_t m_Binding = 0;
        GpuBuffer m_Buffer;
    };
}
