#include "pch.h"
#include "ImGuiLayer.h"

#include "EppoCore/Core/Application.h"
#include "EppoCore/GPU/CommandList.h"
#include "EppoCore/GPU/Device.h"
#include "EppoCore/GPU/Image.h"
#include "EppoCore/GPU/Instance.h"
#include "EppoCore/GPU/PipelineCache.h"
#include "EppoCore/Platform/GUI/Swapchain.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace Eppo
{
    ImGuiLayer::ImGuiLayer(Device& device, Swapchain& swapchain)
        : m_Device(&device), m_Swapchain(&swapchain)
    {
    }

    auto ImGuiLayer::OnAttach() -> void
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // Setup imgui style
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        const auto& app = Application::Get();
        const auto window = app.GetWindow().GetNativeWindow();

        const bool glfwInitialized = ImGui_ImplGlfw_InitForVulkan(window, true);
        EP_ASSERT(glfwInitialized);

        m_ColorFormat = ToVkFormat(m_Swapchain->GetFormat());

        constexpr uint32_t descriptorCount = 64;
        const VkDescriptorPoolSize poolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorCount };
        const VkDescriptorPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = descriptorCount,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize,
        };
        EP_VK_CHECK(vkCreateDescriptorPool(m_Device->GetHandle(), &poolInfo, nullptr, &m_DescriptorPool));

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.ApiVersion = VK_API_VERSION_1_3;
        initInfo.Instance = m_Device->GetInstance().GetHandle();
        initInfo.PhysicalDevice = m_Device->GetPhysicalDevice().GetHandle();
        initInfo.Device = m_Device->GetHandle();
        initInfo.QueueFamily = m_Device->GetQueue(QueueType::Graphics).GetFamily();
        initInfo.Queue = m_Device->GetQueue(QueueType::Graphics).GetHandle();
        initInfo.DescriptorPool = m_DescriptorPool;
        initInfo.MinImageCount = m_Swapchain->GetMinImageCount();
        initInfo.ImageCount = m_Swapchain->GetImageCount();
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.PipelineCache = m_Device->GetPipelineCache().GetHandle();
        initInfo.UseDynamicRendering = true;
        initInfo.PipelineRenderingCreateInfo = VkPipelineRenderingCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &m_ColorFormat,
        };
        initInfo.MinAllocationSize = 1024 * 1024;
        initInfo.CheckVkResultFn = [](const VkResult result)
        {
            EP_VK_CHECK(result);
        };

        const bool vulkanInitialized = ImGui_ImplVulkan_Init(&initInfo);
        EP_ASSERT(vulkanInitialized);

        io.FontDefault = io.Fonts->AddFontDefault();
    }

    auto ImGuiLayer::OnDetach() -> void
    {
        m_Device->WaitIdle();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        vkDestroyDescriptorPool(m_Device->GetHandle(), m_DescriptorPool, nullptr);
        m_DescriptorPool = VK_NULL_HANDLE;
    }

    auto ImGuiLayer::Begin() -> void
    {
        ImGui_ImplVulkan_SetMinImageCount(m_Swapchain->GetMinImageCount());
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    auto ImGuiLayer::End() -> void
    {
        ImGui::Render();
    }

    auto ImGuiLayer::Render(CommandList& commandList) -> void
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandList.GetHandle());
    }

    auto ImGuiLayer::RenderPlatformWindows() -> void
    {
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }
}
