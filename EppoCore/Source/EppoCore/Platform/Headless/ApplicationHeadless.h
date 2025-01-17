#pragma once

#include "EppoCore/Core/Layer.h"
#include "EppoCore/Core/Timer.h"

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

		const char* operator[](const int index) const
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
		std::vector<std::shared_ptr<Layer> > m_LayerStack;

		bool m_Running = true;
		float m_LastFrameTime = 0.0f;
		Timer m_Timer;

		static Application* s_Instance;

		friend int ::main(int argc, char** argv);
	};

	// To be implemented by the client using this library
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
