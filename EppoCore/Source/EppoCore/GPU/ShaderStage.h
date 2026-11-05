#pragma once

#include "EppoCore/GPU/Vulkan.h"

namespace Eppo
{
    enum class ShaderStage : uint8_t
    {
        Vertex,
        Fragment,
        Compute,
    };

    [[nodiscard]] constexpr auto ToVkShaderStage(const ShaderStage stage) -> VkShaderStageFlagBits
    {
        switch (stage)
        {
            case ShaderStage::Vertex:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage::Fragment:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            case ShaderStage::Compute:
                return VK_SHADER_STAGE_COMPUTE_BIT;
        }

        return VK_SHADER_STAGE_ALL;
    }
}
