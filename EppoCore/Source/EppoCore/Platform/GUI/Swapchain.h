#pragma once

#include "EppoCore/GPU/CommandList.h"
#include "EppoCore/GPU/Image.h"
#include "EppoCore/GPU/Queue.h"

namespace Eppo
{
    class Device;
    class Window;

    // The window's swapchain and frame loop: acquire an image, record into the frame's command list, submit and present.
    // Recreates itself when the window is resized or the swapchain goes out of date.
    class Swapchain
    {
    public:
        static constexpr uint32_t FramesInFlight = 2;

        Swapchain(Device& device, const Window& window, bool vsync = true);
        ~Swapchain();

        Swapchain(const Swapchain&) = delete;
        auto operator=(const Swapchain&) -> Swapchain& = delete;
        Swapchain(Swapchain&&) = delete;
        auto operator=(Swapchain&&) -> Swapchain& = delete;

        // Waits for this frame's slot, acquires the next image and starts the frame's command list (with the image as color
        // attachment). False when there's nothing to render to this frame (minimized window, swapchain just recreated).
        [[nodiscard]] auto BeginFrame() -> bool;
        // Submits the frame's command list and presents the image
        auto EndFrame() -> void;

        // Only valid between BeginFrame and EndFrame
        [[nodiscard]] auto GetCommandList() -> CommandList& { return *m_CommandList; }
        [[nodiscard]] auto GetImage() const -> const Image& { return m_Images[m_ImageIndex]; }

        auto SetVSync(bool vsync) -> void;
        [[nodiscard]] auto IsVSync() const -> bool { return m_VSync; }

        [[nodiscard]] auto GetFormat() const -> ImageFormat { return m_Format; }
        [[nodiscard]] auto GetImageCount() const -> uint32_t { return static_cast<uint32_t>(m_Images.size()); }
        [[nodiscard]] auto GetMinImageCount() const -> uint32_t { return m_MinImageCount; }
        [[nodiscard]] auto GetWidth() const -> uint32_t { return m_Width; }
        [[nodiscard]] auto GetHeight() const -> uint32_t { return m_Height; }

    private:
        auto Create() -> void;
        auto DestroyImages() -> void;

    private:
        struct Frame
        {
            VkSemaphore ImageAvailable = VK_NULL_HANDLE;
            SubmitHandle Submitted;
        };

        Device* m_Device;
        const Window* m_Window;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
        ImageFormat m_Format = ImageFormat::BGRA8;
        VkColorSpaceKHR m_ColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_MinImageCount = 2;
        bool m_VSync;
        bool m_NeedsRecreate = false;

        std::vector<Image> m_Images;
        std::vector<VkImageView> m_ImageViews;
        // Per swapchain image, since presentation may still be waiting on one when its frame slot comes around again
        std::vector<VkSemaphore> m_RenderFinished;

        std::array<Frame, FramesInFlight> m_Frames{};
        uint32_t m_FrameIndex = 0;
        uint32_t m_ImageIndex = 0;
        std::optional<CommandList> m_CommandList;
    };
}
