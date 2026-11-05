#pragma once

#include "EppoCore/GPU/GpuBuffer.h"
#include "EppoCore/GPU/Shader.h"
#include "Framebuffer.h"
#include "UniformBuffer.h"
#include "Vertex.h"

#include <glm/glm.hpp>

namespace Eppo
{
    class CommandList;
    class Device;
    class GraphicsPipeline;

    constexpr uint32_t MAX_QUADS = 10000;
    constexpr uint32_t MAX_VERTICES = 4 * MAX_QUADS;

    // Batched 2D quads, rendered into a Framebuffer with the embedded quad shaders
    class Renderer
    {
    public:
        explicit Renderer(Device& device);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        auto operator=(const Renderer&) -> Renderer& = delete;
        Renderer(Renderer&&) = delete;
        auto operator=(Renderer&&) -> Renderer& = delete;

        // Starts rendering into target; EndScene uploads the batch and submits it
        auto BeginScene(Framebuffer& target, const glm::mat4& projection) -> void;
        auto EndScene() -> void;

        auto DrawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) -> void;

    private:
        struct UboData
        {
            glm::mat4 ProjectionMatrix;
        } m_UboData{};

        Device* m_Device = nullptr;
        Framebuffer* m_Target = nullptr;
        std::unique_ptr<CommandList> m_CommandList;

        std::unique_ptr<UniformBuffer> m_UniformBuffer;
        std::unique_ptr<Shader> m_QuadVertexShader;
        std::unique_ptr<Shader> m_QuadFragmentShader;
        std::unique_ptr<GraphicsPipeline> m_QuadPipeline;

        std::unique_ptr<GpuBuffer> m_QuadVertexBuffer;
        std::unique_ptr<GpuBuffer> m_QuadIndexBuffer;

        std::vector<Vertex> m_Vertices;
        uint32_t m_IndexCount = 0;
    };
}
