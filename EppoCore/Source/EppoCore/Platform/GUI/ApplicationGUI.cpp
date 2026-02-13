#include "pch.h"
#include "ApplicationGUI.h"

#include "Renderer.h"

#include <GLFW/glfw3.h>

namespace Eppo
{
    Application* Application::s_Instance = nullptr;

    Application::Application(ApplicationSpecification specification)
        : m_Specification(std::move(specification))
    {
        EP_ASSERT(!s_Instance);
        s_Instance = this;

        // Create window
        WindowSpecification spec(m_Specification.Title, m_Specification.Width, m_Specification.Height);
        m_Window = std::make_unique<Window>(spec);
        m_Window->SetEventCallbackFn(
            [this](Event& e)
            {
                OnEvent(e);
            }
        );

        // Initialize renderer
        m_Renderer = std::make_shared<Renderer>();

        // Create UI layer
        m_ImGuiLayer = std::make_shared<ImGuiLayer>();
        PushLayer(m_ImGuiLayer);
    }

    Application::~Application()
    {
        s_Instance = nullptr;
    }

    void Application::Close()
    {
        m_Running = false;
    }

    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

        for (const auto& layer : m_LayerStack)
        {
            if (e.Handled)
                break;

            layer->OnEvent(e);
        }
    }

    auto Application::GetRenderer() const -> std::shared_ptr<Renderer>
    {
        return m_Renderer;
    }

    auto Application::GetWindow() const -> const Window&
    {
        return *m_Window;
    }

    auto Application::Get() -> Application&
    {
        return *s_Instance;
    }

    auto Application::PushLayer(const std::shared_ptr<Layer>& layer) -> void
    {
        m_LayerStack.emplace_back(layer);
        layer->OnAttach();
    }

    auto Application::OnWindowClose(const WindowCloseEvent& e) -> bool
    {
        Close();

        return true;
    }

    auto Application::OnWindowResize(const WindowResizeEvent& e) -> bool
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            // Minimized application
            m_IsMinimized = true;
            return false;
        }

        m_IsMinimized = false;

        return true;
    }

    auto Application::Run() -> void
    {
        while (m_Running)
        {
            const auto time = static_cast<float>(glfwGetTime());
            const float ts = time - m_LastFrameTime;
            m_LastFrameTime = time;

            Window::PollEvents();

            // Update layers
            for (const auto& layer : m_LayerStack)
                layer->OnUpdate(ts);

            // Update UI
            for (const auto& layer : m_LayerStack)
                layer->PreUIRender();

            m_ImGuiLayer->Begin();

            for (const auto& layer : m_LayerStack)
                layer->OnUIRender();

            m_ImGuiLayer->End();

            for (const auto& layer : m_LayerStack)
                layer->PostUIRender();

            m_Window->SwapBuffers();
        }
    }
}
