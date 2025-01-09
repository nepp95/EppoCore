#pragma once

namespace Eppo
{
	class Layer
	{
	public:
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void OnUpdate(float timestep) {}
		virtual void OnUIRender() {}
	};
}
