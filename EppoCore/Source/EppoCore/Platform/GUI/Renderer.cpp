#include "pch.h"
#include "Renderer.h"

#include "EmbeddedShaders.h"

#include "EppoCore/GPU/CommandList.h"
#include "EppoCore/GPU/Device.h"
#include "EppoCore/GPU/GraphicsPipeline.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Eppo
{
    Renderer::Renderer(Device& device)
        : m_Device(&device)
    {
        ZoneScoped;

        m_UniformBuffer = std::make_unique<UniformBuffer>(device, static_cast<uint32_t>(sizeof(UboData)), 0);

        m_QuadVertexShader = std::make_unique<Shader>(device, ShaderStage::Vertex, s_QuadVertexShader, "Quad.vert");
        m_QuadFragmentShader = std::make_unique<Shader>(device, ShaderStage::Fragment, s_QuadFragmentShader, "Quad.frag");
        EP_ASSERT(m_QuadVertexShader->IsValid() && m_QuadFragmentShader->IsValid());

        const GraphicsPipelineSpecification pipelineSpecification{
            .VertexShader = m_QuadVertexShader.get(),
            .FragmentShader = m_QuadFragmentShader.get(),
            .VertexLayout = { VertexFormat::Float2, VertexFormat::Float4 },
            .ColorFormats = { ImageFormat::RGBA8 },
            .AlphaBlending = true,
            .DebugName = "Quad pipeline",
        };
        m_QuadPipeline = std::make_unique<GraphicsPipeline>(device, pipelineSpecification);
        EP_ASSERT(m_QuadPipeline->IsValid());

        m_QuadVertexBuffer = std::make_unique<GpuBuffer>(
            device,
            GpuBufferSpecification{
                .Size = MAX_VERTICES * sizeof(Vertex),
                .Usage = BufferUsage::Vertex,
                .Memory = MemoryUsage::CpuToGpu,
                .DebugName = "Quad vertices",
            }
        );

        std::vector<uint32_t> indices(6 * MAX_QUADS);
        uint32_t offset = 0;
        for (uint32_t i = 0; i < indices.size(); i += 6)
        {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;

            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;

            offset += 4;
        }

        m_QuadIndexBuffer = std::make_unique<GpuBuffer>(
            device,
            GpuBufferSpecification{ .Size = indices.size() * sizeof(uint32_t), .Usage = BufferUsage::Index, .DebugName = "Quad indices" }
        );
        m_QuadIndexBuffer->Upload(indices.data(), indices.size() * sizeof(uint32_t));

        m_Vertices.reserve(MAX_VERTICES);
    }

    Renderer::~Renderer() = default;

    auto Renderer::BeginScene(Framebuffer& target, const glm::mat4& projection) -> void
    {
        ZoneScoped;
        EP_ASSERT(!m_CommandList);
        EP_ASSERT(target.IsValid());

        m_Target = &target;
        m_UboData.ProjectionMatrix = projection;
        m_UniformBuffer->SetData(&m_UboData, sizeof(UboData));
        m_Vertices.clear();
        m_IndexCount = 0;

        m_CommandList = std::make_unique<CommandList>(*m_Device, QueueType::Graphics);
        m_CommandList->Barrier(target.GetImage(), Access::None, Access::ColorAttachment);
        m_CommandList->Barrier(m_UniformBuffer->GetBuffer(), Access::HostWrite, Access::UniformRead);
    }

    auto Renderer::EndScene() -> void
    {
        ZoneScoped;
        EP_ASSERT(m_CommandList && m_Target);

        CommandList& commandList = *m_CommandList;
        const Image& image = m_Target->GetImage();

        // Uploads and buffer barriers aren't allowed inside dynamic rendering
        if (m_IndexCount > 0)
        {
            m_QuadVertexBuffer->Upload(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));
            commandList.Barrier(*m_QuadVertexBuffer, Access::HostWrite, Access::VertexBuffer);
        }

        const RenderingSpecification renderingSpecification{
            .ColorAttachments = { ColorAttachment{ .Target = &image, .Load = LoadOp::Clear, .ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f } } },
        };
        commandList.BeginRendering(renderingSpecification);

        if (m_IndexCount > 0)
        {
            commandList.Bind(*m_QuadPipeline);
            commandList.Bind(m_UniformBuffer->GetBinding(), m_UniformBuffer->GetBuffer());
            commandList.BindVertexBuffer(*m_QuadVertexBuffer);
            commandList.BindIndexBuffer(*m_QuadIndexBuffer);
            commandList.DrawIndexed(m_IndexCount);
        }

        commandList.EndRendering();
        commandList.Barrier(image, Access::ColorAttachment, Access::FragmentRead);
        m_Device->Submit(commandList);

        m_CommandList.reset();
        m_Target = nullptr;
    }

    auto Renderer::DrawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) -> void
    {
        EP_ASSERT(m_CommandList);

        const glm::vec4 positions[] = {
            { -0.5f, -0.5f, 0.0f, 1.0f },
            { 0.5f,  -0.5f, 0.0f, 1.0f },
            { 0.5f,  0.5f,  0.0f, 1.0f },
            { -0.5f, 0.5f,  0.0f, 1.0f },
        };

        if (m_Vertices.size() + std::size(positions) > MAX_VERTICES)
        {
            Log::Warn("Renderer batch is full; dropping rectangle at ({}, {})", position.x, position.y);
            return;
        }

        const glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));

        for (const glm::vec4& vertexPosition : positions)
            m_Vertices.emplace_back(glm::vec2(transform * vertexPosition), color);

        m_IndexCount += 6;
    }
}
