#include "pch.h"
#include "Window.h"

#include "EppoCore/Platform/GUI/Event/ApplicationEvent.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Eppo
{
    namespace
    {
        void GLFWErrorCallback(int error, const char* description)
        {
            EPPO_ERROR("GLFW Error: ({0}): {1}", error, description);
        }
    }

    Window::Window(WindowSpecification specification) : m_Specification(std::move(specification))
    {
        EPPO_INFO("Creating window {0} ({1}x{2})", m_Specification.Title, m_Specification.Width, m_Specification.Height);

        // Initialize GLFW
        const int success = glfwInit();
        EPPO_ASSERT(success, "Failed to initialize GLFW!")

        // Forward every error to our own error handling
        glfwSetErrorCallback(GLFWErrorCallback);

        // Create window
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_Window = glfwCreateWindow(static_cast<int>(m_Specification.Width), static_cast<int>(m_Specification.Height),
                                    m_Specification.Title.c_str(), nullptr, nullptr);
        EPPO_ASSERT(m_Window)

        // Initialize OpenGL
        glfwMakeContextCurrent(m_Window);
        const int status = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
        EPPO_ASSERT(status)
        EPPO_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5),
                    "Application requires at least OpenGL version 4.5!")

        // Setup event callbacks
        glfwSetWindowUserPointer(m_Window, &m_EventCallback);

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
        {
            const auto& callback = *static_cast<EventCallbackFn*>(glfwGetWindowUserPointer(window));
            WindowCloseEvent e;
            callback(e);
        });

        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
        {
            const auto& callback = *static_cast<EventCallbackFn*>(glfwGetWindowUserPointer(window));
            WindowResizeEvent e(width, height);
            callback(e);
        });
    }

    Window::~Window()
    {
        EPPO_ASSERT(m_Window)
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
        EPPO_ASSERT(m_Window)
        glfwSwapBuffers(m_Window);
    }
}
