#pragma once

#include "EppoCore/GPU/Vulkan.h"

namespace Eppo
{
    class Device;

    enum class ImageFormat : uint8_t
    {
        Undefined = 0,
        R8,
        RG8,
        RGBA8,
        RGBA8_SRGB,
        BGRA8,
        BGRA8_SRGB,
        R16F,
        RGBA16F,
        R32F,
        RG32F,
        RGBA32F,
        Depth32F,
    };

    enum class ImageUsage : uint32_t
    {
        None = 0,
        Sampled = 1 << 0, // read through samplers; also added to the bindless table
        Storage = 1 << 1, // read/written as a storage image (GENERAL layout)
        ColorAttachment = 1 << 2, // rendered to
        DepthAttachment = 1 << 3,
    };
    EP_ENUM_FLAGS(ImageUsage)

    [[nodiscard]] auto ToVkFormat(ImageFormat format) -> VkFormat;
    [[nodiscard]] auto GetBytesPerPixel(ImageFormat format) -> uint32_t;
    [[nodiscard]] auto IsDepthFormat(ImageFormat format) -> bool;

    struct ImageSpecification
    {
        uint32_t Width = 1;
        uint32_t Height = 1;
        ImageFormat Format = ImageFormat::RGBA8;
        ImageUsage Usage = ImageUsage::Sampled;
        std::string DebugName;
    };

    // A 2D image with one mip level. Every image can be a copy source/destination.
    class Image
    {
    public:
        Image(Device& device, ImageSpecification specification);
        // Wraps an image owned elsewhere (e.g. a swapchain image): never destroyed or added to the bindless table
        Image(Device& device, VkImage image, VkImageView view, ImageSpecification specification);
        ~Image();

        Image(const Image&) = delete;
        auto operator=(const Image&) -> Image& = delete;
        Image(Image&& other) noexcept;
        auto operator=(Image&& other) noexcept -> Image&;

        // Uploads tightly packed pixels for the whole image and waits until they're on the GPU. The image ends up in
        // SHADER_READ_ONLY_OPTIMAL when it's sampled, otherwise in GENERAL.
        auto SetData(const void* data, uint64_t size) -> void;

        [[nodiscard]] auto GetHandle() const -> VkImage { return m_Image; }
        [[nodiscard]] auto GetView() const -> VkImageView { return m_View; }
        [[nodiscard]] auto GetWidth() const -> uint32_t { return m_Specification.Width; }
        [[nodiscard]] auto GetHeight() const -> uint32_t { return m_Specification.Height; }
        [[nodiscard]] auto GetFormat() const -> ImageFormat { return m_Specification.Format; }
        [[nodiscard]] auto GetAspect() const -> VkImageAspectFlags;
        [[nodiscard]] auto GetSpecification() const -> const ImageSpecification& { return m_Specification; }

        // Index in the bindless table; BindlessTable::InvalidIndex when the image isn't Sampled
        [[nodiscard]] auto GetBindlessIndex() const -> uint32_t { return m_BindlessIndex; }

    private:
        auto Release() -> void;

    private:
        Device* m_Device = nullptr;
        ImageSpecification m_Specification;

        VkImage m_Image = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = VK_NULL_HANDLE;
        VkImageView m_View = VK_NULL_HANDLE;
        uint32_t m_BindlessIndex = UINT32_MAX;
        bool m_Owned = true;
    };
}
