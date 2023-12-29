#pragma once

#include "Core/Layer.h"

namespace Eppo
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer() = default;
		~ImGuiLayer() = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }

	private:
		bool m_BlockEvents = true;
	};
}
