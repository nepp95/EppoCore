#include "pch.h"
#include "GpuBuffer.h"

#include "EppoCore/GPU/CommandList.h"
#include "EppoCore/GPU/Device.h"

#include <vma/vk_mem_alloc.h>

namespace Eppo
{
    namespace
    {
        auto ToVkUsage(const BufferUsage usage) -> VkBufferUsageFlags
        {
            VkBufferUsageFlags flags =
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

            if (HasFlag(usage, BufferUsage::Storage))
                flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            if (HasFlag(usage, BufferUsage::Uniform))
                flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            if (HasFlag(usage, BufferUsage::Vertex))
                flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            if (HasFlag(usage, BufferUsage::Index))
                flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            if (HasFlag(usage, BufferUsage::Indirect))
                flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

            return flags;
        }

        auto ToVmaFlags(const MemoryUsage memory) -> VmaAllocationCreateFlags
        {
            switch (memory)
            {
                case MemoryUsage::GpuOnly:
                    return 0;
                case MemoryUsage::CpuToGpu:
                    return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
                case MemoryUsage::GpuToCpu:
                    return VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            }

            return 0;
        }
    }

    GpuBuffer::GpuBuffer(Device& device, GpuBufferSpecification specification)
        : m_Device(&device), m_Specification(std::move(specification))
    {
        ZoneScoped;

        EP_ASSERT(m_Specification.Size > 0);

        const auto& families = m_Device->GetQueueFamilies();

        const VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = m_Specification.Size,
            .usage = ToVkUsage(m_Specification.Usage),
            // Shared between the queue families, so no ownership transfers are needed
            .sharingMode = families.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = static_cast<uint32_t>(families.size()),
            .pQueueFamilyIndices = families.data(),
        };

        const VmaAllocationCreateInfo allocationInfo{
            .flags = ToVmaFlags(m_Specification.Memory),
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        VmaAllocationInfo allocationResult{};
        EP_VK_CHECK(vmaCreateBuffer(m_Device->GetAllocator(), &bufferInfo, &allocationInfo, &m_Buffer, &m_Allocation, &allocationResult));
        m_Mapped = allocationResult.pMappedData;

        const VkBufferDeviceAddressInfo addressInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = m_Buffer,
        };
        m_DeviceAddress = vkGetBufferDeviceAddress(m_Device->GetHandle(), &addressInfo);

        m_Device->SetDebugName(VK_OBJECT_TYPE_BUFFER, VkHandleToU64(m_Buffer), m_Specification.DebugName);
    }

    GpuBuffer::~GpuBuffer()
    {
        Release();
    }

    GpuBuffer::GpuBuffer(GpuBuffer&& other) noexcept
        : m_Device(other.m_Device),
          m_Specification(std::move(other.m_Specification)),
          m_Buffer(std::exchange(other.m_Buffer, VK_NULL_HANDLE)),
          m_Allocation(std::exchange(other.m_Allocation, VK_NULL_HANDLE)),
          m_Mapped(std::exchange(other.m_Mapped, nullptr)),
          m_DeviceAddress(std::exchange(other.m_DeviceAddress, 0))
    {
    }

    auto GpuBuffer::operator=(GpuBuffer&& other) noexcept -> GpuBuffer&
    {
        if (this != &other)
        {
            Release();
            m_Device = other.m_Device;
            m_Specification = std::move(other.m_Specification);
            m_Buffer = std::exchange(other.m_Buffer, VK_NULL_HANDLE);
            m_Allocation = std::exchange(other.m_Allocation, VK_NULL_HANDLE);
            m_Mapped = std::exchange(other.m_Mapped, nullptr);
            m_DeviceAddress = std::exchange(other.m_DeviceAddress, 0);
        }

        return *this;
    }

    auto GpuBuffer::Upload(const void* data, const uint64_t size, const uint64_t offset) -> void
    {
        ZoneScoped;

        EP_ASSERT(offset + size <= m_Specification.Size);

        if (m_Mapped)
        {
            std::memcpy(static_cast<uint8_t*>(m_Mapped) + offset, data, size);
            EP_VK_CHECK(vmaFlushAllocation(m_Device->GetAllocator(), m_Allocation, offset, size));
            return;
        }

        GpuBuffer staging(
            *m_Device, { .Size = size, .Usage = BufferUsage::None, .Memory = MemoryUsage::CpuToGpu, .DebugName = "Upload staging" }
        );
        staging.Upload(data, size);

        CommandList commandList(*m_Device, QueueType::Transfer);
        commandList.CopyBuffer(staging, *this, size, 0, offset);
        m_Device->Submit(commandList).Wait();
    }

    auto GpuBuffer::Download(void* data, const uint64_t size, const uint64_t offset) -> void
    {
        ZoneScoped;

        EP_ASSERT(offset + size <= m_Specification.Size);

        if (m_Mapped)
        {
            EP_VK_CHECK(vmaInvalidateAllocation(m_Device->GetAllocator(), m_Allocation, offset, size));
            std::memcpy(data, static_cast<const uint8_t*>(m_Mapped) + offset, size);
            return;
        }

        GpuBuffer staging(
            *m_Device, { .Size = size, .Usage = BufferUsage::None, .Memory = MemoryUsage::GpuToCpu, .DebugName = "Download staging" }
        );

        CommandList commandList(*m_Device, QueueType::Transfer);
        commandList.CopyBuffer(*this, staging, size, offset, 0);
        m_Device->Submit(commandList).Wait();

        staging.Download(data, size);
    }

    auto GpuBuffer::Release() -> void
    {
        if (!m_Buffer)
            return;

        m_Device->Defer(
            [allocator = m_Device->GetAllocator(), buffer = m_Buffer, allocation = m_Allocation]
            {
                vmaDestroyBuffer(allocator, buffer, allocation);
            }
        );

        m_Buffer = VK_NULL_HANDLE;
        m_Allocation = VK_NULL_HANDLE;
        m_Mapped = nullptr;
        m_DeviceAddress = 0;
    }
}
