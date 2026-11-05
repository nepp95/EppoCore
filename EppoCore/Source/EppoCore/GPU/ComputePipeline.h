#pragma once

#include "EppoCore/GPU/Pipeline.h"

namespace Eppo
{
    class ComputePipeline final : public Pipeline
    {
    public:
        ComputePipeline(Device& device, const Shader& shader);
        // Compiles the GLSL compute shader at path
        ComputePipeline(Device& device, const std::filesystem::path& path);

        // Workgroup size declared by the shader (local_size_x/y/z)
        [[nodiscard]] auto GetLocalSize() const -> const std::array<uint32_t, 3>& { return m_LocalSize; }

    private:
        std::array<uint32_t, 3> m_LocalSize = { 1, 1, 1 };
    };
}
