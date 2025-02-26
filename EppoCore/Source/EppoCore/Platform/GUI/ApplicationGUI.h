#pragma once

#include "EppoCore/Core/ApplicationSpecification.h"
#include "EppoCore/Core/Layer.h"
#include "EppoCore/Platform/GUI/Renderer.h"
#include "EppoCore/Platform/GUI/Window.h"
#include "EppoCore/Platform/GUI/Event/ApplicationEvent.h"
#include "EppoCore/Platform/GUI/UI/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Eppo
{
	class Application
	{
	public:
		explicit Application(ApplicationSpecification specification);
		~Application();

		void Close();
		void OnEvent(Event& e);

		[[nodiscard]] const ApplicationSpecification& GetSpecification() const;
		[[nodiscard]] std::shared_ptr<Renderer> GetRenderer() const;
		[[nodiscard]] const Window& GetWindow() const;

		static Application& Get();

	protected:
		void PushLayer(const std::shared_ptr<Layer>& layer);

		bool OnWindowClose(const WindowCloseEvent& e);
		bool OnWindowResize(const WindowResizeEvent& e);

	private:
		void Run();

	private:
		ApplicationSpecification m_Specification;
		std::unique_ptr<Window> m_Window;
		std::shared_ptr<ImGuiLayer> m_ImGuiLayer;
		std::shared_ptr<Renderer> m_Renderer;
		std::vector<std::shared_ptr<Layer>> m_LayerStack;

		bool m_Running = true;
		bool m_IsMinimized = false;
		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// To be implemented by the client using this library
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
