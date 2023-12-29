#pragma once

#include "Event/Event.h"

namespace Eppo
{
	class Layer
	{
	public:
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		
		virtual void OnEvent(Event& e) {}
		virtual void OnUpdate(float timestep) {}
		virtual void OnUIRender() {}
	};
}
