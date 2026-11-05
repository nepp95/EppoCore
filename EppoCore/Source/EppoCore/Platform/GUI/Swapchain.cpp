#include "pch.h"
#include "Swapchain.h"

#include "EppoCore/GPU/Device.h"
#include "EppoCore/GPU/Instance.h"
#include "EppoCore/Platform/GUI/Window.h"

namespace Eppo
{
    namespace
    {
        auto MakeSemaphore(const Device& device, const std::string_view name) -> VkSemaphore
        {
            constexpr VkSemaphoreCreateInfo semaphoreInfo{
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            VkSemaphore semaphore = VK_NULL_HANDLE;
            EP_VK_CHECK(vkCreateSemaphore(device.GetHandle(), &semaphoreInfo, nullptr, &semaphore));
            device.SetDebugName(VK_OBJECT_TYPE_SEMAPHORE, VkHandleToU64(semaphore), name);
            return semaphore;
        }
    }

    Swapchain::Swapchain(Device& device, const Window& window, const bool vsync)
        : m_Device(&device), m_Window(&window), m_VSync(vsync)
    {
        ZoneScoped;

        const VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice().GetHandle();
        m_Surface = window.CreateSurface(m_Device->GetInstance().GetHandle());

        // The graphics queue presents
        VkBool32 presentSupported = VK_FALSE;
        EP_VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(
            physicalDevice, m_Device->GetQueue(QueueType::Graphics).GetFamily(), m_Surface, &presentSupported
        ));
        if (!presentSupported)
            VkFatal("The GPU's graphics queue can't present to this window");

        // UNORM, since ImGui (and the 2D renderer) output colors that are already sRGB encoded
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, formats.data());

        bool found = false;
        for (const auto [format, imageFormat] : {
                 std::pair{ VK_FORMAT_B8G8R8A8_UNORM, ImageFormat::BGRA8 },
                 std::pair{ VK_FORMAT_R8G8B8A8_UNORM, ImageFormat::RGBA8 }
        })
        {
            const auto it = std::ranges::find_if(
                formats,
                [format](const VkSurfaceFormatKHR& surfaceFormat)
                {
                    return surfaceFormat.format == format && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                }
            );

            if (it != formats.end())
            {
                m_Format = imageFormat;
                m_ColorSpace = it->colorSpace;
                found = true;
                break;
            }
        }

        if (!found)
            VkFatal("The window supports neither BGRA8 nor RGBA8 (UNORM, sRGB non-linear) swapchain images");

        for (uint32_t i = 0; i < FramesInFlight; i++)
            m_Frames[i].ImageAvailable = MakeSemaphore(*m_Device, "Swapchain image available " + std::to_string(i));

        Create();
    }

    Swapchain::~Swapchain()
    {
        m_Device->WaitIdle();
        m_CommandList.reset();

        DestroyImages();
        vkDestroySwapchainKHR(m_Device->GetHandle(), m_Swapchain, nullptr);

        for (const Frame& frame : m_Frames)
            vkDestroySemaphore(m_Device->GetHandle(), frame.ImageAvailable, nullptr);
        for (const VkSemaphore semaphore : m_RenderFinished)
            vkDestroySemaphore(m_Device->GetHandle(), semaphore, nullptr);

        vkDestroySurfaceKHR(m_Device->GetInstance().GetHandle(), m_Surface, nullptr);
    }

    auto Swapchain::BeginFrame() -> bool
    {
        ZoneScoped;

        EP_ASSERT(!m_CommandList);

        const auto [width, height] = m_Window->GetFramebufferSize();
        if (width == 0 || height == 0)
            return false;

        if (m_NeedsRecreate || width != m_Width || height != m_Height)
        {
            Create();
            if (m_NeedsRecreate)
                return false;
        }

        Frame& frame = m_Frames[m_FrameIndex];
        frame.Submitted.Wait();

        const VkResult result =
            vkAcquireNextImageKHR(m_Device->GetHandle(), m_Swapchain, UINT64_MAX, frame.ImageAvailable, VK_NULL_HANDLE, &m_ImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            // Nothing was acquired (and the semaphore isn't signaled); try again next frame
            m_NeedsRecreate = true;
            return false;
        }

        if (result == VK_SUBOPTIMAL_KHR)
            m_NeedsRecreate = true; // the image was acquired, so this frame still renders and presents
        else
            EP_VK_CHECK(result);

        m_CommandList.emplace(*m_Device, QueueType::Graphics);

        // Undefined -> color attachment, ordered after the acquire semaphore wait (which happens at color attachment output)
        const VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_2_NONE,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = GetImage().GetHandle(),
            .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
        };

        const VkDependencyInfo dependency{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        };

        vkCmdPipelineBarrier2(m_CommandList->GetHandle(), &dependency);
        return true;
    }

    auto Swapchain::EndFrame() -> void
    {
        ZoneScoped;

        EP_ASSERT(m_CommandList);

        m_CommandList->Barrier(GetImage(), Access::ColorAttachment, Access::Present);

        Frame& frame = m_Frames[m_FrameIndex];
        const SemaphoreSubmit wait{ frame.ImageAvailable, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT };
        const SemaphoreSubmit signal{ m_RenderFinished[m_ImageIndex], VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT };
        frame.Submitted = m_Device->Submit(*m_CommandList, { &wait, 1 }, { &signal, 1 });
        m_CommandList.reset();

        const VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_RenderFinished[m_ImageIndex],
            .swapchainCount = 1,
            .pSwapchains = &m_Swapchain,
            .pImageIndices = &m_ImageIndex,
        };

        const VkResult result = vkQueuePresentKHR(m_Device->GetQueue(QueueType::Graphics).GetHandle(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            m_NeedsRecreate = true;
        else
            EP_VK_CHECK(result);

        m_FrameIndex = (m_FrameIndex + 1) % FramesInFlight;
    }

    auto Swapchain::SetVSync(const bool vsync) -> void
    {
        if (m_VSync == vsync)
            return;

        m_VSync = vsync;
        m_NeedsRecreate = true;
    }

    auto Swapchain::Create() -> void
    {
        ZoneScoped;

        // The old swapchain's images may still be in use
        m_Device->WaitIdle();

        const VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice().GetHandle();

        VkSurfaceCapabilitiesKHR capabilities{};
        EP_VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &capabilities));

        VkExtent2D extent = capabilities.currentExtent;
        if (extent.width == UINT32_MAX)
        {
            const auto [width, height] = m_Window->GetFramebufferSize();
            extent.width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        if (extent.width == 0 || extent.height == 0)
        {
            m_NeedsRecreate = true; // minimized; try again once the window has a size
            return;
        }

        // FIFO is vsync and always available; without vsync prefer MAILBOX (no tearing), then IMMEDIATE
        uint32_t modeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &modeCount, nullptr);
        std::vector<VkPresentModeKHR> modes(modeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &modeCount, modes.data());

        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        if (!m_VSync)
        {
            for (const VkPresentModeKHR preferred : { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR })
            {
                if (std::ranges::find(modes, preferred) != modes.end())
                {
                    presentMode = preferred;
                    break;
                }
            }
        }

        m_MinImageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0)
            m_MinImageCount = std::min(m_MinImageCount, capabilities.maxImageCount);

        // Opaque when supported, otherwise the first mode the surface supports
        VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        for (uint32_t bit = 1; !(capabilities.supportedCompositeAlpha & compositeAlpha) && bit <= VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
             bit <<= 1)
            compositeAlpha = static_cast<VkCompositeAlphaFlagBitsKHR>(bit);

        const VkSwapchainKHR oldSwapchain = m_Swapchain;

        const VkSwapchainCreateInfoKHR swapchainInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_Surface,
            .minImageCount = m_MinImageCount,
            .imageFormat = ToVkFormat(m_Format),
            .imageColorSpace = m_ColorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = compositeAlpha,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = oldSwapchain,
        };

        EP_VK_CHECK(vkCreateSwapchainKHR(m_Device->GetHandle(), &swapchainInfo, nullptr, &m_Swapchain));

        DestroyImages();
        vkDestroySwapchainKHR(m_Device->GetHandle(), oldSwapchain, nullptr);

        m_Width = extent.width;
        m_Height = extent.height;

        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(m_Device->GetHandle(), m_Swapchain, &imageCount, nullptr);
        std::vector<VkImage> images(imageCount);
        vkGetSwapchainImagesKHR(m_Device->GetHandle(), m_Swapchain, &imageCount, images.data());

        m_Images.reserve(imageCount);
        for (uint32_t i = 0; i < imageCount; i++)
        {
            const VkImageViewCreateInfo viewInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = ToVkFormat(m_Format),
                .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
            };

            VkImageView view = VK_NULL_HANDLE;
            EP_VK_CHECK(vkCreateImageView(m_Device->GetHandle(), &viewInfo, nullptr, &view));
            m_ImageViews.push_back(view);

            const std::string name = "Swapchain image " + std::to_string(i);
            m_Device->SetDebugName(VK_OBJECT_TYPE_IMAGE_VIEW, VkHandleToU64(view), name);
            m_Images.emplace_back(
                *m_Device, images[i], view,
                ImageSpecification{
                    .Width = m_Width, .Height = m_Height, .Format = m_Format, .Usage = ImageUsage::ColorAttachment, .DebugName = name }
            );
        }

        // Semaphores are kept across recreation (presentation may still reference them), only new ones are added
        while (m_RenderFinished.size() < imageCount)
            m_RenderFinished.push_back(MakeSemaphore(*m_Device, "Swapchain render finished " + std::to_string(m_RenderFinished.size())));

        m_NeedsRecreate = false;
        Log::Info("Swapchain {}x{}, {} images, {}", m_Width, m_Height, imageCount, m_VSync ? "vsync" : "no vsync");
    }

    auto Swapchain::DestroyImages() -> void
    {
        m_Images.clear();

        for (const VkImageView view : m_ImageViews)
            vkDestroyImageView(m_Device->GetHandle(), view, nullptr);
        m_ImageViews.clear();
    }
}
