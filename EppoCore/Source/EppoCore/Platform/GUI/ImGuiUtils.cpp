#include "pch.h"
#include "ImGuiUtils.h"

#include "EppoCore/GPU/Sampler.h"

#include <imgui_impl_vulkan.h>

namespace Eppo
{
    auto ToImGuiTextureID(const Image& image, const Sampler& sampler, const VkImageLayout layout) -> ImTextureID
    {
        const VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(sampler.GetHandle(), image.GetView(), layout);
        return reinterpret_cast<ImTextureID>(descriptorSet);
    }

    auto RemoveImGuiTexture(const ImTextureID textureID) -> void
    {
        ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(textureID));
    }
}
