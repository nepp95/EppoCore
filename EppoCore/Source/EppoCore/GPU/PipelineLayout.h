#pragma once

#include "EppoCore/GPU/ShaderReflection.h"

namespace Eppo
{
    class Device;
    class Shader;

    // Set 0: push descriptors merged from the shaders' reflection. Set 1: the bindless table. One push constant range.
    class PipelineLayout
    {
    public:
        PipelineLayout(Device& device, std::span<const Shader* const> shaders, std::string_view name);
        ~PipelineLayout();

        PipelineLayout(const PipelineLayout&) = delete;
        auto operator=(const PipelineLayout&) -> PipelineLayout& = delete;
        PipelineLayout(PipelineLayout&&) = delete;
        auto operator=(PipelineLayout&&) -> PipelineLayout& = delete;

        // False when the shaders are invalid or disagree on a binding (errors are logged)
        [[nodiscard]] auto IsValid() const -> bool { return m_Layout != VK_NULL_HANDLE; }
        [[nodiscard]] auto GetHandle() const -> VkPipelineLayout { return m_Layout; }

        // Nullptr when no shader uses that binding in set 0
        [[nodiscard]] auto FindBinding(uint32_t binding) const -> const ShaderBinding*;
        [[nodiscard]] auto GetPushConstantSize() const -> uint32_t { return m_PushConstantSize; }
        [[nodiscard]] auto GetPushConstantStages() const -> VkShaderStageFlags { return m_PushConstantStages; }

    private:
        Device* m_Device;
        std::vector<ShaderBinding> m_Bindings;
        uint32_t m_PushConstantSize = 0;
        VkShaderStageFlags m_PushConstantStages = 0;
        VkDescriptorSetLayout m_PushDescriptorLayout = VK_NULL_HANDLE;
        VkPipelineLayout m_Layout = VK_NULL_HANDLE;
    };
}
