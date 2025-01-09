#pragma once

#include "EppoCore/Core/Layer.h"

namespace Eppo
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer() = default;
		~ImGuiLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;

		void Begin();
		void End();
	};
}
