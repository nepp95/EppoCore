#pragma once

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
        {}
    };

    class Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        explicit Window(WindowSpecification specification);
        ~Window();

        void SetEventCallbackFn(const EventCallbackFn& callback);
        static void PollEvents();
        void SwapBuffers() const;

        static void SetVSync(bool state);

        [[nodiscard]] uint32_t GetWidth() const { return m_Specification.Width; }
        [[nodiscard]] uint32_t GetHeight() const { return m_Specification.Height; }

        [[nodiscard]] GLFWwindow* GetNativeWindow() const { return m_Window; }

    private:
        WindowSpecification m_Specification;
        GLFWwindow* m_Window;

        EventCallbackFn m_EventCallback;
    };
}
