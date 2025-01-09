#pragma once


struct GLFWwindow;

namespace Eppo
{
	struct WindowSpecification
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowSpecification(std::string title, const uint32_t width = 800, const uint32_t height = 600)
			: Title(std::move(title)), Width(width), Height(height)
		{}
	};

	class Window
	{
	public:
		explicit Window(WindowSpecification specification);
		~Window();

		static void PollEvents();
		void SwapBuffers() const;

		[[nodiscard]] uint32_t GetWidth() const { return m_Specification.Width; }
		[[nodiscard]] uint32_t GetHeight() const { return m_Specification.Height; }

		[[nodiscard]] GLFWwindow* GetNativeWindow() const { return m_Window; }

	private:
		WindowSpecification m_Specification;

		struct WindowData
		{
			uint32_t Width;
			uint32_t Height;
		} m_Data;

		GLFWwindow* m_Window;
	};
}
