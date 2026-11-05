#include "pch.h"
#include "PipelineLayout.h"

#include "EppoCore/GPU/BindlessTable.h"
#include "EppoCore/GPU/Device.h"
#include "EppoCore/GPU/Shader.h"

namespace Eppo
{
    PipelineLayout::PipelineLayout(Device& device, const std::span<const Shader* const> shaders, const std::string_view name)
        : m_Device(&device)
    {
        ZoneScoped;

        // Merge the bindings of every stage; a binding used by several stages must have the same type and count in each
        bool valid = true;
        for (const Shader* shader : shaders)
        {
            if (!shader || !shader->IsValid())
            {
                Log::Error("Pipeline {} has an invalid shader", name);
                return;
            }

            const ShaderReflection& reflection = shader->GetReflection();
            for (const ShaderBinding& binding : reflection.Bindings)
            {
                auto existing = std::ranges::find(m_Bindings, binding.Binding, &ShaderBinding::Binding);
                if (existing == m_Bindings.end())
                {
                    m_Bindings.push_back(binding);
                    continue;
                }

                if (existing->Type != binding.Type || existing->Count != binding.Count)
                {
                    Log::Error(
                        "Pipeline {}: binding {} has a different type or count in shader {}", name, binding.Binding, shader->GetName()
                    );
                    valid = false;
                }

                existing->Stages |= binding.Stages;
            }

            if (reflection.PushConstantSize > 0)
            {
                m_PushConstantSize = std::max(m_PushConstantSize, reflection.PushConstantSize);
                m_PushConstantStages |= ToVkShaderStage(shader->GetStage());
            }
        }

        uint32_t descriptorCount = 0;
        for (const ShaderBinding& binding : m_Bindings)
            descriptorCount += binding.Count;

        const uint32_t maxPushDescriptors = m_Device->GetPhysicalDevice().GetMaxPushDescriptors();
        if (descriptorCount > maxPushDescriptors)
        {
            Log::Error(
                "Pipeline {} uses {} descriptors in set 0, but this GPU pushes at most {}; put large arrays in the bindless table", name,
                descriptorCount, maxPushDescriptors
            );
            valid = false;
        }

        if (!valid)
            return;

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        for (const ShaderBinding& binding : m_Bindings)
        {
            layoutBindings.push_back(
                {
                    .binding = binding.Binding,
                    .descriptorType = binding.Type,
                    .descriptorCount = binding.Count,
                    .stageFlags = binding.Stages,
                }
            );
        }

        const VkDescriptorSetLayoutCreateInfo pushDescriptorInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
            .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
            .pBindings = layoutBindings.data(),
        };

        EP_VK_CHECK(vkCreateDescriptorSetLayout(m_Device->GetHandle(), &pushDescriptorInfo, nullptr, &m_PushDescriptorLayout));

        // Every pipeline has the bindless table as set 1, used or not, so it can stay bound across pipelines
        const std::array setLayouts = { m_PushDescriptorLayout, m_Device->GetBindlessTable().GetLayout() };

        const VkPushConstantRange pushConstantRange{
            .stageFlags = m_PushConstantStages,
            .offset = 0,
            .size = m_PushConstantSize,
        };

        const VkPipelineLayoutCreateInfo layoutInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
            .pSetLayouts = setLayouts.data(),
            .pushConstantRangeCount = m_PushConstantSize > 0 ? 1u : 0u,
            .pPushConstantRanges = &pushConstantRange,
        };

        EP_VK_CHECK(vkCreatePipelineLayout(m_Device->GetHandle(), &layoutInfo, nullptr, &m_Layout));
        m_Device->SetDebugName(VK_OBJECT_TYPE_PIPELINE_LAYOUT, VkHandleToU64(m_Layout), name);
    }

    PipelineLayout::~PipelineLayout()
    {
        // Command lists that are still in flight reference the layout
        m_Device->Defer(
            [device = m_Device->GetHandle(), layout = m_Layout, pushDescriptorLayout = m_PushDescriptorLayout]
            {
                vkDestroyPipelineLayout(device, layout, nullptr);
                vkDestroyDescriptorSetLayout(device, pushDescriptorLayout, nullptr);
            }
        );
    }

    auto PipelineLayout::FindBinding(const uint32_t binding) const -> const ShaderBinding*
    {
        const auto it = std::ranges::find(m_Bindings, binding, &ShaderBinding::Binding);
        return it != m_Bindings.end() ? &*it : nullptr;
    }
}
