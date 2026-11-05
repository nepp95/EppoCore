#include "pch.h"
#include "Image.h"

#include "EppoCore/GPU/BindlessTable.h"
#include "EppoCore/GPU/CommandList.h"
#include "EppoCore/GPU/Device.h"
#include "EppoCore/GPU/GpuBuffer.h"

#include <vma/vk_mem_alloc.h>

namespace Eppo
{
    auto ToVkFormat(const ImageFormat format) -> VkFormat
    {
        switch (format)
        {
            case ImageFormat::Undefined:
                return VK_FORMAT_UNDEFINED;
            case ImageFormat::R8:
                return VK_FORMAT_R8_UNORM;
            case ImageFormat::RG8:
                return VK_FORMAT_R8G8_UNORM;
            case ImageFormat::RGBA8:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case ImageFormat::RGBA8_SRGB:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case ImageFormat::BGRA8:
                return VK_FORMAT_B8G8R8A8_UNORM;
            case ImageFormat::BGRA8_SRGB:
                return VK_FORMAT_B8G8R8A8_SRGB;
            case ImageFormat::R16F:
                return VK_FORMAT_R16_SFLOAT;
            case ImageFormat::RGBA16F:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
            case ImageFormat::R32F:
                return VK_FORMAT_R32_SFLOAT;
            case ImageFormat::RG32F:
                return VK_FORMAT_R32G32_SFLOAT;
            case ImageFormat::RGBA32F:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case ImageFormat::Depth32F:
                return VK_FORMAT_D32_SFLOAT;
        }

        return VK_FORMAT_UNDEFINED;
    }

    auto GetBytesPerPixel(const ImageFormat format) -> uint32_t
    {
        switch (format)
        {
            case ImageFormat::Undefined:
                return 0;
            case ImageFormat::R8:
                return 1;
            case ImageFormat::RG8:
            case ImageFormat::R16F:
                return 2;
            case ImageFormat::RGBA8:
            case ImageFormat::RGBA8_SRGB:
            case ImageFormat::BGRA8:
            case ImageFormat::BGRA8_SRGB:
            case ImageFormat::R32F:
            case ImageFormat::Depth32F:
                return 4;
            case ImageFormat::RGBA16F:
            case ImageFormat::RG32F:
                return 8;
            case ImageFormat::RGBA32F:
                return 16;
        }

        return 0;
    }

    auto IsDepthFormat(const ImageFormat format) -> bool
    {
        return format == ImageFormat::Depth32F;
    }

    Image::Image(Device& device, ImageSpecification specification)
        : m_Device(&device), m_Specification(std::move(specification))
    {
        ZoneScoped;

        EP_ASSERT(m_Specification.Width > 0 && m_Specification.Height > 0);
        EP_ASSERT(m_Specification.Format != ImageFormat::Undefined);

        VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (HasFlag(m_Specification.Usage, ImageUsage::Sampled))
            usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (HasFlag(m_Specification.Usage, ImageUsage::Storage))
            usage |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (HasFlag(m_Specification.Usage, ImageUsage::ColorAttachment))
            usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (HasFlag(m_Specification.Usage, ImageUsage::DepthAttachment))
            usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        const auto& families = m_Device->GetQueueFamilies();
        const VkFormat format = ToVkFormat(m_Specification.Format);

        const VkImageCreateInfo imageInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = { m_Specification.Width, m_Specification.Height, 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = usage,
            // Shared between the queue families, so no ownership transfers are needed
            .sharingMode = families.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = static_cast<uint32_t>(families.size()),
            .pQueueFamilyIndices = families.data(),
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        constexpr VmaAllocationCreateInfo allocationInfo{
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        EP_VK_CHECK(vmaCreateImage(m_Device->GetAllocator(), &imageInfo, &allocationInfo, &m_Image, &m_Allocation, nullptr));

        const VkImageViewCreateInfo viewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_Image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .subresourceRange = { GetAspect(), 0, 1, 0, 1 },
        };

        EP_VK_CHECK(vkCreateImageView(m_Device->GetHandle(), &viewInfo, nullptr, &m_View));

        m_Device->SetDebugName(VK_OBJECT_TYPE_IMAGE, VkHandleToU64(m_Image), m_Specification.DebugName);
        m_Device->SetDebugName(VK_OBJECT_TYPE_IMAGE_VIEW, VkHandleToU64(m_View), m_Specification.DebugName);

        if (HasFlag(m_Specification.Usage, ImageUsage::Sampled))
            m_BindlessIndex = m_Device->AddBindlessImage(m_View);
    }

    Image::Image(Device& device, const VkImage image, const VkImageView view, ImageSpecification specification)
        : m_Device(&device), m_Specification(std::move(specification)), m_Image(image), m_View(view), m_Owned(false)
    {
    }

    Image::~Image()
    {
        Release();
    }

    Image::Image(Image&& other) noexcept
        : m_Device(other.m_Device),
          m_Specification(std::move(other.m_Specification)),
          m_Image(std::exchange(other.m_Image, VK_NULL_HANDLE)),
          m_Allocation(std::exchange(other.m_Allocation, VK_NULL_HANDLE)),
          m_View(std::exchange(other.m_View, VK_NULL_HANDLE)),
          m_BindlessIndex(std::exchange(other.m_BindlessIndex, BindlessTable::InvalidIndex)),
          m_Owned(other.m_Owned)
    {
    }

    auto Image::operator=(Image&& other) noexcept -> Image&
    {
        if (this != &other)
        {
            Release();
            m_Device = other.m_Device;
            m_Specification = std::move(other.m_Specification);
            m_Image = std::exchange(other.m_Image, VK_NULL_HANDLE);
            m_Allocation = std::exchange(other.m_Allocation, VK_NULL_HANDLE);
            m_View = std::exchange(other.m_View, VK_NULL_HANDLE);
            m_BindlessIndex = std::exchange(other.m_BindlessIndex, BindlessTable::InvalidIndex);
            m_Owned = other.m_Owned;
        }

        return *this;
    }

    auto Image::SetData(const void* data, const uint64_t size) -> void
    {
        ZoneScoped;

        EP_ASSERT(size == static_cast<uint64_t>(m_Specification.Width) * m_Specification.Height * GetBytesPerPixel(m_Specification.Format));

        GpuBuffer staging(
            *m_Device, { .Size = size, .Usage = BufferUsage::None, .Memory = MemoryUsage::CpuToGpu, .DebugName = "Image staging" }
        );
        staging.Upload(data, size);

        const Access finalAccess = HasFlag(m_Specification.Usage, ImageUsage::Sampled) ? Access::ShaderRead : Access::ComputeRead;

        // Graphics queue, since the final layout transition may target fragment shader reads
        CommandList commandList(*m_Device, QueueType::Graphics);
        commandList.Barrier(*this, Access::None, Access::TransferWrite);
        commandList.CopyBufferToImage(staging, *this);
        commandList.Barrier(*this, Access::TransferWrite, finalAccess);
        m_Device->Submit(commandList).Wait();
    }

    auto Image::GetAspect() const -> VkImageAspectFlags
    {
        return IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    }

    auto Image::Release() -> void
    {
        if (!m_Image)
            return;

        if (!m_Owned)
        {
            m_Image = VK_NULL_HANDLE;
            m_View = VK_NULL_HANDLE;
            return;
        }

        m_Device->RemoveBindlessImage(m_BindlessIndex);
        m_Device->Defer(
            [device = m_Device->GetHandle(), allocator = m_Device->GetAllocator(), image = m_Image, allocation = m_Allocation,
             view = m_View]
            {
                vkDestroyImageView(device, view, nullptr);
                vmaDestroyImage(allocator, image, allocation);
            }
        );

        m_Image = VK_NULL_HANDLE;
        m_Allocation = VK_NULL_HANDLE;
        m_View = VK_NULL_HANDLE;
        m_BindlessIndex = BindlessTable::InvalidIndex;
    }
}
