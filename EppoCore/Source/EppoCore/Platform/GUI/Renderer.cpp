#include "Renderer.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "ApplicationGUI.h"
#include "pch.h"

namespace Eppo
{
    Renderer::Renderer()
    {
        // Create uniform buffer
        m_UniformBuffer = std::make_unique<UniformBuffer>(sizeof(UboData), 0);

        // Compile shader
        const auto& spec = Application::Get().GetSpecification();

        std::array paths = { spec.LocalWorkingDirectory / "Shaders" / "Quad.vert", spec.LocalWorkingDirectory / "Shaders" / "Quad.frag" };
        m_QuadShader = std::make_unique<Shader>(paths);

        paths = { spec.LocalWorkingDirectory / "Shaders" / "Text.vert", spec.LocalWorkingDirectory / "Shaders" / "Text.frag" };
        m_TextShader = std::make_unique<Shader>(paths);

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

        // Setup OpenGL
        // Create quad vertex buffer
        glCreateBuffers(1, &m_QuadVertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, m_QuadVertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

        // Create quad index buffer
        glCreateBuffers(1, &m_QuadIndexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, m_QuadIndexBufferID);
        glBufferData(GL_ARRAY_BUFFER, MAX_INDICES * sizeof(uint32_t), quadIndices, GL_STATIC_DRAW);

        // Create quad vertex array
        glCreateVertexArrays(1, &m_QuadVertexArrayID);
        glBindVertexArray(m_QuadVertexArrayID);
        glBindBuffer(GL_ARRAY_BUFFER, m_QuadVertexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIndexBufferID);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(sizeof(glm::vec2)));

        // Create text vertex buffer
        glCreateBuffers(1, &m_TextVertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, m_TextVertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(TextVertex), nullptr, GL_DYNAMIC_DRAW);

        // Create text index buffer
        glCreateBuffers(1, &m_TextIndexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, m_TextIndexBufferID);
        glBufferData(GL_ARRAY_BUFFER, MAX_INDICES * sizeof(uint32_t), quadIndices, GL_STATIC_DRAW);

        // Create text vertex array
        glCreateVertexArrays(1, &m_TextVertexArrayID);
        glBindVertexArray(m_TextVertexArrayID);
        glBindBuffer(GL_ARRAY_BUFFER, m_TextVertexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_TextIndexBufferID);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), nullptr);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), reinterpret_cast<const void*>(sizeof(glm::vec2)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(TextVertex),
                              reinterpret_cast<const void*>(sizeof(glm::vec2) + sizeof(glm::vec2)));

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

        for (const auto& character : m_Characters | std::views::values)
            glDeleteTextures(1, &character.RendererID);

        glDeleteVertexArrays(1, &m_TextVertexArrayID);
        glDeleteBuffers(1, &m_TextVertexBufferID);

        glDeleteVertexArrays(1, &m_QuadVertexArrayID);
        glDeleteBuffers(1, &m_QuadVertexBufferID);
        glDeleteBuffers(1, &m_QuadIndexBufferID);
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
        if (m_QuadIndexCount)
        {
            // Update vertex buffer
            const auto dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(m_QuadVertexBufferPtr) -
                reinterpret_cast<uint8_t*>(m_QuadVertexBufferBase));
            glBindBuffer(GL_ARRAY_BUFFER, m_QuadVertexBufferID);
            glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, m_QuadVertexBufferBase);

            // Draw call
            m_QuadShader->Bind();
            glBindVertexArray(m_QuadVertexArrayID);
            glDrawElements(GL_TRIANGLES, static_cast<int>(m_QuadIndexCount), GL_UNSIGNED_INT, nullptr);
        }

        if (m_TextIndexCount)
        {
            // Update vertex buffer
            const auto dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(m_TextVertexBufferPtr) -
                reinterpret_cast<uint8_t*>(m_TextVertexBufferBase));
            glBindBuffer(GL_ARRAY_BUFFER, m_TextVertexBufferID);
            glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, m_TextVertexBufferBase);

            // Bind textures
            for (uint32_t i = 0; i < m_TextureIndex; i++)
                glBindTextureUnit(i, m_Textures[i]);

            // Draw call
            m_TextShader->Bind();
            glBindVertexArray(m_TextVertexArrayID);
            glDrawElements(GL_TRIANGLES, static_cast<int>(m_TextIndexCount), GL_UNSIGNED_INT, nullptr);
        }
    }
}
