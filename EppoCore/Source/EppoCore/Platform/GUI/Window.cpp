#include "pch.h"
#include "Window.h"

#include "EppoCore/Platform/GUI/Event/ApplicationEvent.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Eppo
{
    static void GLFWErrorCallback(int error, const char* description)
    {
        Log::Error("GLFW Error: ({0}): {1}", error, description);
    }

    static void OpenGLMessageCallback(
        unsigned source, unsigned type, unsigned id, unsigned severity, int length, const char* message, const void* userParam
    )
    {
        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:
            case GL_DEBUG_SEVERITY_MEDIUM:
            {
                Log::Error("{}", message);
                break;
            }

            case GL_DEBUG_SEVERITY_LOW:
            {
                Log::Warn("{}", message);
                break;
            }

            case GL_DEBUG_SEVERITY_NOTIFICATION:
            {
                Log::Info("{}", message);
                break;
            }
        }
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
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_Window = glfwCreateWindow(
            static_cast<int>(m_Specification.Width), static_cast<int>(m_Specification.Height), m_Specification.Title.c_str(), nullptr,
            nullptr
        );
        EP_ASSERT(m_Window);

        // Initialize OpenGL
        glfwMakeContextCurrent(m_Window);
        const int status = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
        EP_ASSERT(status);
        EP_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5));

// Enable debug output
#ifdef EP_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(OpenGLMessageCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, false);
#endif

        // Setup OpenGL states
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);

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
        EP_ASSERT(m_Window);
        glfwSwapBuffers(m_Window);
    }

    void Window::SetVSync(const bool state)
    {
        glfwSwapInterval(state ? 1 : 0);
    }
}
