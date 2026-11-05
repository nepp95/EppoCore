#include "pch.h"
#include "Pipeline.h"

#include "EppoCore/GPU/Device.h"

namespace Eppo
{
    Pipeline::Pipeline(
        Device& device, const VkPipelineBindPoint bindPoint, const std::span<const Shader* const> shaders, const std::string_view name
    )
        : m_Device(&device), m_BindPoint(bindPoint), m_Name(name), m_Layout(device, shaders, name)
    {
    }

    Pipeline::~Pipeline()
    {
        if (!m_Pipeline)
            return;

        // Command lists that are still in flight reference the pipeline
        m_Device->Defer(
            [device = m_Device->GetHandle(), pipeline = m_Pipeline]
            {
                vkDestroyPipeline(device, pipeline, nullptr);
            }
        );
    }
}
