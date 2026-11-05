#pragma once

#include "EppoCore/GPU/ShaderStage.h"

namespace Eppo
{
    // One descriptor in the push descriptor set (set 0)
    struct ShaderBinding
    {
        uint32_t Binding = 0;
        VkDescriptorType Type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        uint32_t Count = 1;
        VkShaderStageFlags Stages = 0;
    };

    // What a shader expects to be bound, read from its SPIR-V
    struct ShaderReflection
    {
        std::vector<ShaderBinding> Bindings;
        bool UsesBindlessTable = false;
        uint32_t PushConstantSize = 0;
        std::array<uint32_t, 3> LocalSize = { 1, 1, 1 }; // compute workgroup size

        // Nullopt (with logged errors) for invalid SPIR-V, or descriptor sets other than 0 (push descriptors) and 1 (bindless table)
        [[nodiscard]] static auto Reflect(std::span<const uint32_t> spirv, ShaderStage stage, std::string_view name)
            -> std::optional<ShaderReflection>;
    };
}
