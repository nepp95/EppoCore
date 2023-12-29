#pragma once

#include "Core/Layer.h"
#include "Core/Window.h"
#include "Event/ApplicationEvent.h"
#include "Event/Event.h"

int main(int argc, char** argv);

namespace Eppo
{
	struct ApplicationSpecification
	{
		std::string Title;
		uint32_t Width = 800;
		uint32_t Height = 600;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void OnEvent(Event& e);

		bool IsMinimized() const { return m_Minimized; }

		const ApplicationSpecification& GetSpecification() const { return m_Specification; }
		const Window& GetWindow() const { return *m_Window; }

		static Application& Get() { return *s_Instance; }

	protected:
		void PushLayer(const std::shared_ptr<Layer>& layer);

	private:
		void Run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		ApplicationSpecification m_Specification;
		std::unique_ptr<Window> m_Window;
		std::vector<std::shared_ptr<Layer>> m_LayerStack;

		bool m_Running = true;
		bool m_Minimized = false;
		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// To be implemented by the client using this library
	Application* CreateApplication();
}
