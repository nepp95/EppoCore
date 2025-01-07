#pragma once

#include "EppoCore/Core/Layer.h"
#include "EppoCore/Core/Window.h"
#include "EppoCore/Event/ApplicationEvent.h"
#include "EppoCore/Event/Event.h"
#include "EppoCore/UI/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Eppo
{
	struct ApplicationCommandLineArgs
	{
		ApplicationCommandLineArgs() = default;
		ApplicationCommandLineArgs(int argc, char** argv)
			: Count(argc), Args(argv)
		{}

		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			EPPO_ASSERT((index < Count));
			if (index >= Count)
				return "";
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Title;
		uint32_t Width = 1280;
		uint32_t Height = 720;

		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
	public:
		Application(ApplicationSpecification specification);
		~Application();

		void Close();
		void OnEvent(Event& e);

		bool IsMinimized() const { return m_Minimized; }

		const ApplicationSpecification& GetSpecification() const;
		const Window& GetWindow() const;

		static Application& Get();

	protected:
		void PushLayer(const std::shared_ptr<Layer>& layer);

	private:
		void Run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		ApplicationSpecification m_Specification;
		std::unique_ptr<Window> m_Window;
		std::shared_ptr<ImGuiLayer> m_ImGuiLayer;
		std::vector<std::shared_ptr<Layer>> m_LayerStack;

		bool m_Running = true;
		bool m_Minimized = false;
		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// To be implemented by the client using this library
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
