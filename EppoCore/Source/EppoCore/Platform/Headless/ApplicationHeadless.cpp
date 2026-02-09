#include "pch.h"
#include "ApplicationHeadless.h"

namespace Eppo
{
    Application* Application::s_Instance = nullptr;

    Application::Application(ApplicationSpecification specification)
        : m_Specification(std::move(specification))
    {
        EP_ASSERT(!s_Instance);
        s_Instance = this;
    }

    Application::~Application()
    {
        s_Instance = nullptr;
    }

    auto Application::Close() -> void
    {
        m_Running = false;
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

    auto Application::Run() -> void
    {
        while (m_Running)
        {
            const auto time = m_Timer.GetElapsedMilliseconds();
            const float ts = time - m_LastFrameTime;
            m_LastFrameTime = time;

            // Update layers
            for (const auto& layer : m_LayerStack)
                layer->OnUpdate(ts);
        }
    }
}
