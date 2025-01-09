#include "pch.h"
#include "ApplicationHeadless.h"

namespace Eppo
{
	Application* Application::s_Instance = nullptr;

	Application::Application(ApplicationSpecification specification)
		: m_Specification(std::move(specification))
	{
		EPPO_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
	}

	Application::~Application()
	{
		s_Instance = nullptr;
	}

	void Application::Close()
	{
		m_Running = false;
	}

	const ApplicationSpecification& Application::GetSpecification() const
	{
		return m_Specification;
	}

	Application& Application::Get()
	{
		return *s_Instance;
	}

	void Application::PushLayer(const std::shared_ptr<Layer>& layer)
	{
		m_LayerStack.emplace_back(layer);
		layer->OnAttach();
	}

	void Application::Run()
	{
		while (m_Running)
		{
			const float time = static_cast<float>(m_Timer.GetElapsedMilliseconds());
			const float ts = time - m_LastFrameTime;
			m_LastFrameTime = time;

			// Update layers
			for (const auto& layer : m_LayerStack)
				layer->OnUpdate(ts);
		}
	}
}
