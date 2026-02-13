#pragma once

#ifdef EP_GUI
    #include "EppoCore/Platform/GUI/Event/Event.h"
#endif

namespace Eppo
{
    class Layer
    {
    public:
        virtual ~Layer() = default;

        virtual auto OnAttach() -> void {}
        virtual auto OnDetach() -> void {}

        virtual auto OnUpdate(float timestep) -> void {}
#ifdef EP_GUI
        virtual auto OnEvent(Event& e) -> void {}
        virtual auto PreUIRender() -> void {}
        virtual auto OnUIRender() -> void {}
        virtual auto PostUIRender() -> void {}
#endif
    };
}
