#include "pch.h"
#include "ApplicationGUI.h"

#include "EppoCore/GPU/Device.h"
#include "EppoCore/GPU/Instance.h"
#include "EppoCore/Platform/GUI/Swapchain.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Eppo
{
    Application* Application::s_Instance = nullptr;

    Application::Application(ApplicationSpecification specification)
        : m_Specification(std::move(specification))
    {
        EP_ASSERT(!s_Instance);
        s_Instance = this;

        // Create window
        WindowSpecification spec(m_Specification.Title, m_Specification.Width, m_Specification.Height);
        m_Window = std::make_unique<Window>(spec);
        m_Window->SetEventCallbackFn(
            [this](Event& e)
            {
                OnEvent(e);
            }
        );

        // The device needs the window's surface extensions, and the swapchain needs the device
        m_Device = std::make_unique<Device>(DeviceSpecification{
            .InstanceExtensions = Window::GetRequiredInstanceExtensions(),
            .DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME },
            .PipelineCachePath = m_Specification.LocalWorkingDirectory / "pipeline_cache.bin",
        });
        m_Swapchain = std::make_unique<Swapchain>(*m_Device, *m_Window, m_Specification.VSync);

        // Initialize renderer
        m_Renderer = std::make_shared<Renderer>(*m_Device);

        // Create UI layer
        m_ImGuiLayer = std::make_shared<ImGuiLayer>(*m_Device, *m_Swapchain);
        PushLayer(m_ImGuiLayer);
    }

    Application::~Application()
    {
        if (m_Device)
            m_Device->WaitIdle();

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
            (*it)->OnDetach();

        m_LayerStack.clear();
        s_Instance = nullptr;
    }

    void Application::Close()
    {
        m_Running = false;
    }

    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

        for (const auto& layer : m_LayerStack)
        {
            if (e.Handled)
                break;

            layer->OnEvent(e);
        }
    }

    auto Application::GetRenderer() const -> std::shared_ptr<Renderer>
    {
        return m_Renderer;
    }

    auto Application::GetWindow() const -> const Window&
    {
        return *m_Window;
    }

    auto Application::GetDevice() const -> Device&
    {
        return *m_Device;
    }

    auto Application::GetSwapchain() const -> Swapchain&
    {
        return *m_Swapchain;
    }

    auto Application::Get() -> Application&
    {
        return *s_Instance;
    }

    auto Application::PushLayer(const std::shared_ptr<Layer>& layer) -> void
    {
        m_LayerStack.emplace_back(layer);
        layer->OnAttach();
    }

    auto Application::OnWindowClose(const WindowCloseEvent& e) -> bool
    {
        Close();

        return true;
    }

    auto Application::OnWindowResize(const WindowResizeEvent& e) -> bool
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            // Minimized application
            m_IsMinimized = true;
            return false;
        }

        m_IsMinimized = false;

        return true;
    }

    auto Application::Run() -> void
    {
        while (m_Running)
        {
            const auto time = static_cast<float>(glfwGetTime());
            const float ts = time - m_LastFrameTime;
            m_LastFrameTime = time;

            {
                ZoneScopedN("PollEvents");
                Window::PollEvents();
            }

            if (m_IsMinimized)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                FrameMark;
                continue;
            }

            if (!m_Swapchain->BeginFrame())
            {
                FrameMark;
                continue;
            }

            // Update layers
            {
                ZoneScopedN("Update");
                for (const auto& layer : m_LayerStack)
                    layer->OnUpdate(ts);
            }

            // Update UI
            {
                ZoneScopedN("UI");
                for (const auto& layer : m_LayerStack)
                    layer->PreUIRender();

                m_ImGuiLayer->Begin();

                for (const auto& layer : m_LayerStack)
                    layer->OnUIRender();

                m_ImGuiLayer->End();

                for (const auto& layer : m_LayerStack)
                    layer->PostUIRender();
            }

            {
                ZoneScopedN("Present");
                CommandList& commandList = m_Swapchain->GetCommandList();
                const RenderingSpecification renderingSpecification{
                    .ColorAttachments = { ColorAttachment{
                        .Target = &m_Swapchain->GetImage(), .Load = LoadOp::Clear, .ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f } } },
                };
                commandList.BeginRendering(renderingSpecification);
                m_ImGuiLayer->Render(commandList);
                commandList.EndRendering();

                m_Swapchain->EndFrame();
            }

            m_ImGuiLayer->RenderPlatformWindows();

            FrameMark;
        }
    }
}
