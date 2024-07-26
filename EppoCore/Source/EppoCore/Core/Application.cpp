#include "pch.h"
#include "Application.h"

#include <GLFW/glfw3.h>

namespace Eppo
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		EPPO_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		#ifdef EPPO_HEADLESS
		// Create window
		WindowSpecification spec(m_Specification.Title, m_Specification.Width, m_Specification.Height);
		m_Window = std::make_unique<Window>(spec);
		m_Window->SetEventCallbackFn(BIND_EVENT_FN(Application::OnEvent));

		// Create UI layer
		m_ImGuiLayer = std::make_shared<ImGuiLayer>();
		PushLayer(m_ImGuiLayer);
		#endif
	}

	Application::~Application()
	{

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

	void Application::PushLayer(const std::shared_ptr<Layer>& layer)
	{
		m_LayerStack.emplace_back(layer);
		layer->OnAttach();
	}

	void Application::Run()
	{
		while (m_Running)
		{
			float time = static_cast<float>(glfwGetTime());
			float ts = time - m_LastFrameTime;
			m_LastFrameTime = time;

			m_Window->PollEvents();

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

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		Close();

		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;

		return false;
	}
}
