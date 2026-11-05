#include "pch.h"
#include "Window.h"

#include "EppoCore/Platform/GUI/Event/ApplicationEvent.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Eppo
{
    static void GLFWErrorCallback(int error, const char* description)
    {
        Log::Error("GLFW Error: ({0}): {1}", error, description);
    }

    Window::Window(WindowSpecification specification)
        : m_Specification(std::move(specification))
    {
        Log::Info("Creating window {0} ({1}x{2})", m_Specification.Title, m_Specification.Width, m_Specification.Height);

        // Initialize GLFW
        const int success = glfwInit();
        EP_ASSERT(success);

        // Forward every error to our own error handling
        glfwSetErrorCallback(GLFWErrorCallback);

        // Create window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_Window = glfwCreateWindow(
            static_cast<int>(m_Specification.Width), static_cast<int>(m_Specification.Height), m_Specification.Title.c_str(), nullptr,
            nullptr
        );
        EP_ASSERT(m_Window);

        // Setup event callbacks
        glfwSetWindowUserPointer(m_Window, &m_EventCallback);

        glfwSetWindowCloseCallback(
            m_Window,
            [](GLFWwindow* window)
            {
                const auto& callback = *static_cast<EventCallbackFn*>(glfwGetWindowUserPointer(window));
                WindowCloseEvent e;
                callback(e);
            }
        );

        glfwSetWindowSizeCallback(
            m_Window,
            [](GLFWwindow* window, int width, int height)
            {
                const auto& callback = *static_cast<EventCallbackFn*>(glfwGetWindowUserPointer(window));
                WindowResizeEvent e(width, height);
                callback(e);
            }
        );
    }

    Window::~Window()
    {
        EP_ASSERT(m_Window);
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    void Window::SetEventCallbackFn(const EventCallbackFn& callback)
    {
        m_EventCallback = callback;
    }

    void Window::PollEvents()
    {
        glfwPollEvents();
    }

    void Window::SwapBuffers() const
    {
    }

    void Window::SetVSync(const bool state)
    {
    }
}
