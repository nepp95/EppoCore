#pragma once

#include "EppoCore/Core/KeyCodes.h"
#include "EppoCore/Core/MouseCodes.h"

#include <glm/glm.hpp>

namespace Eppo
{
	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);
		static bool IsMouseButtonPressed(MouseCode button);

		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}
