#pragma once

#include "EppoCore/GPU/Image.h"
#include "EppoCore/GPU/Pipeline.h"

namespace Eppo
{
    enum class VertexFormat : uint8_t
    {
        Float,
        Float2,
        Float3,
        Float4,
        Int,
        UInt,
    };

    enum class CullMode : uint8_t
    {
        None,
        Back,
        Front,
    };

    enum class PrimitiveTopology : uint8_t
    {
        TriangleList,
        TriangleStrip,
        LineList,
        PointList,
    };

    struct GraphicsPipelineSpecification
    {
        const Shader* VertexShader = nullptr;
        const Shader* FragmentShader = nullptr;

        // One interleaved vertex buffer (binding 0); attribute i is at location i, packed in order
        std::vector<VertexFormat> VertexLayout;

        // Formats of the attachments it renders to (dynamic rendering)
        std::vector<ImageFormat> ColorFormats;
        ImageFormat DepthFormat = ImageFormat::Undefined;

        bool AlphaBlending = false;
        bool DepthTest = false;
        bool DepthWrite = false;
        CullMode Cull = CullMode::None;
        PrimitiveTopology Topology = PrimitiveTopology::TriangleList;

        std::string DebugName;
    };

    // Viewport and scissor are dynamic state, set by CommandList::BeginRendering (or SetViewport/SetScissor)
    class GraphicsPipeline final : public Pipeline
    {
    public:
        GraphicsPipeline(Device& device, const GraphicsPipelineSpecification& specification);
    };
}
