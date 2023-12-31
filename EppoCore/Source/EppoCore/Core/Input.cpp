#include "pch.h"
#include "Input.h"

#include "EppoCore/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Eppo
{
	bool Input::IsKeyPressed(KeyCode key)
	{
		GLFWwindow* window = Application::Get().GetWindow().GetNativeWindow();
		int keyState = glfwGetKey(window, key);

		return keyState == GLFW_PRESS;
	}

	bool Input::IsMouseButtonPressed(MouseCode button)
	{
		GLFWwindow* window = Application::Get().GetWindow().GetNativeWindow();
		int buttonState = glfwGetMouseButton(window, button);

		return buttonState == GLFW_PRESS;
	}

	glm::vec2 Input::GetMousePosition()
	{
		GLFWwindow* window = Application::Get().GetWindow().GetNativeWindow();
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		return glm::vec2(static_cast<float>(xPos), static_cast<float>(yPos));
	}

	float Input::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		return GetMousePosition().y;
	}

	void Input::SetCursorMode(CursorMode mode)
	{
		GLFWwindow* window = Application::Get().GetWindow().GetNativeWindow();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
	}
}
