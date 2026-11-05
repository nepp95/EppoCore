#pragma once

#include "EppoCore/GPU/Vulkan.h"
#include "EppoCore/Platform/GUI/Event/Event.h"

struct GLFWwindow;

namespace Eppo
{
    struct WindowSpecification
    {
        std::string Title;
        uint32_t Width;
        uint32_t Height;

        WindowSpecification(std::string title, const uint32_t width = 1280, const uint32_t height = 720)
            : Title(std::move(title)), Width(width), Height(height)
        {
        }
    };

    class Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        explicit Window(WindowSpecification specification);
        ~Window();

        void SetEventCallbackFn(const EventCallbackFn& callback);
        static void PollEvents();

        // Instance extensions Vulkan needs to present to windows (valid once a window exists)
        [[nodiscard]] static auto GetRequiredInstanceExtensions() -> std::vector<const char*>;
        // The caller owns the surface and destroys it with vkDestroySurfaceKHR
        [[nodiscard]] auto CreateSurface(VkInstance instance) const -> VkSurfaceKHR;
        // Size in pixels (can differ from the window size on high-DPI displays); 0x0 while minimized
        [[nodiscard]] auto GetFramebufferSize() const -> std::pair<uint32_t, uint32_t>;

        [[nodiscard]] uint32_t GetWidth() const { return m_Specification.Width; }
        [[nodiscard]] uint32_t GetHeight() const { return m_Specification.Height; }

        [[nodiscard]] GLFWwindow* GetNativeWindow() const { return m_Window; }

    private:
        WindowSpecification m_Specification;
        GLFWwindow* m_Window;

        EventCallbackFn m_EventCallback;
    };
}
