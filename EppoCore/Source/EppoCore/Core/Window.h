#pragma once

#include "EppoCore/Event/Event.h"

struct GLFWwindow;

namespace Eppo
{
	struct WindowSpecification
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowSpecification(const std::string& title, uint32_t width = 800, uint32_t height = 600)
			: Title(title), Width(width), Height(height)
		{}
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		Window(const WindowSpecification& specification);
		~Window();

		void PollEvents();
		void SwapBuffers();

		void SetEventCallbackFn(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }

		uint32_t GetWidth() const { return m_Specification.Width; }
		uint32_t GetHeight() const { return m_Specification.Height; }

		GLFWwindow* GetNativeWindow() const { return m_Window; }

	private:
		WindowSpecification m_Specification;

		struct WindowData
		{
			uint32_t Width;
			uint32_t Height;

			EventCallbackFn EventCallback;
		} m_Data;

		GLFWwindow* m_Window;
	};
}
