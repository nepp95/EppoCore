#pragma once

#ifdef EPPO_GUI
    #include "EppoCore/Platform/GUI/Event/Event.h"
#endif

namespace Eppo
{
    class Layer
    {
    public:
        virtual ~Layer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}

        virtual void OnUpdate(float timestep) {}
        #ifdef EPPO_GUI
        virtual void OnEvent(Event& e) {}
        #endif
        virtual void OnUIRender() {}
    };
}
