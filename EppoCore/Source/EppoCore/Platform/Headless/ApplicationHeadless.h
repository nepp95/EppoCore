#pragma once

#include "EppoCore/Core/ApplicationSpecification.h"
#include "EppoCore/Core/Layer.h"
#include "EppoCore/Core/Timer.h"

auto main(int argc, char** argv) -> int;

namespace Eppo
{
    class Application
    {
    public:
        explicit Application(ApplicationSpecification specification);
        ~Application();

        auto Close() -> void;

        [[nodiscard]] constexpr auto GetSpecification() const -> const ApplicationSpecification& { return m_Specification; }

        static auto Get() -> Application&;

    protected:
        auto PushLayer(const std::shared_ptr<Layer>& layer) -> void;

    private:
        auto Run() -> void;

    private:
        ApplicationSpecification m_Specification;
        std::vector<std::shared_ptr<Layer>> m_LayerStack;

        bool m_Running = true;
        float m_LastFrameTime = 0.0f;
        Timer m_Timer;

        static Application* s_Instance;

        friend auto ::main(int argc, char** argv) -> int;
    };

    // To be implemented by the client using this library
    auto CreateApplication(ApplicationCommandLineArgs args) -> Application*;
}
