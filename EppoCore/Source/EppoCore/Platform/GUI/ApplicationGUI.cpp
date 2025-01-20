#include "pch.h"
#include "ApplicationGUI.h"

#include <GLFW/glfw3.h>

namespace Eppo
{
	Application* Application::s_Instance = nullptr;

	Application::Application(ApplicationSpecification specification)
		: m_Specification(std::move(specification))
	{
		EPPO_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		// Create window
		WindowSpecification spec(m_Specification.Title, m_Specification.Width, m_Specification.Height);
		m_Window = std::make_unique<Window>(spec);

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

	const ApplicationSpecification& Application::GetSpecification() const
	{
		return m_Specification;
	}

	const Window& Application::GetWindow() const
	{
		return *m_Window;
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
			const auto time = static_cast<float>(glfwGetTime());
			const float ts = time - m_LastFrameTime;
			m_LastFrameTime = time;

			Window::PollEvents();

			// Update layers
			for (const auto& layer : m_LayerStack)
				layer->OnUpdate(ts);

			// Update UI
			m_ImGuiLayer->Begin();

			for (const auto& layer : m_LayerStack)
				layer->OnUIRender();

			m_ImGuiLayer->End();

			m_Window->SwapBuffers();
		}
	}
}
