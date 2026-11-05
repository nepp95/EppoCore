#include "pch.h"
#include "ComputePipeline.h"

#include "EppoCore/GPU/Device.h"
#include "EppoCore/GPU/PipelineCache.h"
#include "EppoCore/GPU/Shader.h"

namespace Eppo
{
    namespace
    {
        auto AsSpan(const Shader& shader) -> std::array<const Shader*, 1>
        {
            return { &shader };
        }
    }

    ComputePipeline::ComputePipeline(Device& device, const Shader& shader)
        : Pipeline(device, VK_PIPELINE_BIND_POINT_COMPUTE, AsSpan(shader), shader.GetName())
    {
        ZoneScoped;

        if (!m_Layout.IsValid())
            return;

        if (shader.GetStage() != ShaderStage::Compute)
        {
            Log::Error("Compute pipeline {} needs a compute shader", m_Name);
            return;
        }

        m_LocalSize = shader.GetReflection().LocalSize;

        const VkComputePipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage =
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                    .module = shader.GetModule(),
                    .pName = "main",
                },
            .layout = m_Layout.GetHandle(),
        };

        EP_VK_CHECK(vkCreateComputePipelines(
            m_Device->GetHandle(), m_Device->GetPipelineCache().GetHandle(), 1, &pipelineInfo, nullptr, &m_Pipeline
        ));
        m_Device->SetDebugName(VK_OBJECT_TYPE_PIPELINE, VkHandleToU64(m_Pipeline), m_Name);
    }

    ComputePipeline::ComputePipeline(Device& device, const std::filesystem::path& path)
        : ComputePipeline(device, Shader(device, path))
    {
    }
}
