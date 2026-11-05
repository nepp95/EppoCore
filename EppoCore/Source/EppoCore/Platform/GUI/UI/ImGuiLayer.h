#pragma once

#include "EppoCore/Core/Layer.h"
#include "EppoCore/GPU/Vulkan.h"

namespace Eppo
{
    class CommandList;
    class Device;
    class Swapchain;

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer(Device& device, Swapchain& swapchain);
        ~ImGuiLayer() override = default;

        auto OnAttach() -> void override;
        auto OnDetach() -> void override;

        auto Begin() -> void;
        auto End() -> void;

        // Records ImGui's draw data into the command list; BeginRendering must already be active
        auto Render(CommandList& commandList) -> void;
        // Presents the docking viewports; call after the main frame has been submitted
        auto RenderPlatformWindows() -> void;

    private:
        Device* m_Device;
        Swapchain* m_Swapchain;
        // The backend keeps a pointer to this, so it must outlive ImGui_ImplVulkan_Init
        VkFormat m_ColorFormat = VK_FORMAT_UNDEFINED;
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    };
}
