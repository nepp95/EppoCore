#pragma once

#include "EppoCore/GPU/PipelineLayout.h"

namespace Eppo
{
    class Device;
    class Shader;

    // Base for compute and graphics pipelines. Check IsValid(): shader/layout errors are logged, not fatal.
    class Pipeline
    {
    public:
        virtual ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        auto operator=(const Pipeline&) -> Pipeline& = delete;
        Pipeline(Pipeline&&) = delete;
        auto operator=(Pipeline&&) -> Pipeline& = delete;

        [[nodiscard]] auto IsValid() const -> bool { return m_Pipeline != VK_NULL_HANDLE; }
        [[nodiscard]] auto GetHandle() const -> VkPipeline { return m_Pipeline; }
        [[nodiscard]] auto GetBindPoint() const -> VkPipelineBindPoint { return m_BindPoint; }
        [[nodiscard]] auto GetLayout() const -> const PipelineLayout& { return m_Layout; }
        [[nodiscard]] auto GetName() const -> const std::string& { return m_Name; }

    protected:
        Pipeline(Device& device, VkPipelineBindPoint bindPoint, std::span<const Shader* const> shaders, std::string_view name);

    protected:
        Device* m_Device;
        VkPipelineBindPoint m_BindPoint;
        std::string m_Name;
        PipelineLayout m_Layout;
        VkPipeline m_Pipeline = VK_NULL_HANDLE;
    };
}
