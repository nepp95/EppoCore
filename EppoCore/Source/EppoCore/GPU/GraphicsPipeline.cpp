#include "pch.h"
#include "GraphicsPipeline.h"

#include "EppoCore/GPU/Device.h"
#include "EppoCore/GPU/PipelineCache.h"
#include "EppoCore/GPU/Shader.h"

namespace Eppo
{
    namespace
    {
        auto ToVkFormat(const VertexFormat format) -> VkFormat
        {
            switch (format)
            {
                case VertexFormat::Float:
                    return VK_FORMAT_R32_SFLOAT;
                case VertexFormat::Float2:
                    return VK_FORMAT_R32G32_SFLOAT;
                case VertexFormat::Float3:
                    return VK_FORMAT_R32G32B32_SFLOAT;
                case VertexFormat::Float4:
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
                case VertexFormat::Int:
                    return VK_FORMAT_R32_SINT;
                case VertexFormat::UInt:
                    return VK_FORMAT_R32_UINT;
            }

            return VK_FORMAT_UNDEFINED;
        }

        auto GetSize(const VertexFormat format) -> uint32_t
        {
            switch (format)
            {
                case VertexFormat::Float:
                case VertexFormat::Int:
                case VertexFormat::UInt:
                    return 4;
                case VertexFormat::Float2:
                    return 8;
                case VertexFormat::Float3:
                    return 12;
                case VertexFormat::Float4:
                    return 16;
            }

            return 0;
        }

        auto ToVkCullMode(const CullMode mode) -> VkCullModeFlags
        {
            switch (mode)
            {
                case CullMode::None:
                    return VK_CULL_MODE_NONE;
                case CullMode::Back:
                    return VK_CULL_MODE_BACK_BIT;
                case CullMode::Front:
                    return VK_CULL_MODE_FRONT_BIT;
            }

            return VK_CULL_MODE_NONE;
        }

        auto ToVkTopology(const PrimitiveTopology topology) -> VkPrimitiveTopology
        {
            switch (topology)
            {
                case PrimitiveTopology::TriangleList:
                    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                case PrimitiveTopology::TriangleStrip:
                    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
                case PrimitiveTopology::LineList:
                    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                case PrimitiveTopology::PointList:
                    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            }

            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }

        auto GetShaders(const GraphicsPipelineSpecification& specification) -> std::array<const Shader*, 2>
        {
            return { specification.VertexShader, specification.FragmentShader };
        }
    }

    GraphicsPipeline::GraphicsPipeline(Device& device, const GraphicsPipelineSpecification& specification)
        : Pipeline(device, VK_PIPELINE_BIND_POINT_GRAPHICS, GetShaders(specification), specification.DebugName)
    {
        ZoneScoped;

        if (!m_Layout.IsValid())
            return;

        if (specification.VertexShader->GetStage() != ShaderStage::Vertex ||
            specification.FragmentShader->GetStage() != ShaderStage::Fragment)
        {
            Log::Error("Graphics pipeline {} needs a vertex and a fragment shader", m_Name);
            return;
        }

        const std::array stages = {
            VkPipelineShaderStageCreateInfo{
                                            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                            .stage = VK_SHADER_STAGE_VERTEX_BIT,
                                            .module = specification.VertexShader->GetModule(),
                                            .pName = "main",
                                            },
            VkPipelineShaderStageCreateInfo{
                                            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                                            .module = specification.FragmentShader->GetModule(),
                                            .pName = "main",
                                            },
        };

        // Vertex input: one interleaved buffer
        std::vector<VkVertexInputAttributeDescription> attributes;
        uint32_t stride = 0;
        for (uint32_t i = 0; i < specification.VertexLayout.size(); i++)
        {
            const VertexFormat format = specification.VertexLayout[i];
            attributes.push_back({ .location = i, .binding = 0, .format = ToVkFormat(format), .offset = stride });
            stride += GetSize(format);
        }

        const VkVertexInputBindingDescription vertexBinding{
            .binding = 0,
            .stride = stride,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        const bool hasVertices = !attributes.empty();
        const VkPipelineVertexInputStateCreateInfo vertexInput{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = hasVertices ? 1u : 0u,
            .pVertexBindingDescriptions = &vertexBinding,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size()),
            .pVertexAttributeDescriptions = attributes.data(),
        };

        const VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = ToVkTopology(specification.Topology),
        };

        const VkPipelineViewportStateCreateInfo viewport{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        const VkPipelineRasterizationStateCreateInfo rasterization{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = ToVkCullMode(specification.Cull),
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .lineWidth = 1.0f,
        };

        const VkPipelineMultisampleStateCreateInfo multisample{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        };

        const VkPipelineDepthStencilStateCreateInfo depthStencil{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = specification.DepthTest,
            .depthWriteEnable = specification.DepthWrite,
            .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
        };

        // Same blend state for every color attachment
        const VkPipelineColorBlendAttachmentState blendAttachment{
            .blendEnable = specification.AlphaBlending,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };
        const std::vector blendAttachments(specification.ColorFormats.size(), blendAttachment);

        const VkPipelineColorBlendStateCreateInfo colorBlend{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(blendAttachments.size()),
            .pAttachments = blendAttachments.data(),
        };

        constexpr std::array dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        const VkPipelineDynamicStateCreateInfo dynamicState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data(),
        };

        std::vector<VkFormat> colorFormats;
        for (const ImageFormat format : specification.ColorFormats)
            colorFormats.push_back(ToVkFormat(format));

        const VkPipelineRenderingCreateInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = static_cast<uint32_t>(colorFormats.size()),
            .pColorAttachmentFormats = colorFormats.data(),
            .depthAttachmentFormat = Eppo::ToVkFormat(specification.DepthFormat),
        };

        const VkGraphicsPipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderingInfo,
            .stageCount = static_cast<uint32_t>(stages.size()),
            .pStages = stages.data(),
            .pVertexInputState = &vertexInput,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewport,
            .pRasterizationState = &rasterization,
            .pMultisampleState = &multisample,
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &colorBlend,
            .pDynamicState = &dynamicState,
            .layout = m_Layout.GetHandle(),
        };

        EP_VK_CHECK(vkCreateGraphicsPipelines(
            m_Device->GetHandle(), m_Device->GetPipelineCache().GetHandle(), 1, &pipelineInfo, nullptr, &m_Pipeline
        ));
        m_Device->SetDebugName(VK_OBJECT_TYPE_PIPELINE, VkHandleToU64(m_Pipeline), m_Name);
    }
}
