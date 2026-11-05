#include "pch.h"
#include "Renderer.h"

#include "ApplicationGUI.h"
#include "EmbeddedShaders.h"

#include <glm/gtc/type_ptr.hpp>

namespace Eppo
{
    Renderer::Renderer()
    {
        // Create uniform buffer
        m_UniformBuffer = std::make_unique<UniformBuffer>(sizeof(UboData), 0);

        // Compile shader
        m_QuadShader = std::make_unique<Shader>();
        m_TextShader = std::make_unique<Shader>();

        // Setup index data
        const auto quadIndices = new uint32_t[MAX_INDICES];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < MAX_INDICES; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }

        // Setup vertex data
        m_QuadVertexBufferBase = new Vertex[MAX_VERTICES * sizeof(Vertex)];
        m_TextVertexBufferBase = new TextVertex[MAX_VERTICES * sizeof(TextVertex)];

        m_QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        m_QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
        m_QuadVertexPositions[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
        m_QuadVertexPositions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };

        delete[] quadIndices;
    }

    Renderer::~Renderer()
    {
        delete[] m_QuadVertexBufferBase;
        delete[] m_TextVertexBufferBase;
    }

    void Renderer::BeginScene(const glm::mat4& projection)
    {
        m_UboData.ProjectionMatrix = projection;
        m_UniformBuffer->SetData(&m_UboData, sizeof(UboData));

        StartBatch();
    }

    void Renderer::EndScene() const
    {
        Flush();
    }

    void Renderer::DrawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        const glm::mat4 transform =
                glm::translate(glm::mat4(1.0f), glm::vec3(position, 1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));

        for (const auto& vertexPosition : m_QuadVertexPositions)
        {
            m_QuadVertexBufferPtr->Position = transform * vertexPosition;
            m_QuadVertexBufferPtr->Color = color;
            m_QuadVertexBufferPtr++;
        }

        m_QuadIndexCount += 6;
    }

    void Renderer::DrawString(const glm::vec2& position, const std::string& text, float fontSize)
    {
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        float xOffset = 0.0f;

        for (const auto& c : text)
        {
            const Character& ch = m_Characters[c];
            const float xPos = position.x + ch.Bearing.x + xOffset;
            const float yPos = position.y - ch.Bearing.y;
            const float width = ch.Size.x;
            const float height = ch.Size.y;

            //const glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, yPos, 1.0f)) *
            //glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));

            const glm::vec2 vertices[4] = {
                { xPos, yPos },
                { xPos + width, yPos },
                { xPos + width, yPos + height },
                { xPos, yPos + height },
            };

            float texIndex = 0.0f;
            for (uint32_t i = 0; i < m_TextureIndex; i++)
            {
                if (m_Textures[i] == ch.RendererID)
                {
                    texIndex = static_cast<float>(i);
                    break;
                }
            }

            if (texIndex == 0.0f)
            {
                if (m_TextureIndex >= MAX_TEXTURES)
                    NextBatch();

                texIndex = static_cast<float>(m_TextureIndex);
                m_Textures[m_TextureIndex] = ch.RendererID;
                m_TextureIndex++;
            }

            for (uint32_t i = 0; i < 4; i++)
            {
                m_TextVertexBufferPtr->Position = vertices[i];
                m_TextVertexBufferPtr->TexCoords = textureCoords[i];
                m_TextVertexBufferPtr->TexIndex = texIndex;
                m_TextVertexBufferPtr++;
            }

            m_TextIndexCount += 6;
            xOffset += ch.Advance >> 6;
        }
    }

    void Renderer::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer::StartBatch()
    {
        m_QuadVertexBufferPtr = m_QuadVertexBufferBase;
        m_QuadIndexCount = 0;

        m_TextVertexBufferPtr = m_TextVertexBufferBase;
        m_TextIndexCount = 0;

        m_TextureIndex = 0;
    }

    void Renderer::Flush() const
    {
        
    }
}
