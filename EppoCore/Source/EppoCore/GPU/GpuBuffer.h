#pragma once

#include "EppoCore/GPU/Vulkan.h"

namespace Eppo
{
    class Device;

    enum class BufferUsage : uint32_t
    {
        None = 0,
        Storage = 1 << 0,
        Uniform = 1 << 1,
        Vertex = 1 << 2,
        Index = 1 << 3,
        Indirect = 1 << 4,
    };
    EP_ENUM_FLAGS(BufferUsage)

    enum class MemoryUsage : uint8_t
    {
        GpuOnly, // device local, written through Upload
        CpuToGpu, // host visible and mapped, for data the CPU writes often (uniforms, staging)
        GpuToCpu, // host visible and mapped, for reading results back
    };

    struct GpuBufferSpecification
    {
        uint64_t Size = 0;
        BufferUsage Usage = BufferUsage::Storage;
        MemoryUsage Memory = MemoryUsage::GpuOnly;
        std::string DebugName;
    };

    // A buffer in GPU memory. Every buffer can be a copy source/destination and has a device address for shaders.
    class GpuBuffer
    {
    public:
        GpuBuffer(Device& device, GpuBufferSpecification specification);
        ~GpuBuffer();

        GpuBuffer(const GpuBuffer&) = delete;
        auto operator=(const GpuBuffer&) -> GpuBuffer& = delete;
        GpuBuffer(GpuBuffer&& other) noexcept;
        auto operator=(GpuBuffer&& other) noexcept -> GpuBuffer&;

        // Writes directly when the buffer is mapped, otherwise through a staging buffer; waits until the data is on the GPU
        auto Upload(const void* data, uint64_t size, uint64_t offset = 0) -> void;
        // Reads directly when the buffer is mapped, otherwise through a staging buffer; waits for the copy
        auto Download(void* data, uint64_t size, uint64_t offset = 0) -> void;

        template<typename T, size_t Extent>
        auto Upload(std::span<T, Extent> data, uint64_t offset = 0) -> void
        {
            Upload(data.data(), data.size_bytes(), offset);
        }

        template<typename T, size_t Extent>
        auto Download(std::span<T, Extent> data, uint64_t offset = 0) -> void
        {
            Download(data.data(), data.size_bytes(), offset);
        }

        // Nullptr for GpuOnly buffers
        [[nodiscard]] auto GetMapped() const -> void* { return m_Mapped; }

        [[nodiscard]] auto GetHandle() const -> VkBuffer { return m_Buffer; }
        [[nodiscard]] auto GetSize() const -> uint64_t { return m_Specification.Size; }
        [[nodiscard]] auto GetDeviceAddress() const -> VkDeviceAddress { return m_DeviceAddress; }
        [[nodiscard]] auto GetSpecification() const -> const GpuBufferSpecification& { return m_Specification; }

    private:
        auto Release() -> void;

    private:
        Device* m_Device = nullptr;
        GpuBufferSpecification m_Specification;

        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = VK_NULL_HANDLE;
        void* m_Mapped = nullptr;
        VkDeviceAddress m_DeviceAddress = 0;
    };
}
