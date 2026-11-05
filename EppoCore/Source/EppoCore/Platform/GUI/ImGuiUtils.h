#pragma once

#include "EppoCore/GPU/Image.h"

#include <imgui.h>

namespace Eppo
{
    class Sampler;

    // Registers the image with ImGui's Vulkan backend, for ImGui::Image / ImGui::ImageButton. Remove it with
    // RemoveImGuiTexture once the image is destroyed.
    [[nodiscard]] auto
    ToImGuiTextureID(const Image& image, const Sampler& sampler, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        -> ImTextureID;
    auto RemoveImGuiTexture(ImTextureID textureID) -> void;
}
