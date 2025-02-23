#pragma once

#include "EppoCore/Core/ApplicationSpecification.h"
#include "EppoCore/Core/Layer.h"
#include "EppoCore/Core/Timer.h"

int main(int argc, char** argv);

namespace Eppo
{
	class Application
	{
	public:
		explicit Application(ApplicationSpecification specification);

		~Application();

		void Close();

		[[nodiscard]] const ApplicationSpecification& GetSpecification() const;

		static Application& Get();

	protected:
		void PushLayer(const std::shared_ptr<Layer>& layer);

	private:
		void Run();

	private:
		ApplicationSpecification m_Specification;
		std::vector<std::shared_ptr<Layer>> m_LayerStack;

		bool m_Running = true;
		float m_LastFrameTime = 0.0f;
		Timer m_Timer;

		static Application* s_Instance;

		friend int ::main(int argc, char** argv);
	};

	// To be implemented by the client using this library
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
